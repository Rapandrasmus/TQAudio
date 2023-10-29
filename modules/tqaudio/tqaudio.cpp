#include "tqaudio.h"

#define MA_NO_VORBIS
#define MINIAUDIO_IMPLEMENTATION
#define MA_DEBUG_OUTPUT
#include "thirdparty/miniaudio/miniaudio.h"
#include "thirdparty/miniaudio/extras/miniaudio_libvorbis.h"
#include "core/os/os.h"
#include "tqaudio_macros.h"

TQAudio *TQAudio::singleton = nullptr;
SafeNumeric<uint64_t> TQAudio::sound_source_uid;

void TQAudio::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("set_desired_buffer_size_msec", "desired_buffer_size"), &TQAudio::set_desired_buffer_size_msec);
	ClassDB::bind_method(D_METHOD("get_desired_buffer_size_msec"), &TQAudio::get_desired_buffer_size_msec);
	ClassDB::bind_method(D_METHOD("set_master_volume", "linear_volume"), &TQAudio::set_master_volume);
	ClassDB::bind_method(D_METHOD("get_master_volume"), &TQAudio::get_master_volume);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "master_volume"), "set_master_volume", "get_master_volume");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "desired_buffer_size_msec"), "set_desired_buffer_size_msec", "get_desired_buffer_size_msec");
	
	ClassDB::bind_method(D_METHOD("set_dsp_time", "new_time"), &TQAudio::set_dsp_time);
	ClassDB::bind_method(D_METHOD("get_dsp_time"), &TQAudio::get_dsp_time);
	ClassDB::bind_method(D_METHOD("get_actual_buffer_size"), &TQAudio::get_actual_buffer_size);
	ClassDB::bind_method(D_METHOD("get_current_backend_name"), &TQAudio::get_current_backend_name);

	ClassDB::bind_method(D_METHOD("create_group", "group_name", "parent_group"), &TQAudio::create_group);
    ClassDB::bind_method(D_METHOD("initialize"), &TQAudio::godot_initialize);
	ClassDB::bind_method(D_METHOD("get_initialization_error"), &TQAudio::get_initialization_error);
	ClassDB::bind_method(D_METHOD("register_sound_from_encoded_memory", "name_hint", "data"), &TQAudio::register_sound_from_encoded_memory);
	ClassDB::bind_method(D_METHOD("register_sound_from_decoded_memory", "name_hint", "data", "sample_rate"), &TQAudio::register_sound_from_decoded_memory);
}

#pragma region vorbis decoder junk
static ma_result ma_decoding_backend_init__libvorbis(void* pUserData, ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void* pReadSeekTellUserData, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend)
{
    ma_result result;
    ma_libvorbis* pVorbis;

    (void)pUserData;

    pVorbis = (ma_libvorbis*)ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
    if (pVorbis == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    result = ma_libvorbis_init(onRead, onSeek, onTell, pReadSeekTellUserData, pConfig, pAllocationCallbacks, pVorbis);
    if (result != MA_SUCCESS) {
        ma_free(pVorbis, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pVorbis;

    return MA_SUCCESS;
}

static ma_result ma_decoding_backend_init_file__libvorbis(void* pUserData, const char* pFilePath, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend)
{
    ma_result result;
    ma_libvorbis* pVorbis;

    (void)pUserData;

    pVorbis = (ma_libvorbis*)ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
    if (pVorbis == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    result = ma_libvorbis_init_file(pFilePath, pConfig, pAllocationCallbacks, pVorbis);
    if (result != MA_SUCCESS) {
        ma_free(pVorbis, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pVorbis;

    return MA_SUCCESS;
}

static void ma_decoding_backend_uninit__libvorbis(void* pUserData, ma_data_source* pBackend, const ma_allocation_callbacks* pAllocationCallbacks)
{
    ma_libvorbis* pVorbis = (ma_libvorbis*)pBackend;

    (void)pUserData;

    ma_libvorbis_uninit(pVorbis, pAllocationCallbacks);
    ma_free(pVorbis, pAllocationCallbacks);
}

static ma_result ma_decoding_backend_get_channel_map__libvorbis(void* pUserData, ma_data_source* pBackend, ma_channel* pChannelMap, size_t channelMapCap)
{
    ma_libvorbis* pVorbis = (ma_libvorbis*)pBackend;

    (void)pUserData;

    return ma_libvorbis_get_data_format(pVorbis, NULL, NULL, NULL, pChannelMap, channelMapCap);
}

static ma_decoding_backend_vtable g_ma_decoding_backend_vtable_libvorbis =
{
    ma_decoding_backend_init__libvorbis,
    ma_decoding_backend_init_file__libvorbis,
    NULL, /* onInitFileW() */
    NULL, /* onInitMemory() */
    ma_decoding_backend_uninit__libvorbis
};
#pragma endregion

String TQAudio::get_initialization_error() const {
	return "";
}

void TQAudio::ma_data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount) {
	TQAudio *tqaudio = (TQAudio *)pDevice->pUserData;
	if (tqaudio != NULL) {
		ma_engine_read_pcm_frames(&tqaudio->engine, pOutput, frameCount, NULL);
		tqaudio->clock->measure();
	}
}

Ref<TQAudioSourceEncodedMemory> TQAudio::register_sound_from_encoded_memory(String m_name_hint, PackedByteArray m_data) {
	return memnew(TQAudioSourceEncodedMemory(m_name_hint, m_data));
}

Ref<TQAudioSourceDecodedMemory> TQAudio::register_sound_from_decoded_memory(String m_name_hint, PackedByteArray m_data, uint32_t sample_rate) {
	return memnew(TQAudioSourceDecodedMemory(m_name_hint, m_data, sample_rate));
}

Ref<TQAudioGroup> TQAudio::create_group(String m_group_name, Ref<TQAudioGroup> m_parent_group) {
	Ref<TQAudioGroup> out_group = memnew(TQAudioGroup(m_group_name, m_parent_group));
	groups.push_back(out_group);
	return out_group;
}

ma_backend TQAudio::string_to_backend(String str) {
	str = str.to_lower();
	if (str == "wasapi") {
		return ma_backend_wasapi;
	} else if (str == "directsound") {
		return ma_backend_dsound;
	} else if (str == "winmm") {
		return ma_backend_winmm;
	} else if (str == "coreaudio") {
		return ma_backend_coreaudio;
	} else if (str == "sndio") {
		return ma_backend_sndio;
	} else if (str == "audio4") {
		return ma_backend_audio4;
	} else if (str == "oss") {
		return ma_backend_oss;
	} else if (str == "pulseaudio") {
		return ma_backend_pulseaudio;
	} else if (str == "alsa") {
		return ma_backend_alsa;
	} else if (str == "jack") {
		return ma_backend_jack;
	} else if (str == "aaudio") {
		return ma_backend_aaudio;
	} else if (str == "opensl") {
		return ma_backend_opensl;
	} else if (str == "webaudio") {
		return ma_backend_webaudio;
	}
	return ma_backend_null;
}

Error TQAudio::godot_initialize() {
	return initialize(ma_backend_null);
}

Error TQAudio::initialize(ma_backend forced_backend) {
	List<String> args = OS::get_singleton()->get_cmdline_args();
	ma_backend backend_to_force = ma_backend_null;

#ifdef X11_ENABLED
	// PipeWire on deck sucks, lets check if we are on a deck and force alsa by default
	String deck_variable = OS::get_singleton()->get_environment("SteamDeck");
	if (!deck_variable.is_empty() && deck_variable.to_int() > 0) {
		backend_to_force = ma_backend_alsa;
	}
#endif

	for (int i = 0; i < args.size() - 1; i++) {
		if (args[i] == "--tqaudio-backend") {
			backend_to_force = string_to_backend(args[i + 1]);
		}
	}

	clock->measure();

	ma_result result;

	ma_device_config device_config = ma_device_config_init(ma_device_type_playback);
	device_config.pUserData = this;
	device_config.dataCallback = ma_data_callback;
	device_config.playback.format = ma_format_f32;
	device_config.playback.channels = 2;
	device_config.performanceProfile = ma_performance_profile_low_latency;
	if (desired_buffer_size_msec > 0) {
		device_config.periodSizeInMilliseconds = desired_buffer_size_msec;
	}

	// This is necessary to enable WASAPI low latency mode
	device_config.wasapi.noAutoConvertSRC = true;

	ma_backend *backends = NULL;
	uint64_t backend_count = 0;

	if (forced_backend != ma_backend_null) {
		backend_count = 1;
		backends = new ma_backend[1]{ forced_backend };
	}

	result = ma_context_init(backends, 1, NULL, &context);
	MA_ERR_RET(result, "Context init failed");

	delete[] backends;

	result = ma_device_init(&context, &device_config, &device);
	MA_ERR_RET(result, "Device init failed");

	ma_engine_config engine_config = ma_engine_config_init();
	engine_config.pDevice = &device;
	engine_config.channels = 2;
	engine_config.pContext = &context;
	if (desired_buffer_size_msec > 0) {
		engine_config.periodSizeInMilliseconds = desired_buffer_size_msec;
	}

	// Setup libvorbis

	ma_resource_manager_config resourceManagerConfig;

	/*
	Custom backend vtables
	*/
	ma_decoding_backend_vtable *pCustomBackendVTables[] = {
		&g_ma_decoding_backend_vtable_libvorbis
	};

	/* Using custom decoding backends requires a resource manager. */
	resourceManagerConfig = ma_resource_manager_config_init();
	resourceManagerConfig.ppCustomDecodingBackendVTables = pCustomBackendVTables;
	resourceManagerConfig.customDecodingBackendCount = sizeof(pCustomBackendVTables) / sizeof(pCustomBackendVTables[0]);
	resourceManagerConfig.pCustomDecodingBackendUserData = NULL;
	resourceManagerConfig.decodedFormat = ma_format_f32;

	result = ma_resource_manager_init(&resourceManagerConfig, &resource_manager);

	MA_ERR_RET(result, "Resource manager init failed!");

	engine_config.pResourceManager = &resource_manager;

	result = ma_engine_init(&engine_config, &engine);

	MA_ERR_RET(result, "Audio engine init failed!");

	initialized = true;

	return OK;
}

ma_engine *TQAudio::get_engine() 
{
	return &engine;
}

Ref<TQAudioClock> TQAudio::get_clock() 
{
	return clock;
}

TQAudio::TQAudio() 
{
	clock.instantiate();
    singleton = this;
	sound_source_uid.set(0);
}

void TQAudio::set_desired_buffer_size_msec(uint64_t m_new_buffer_size) {
	desired_buffer_size_msec = m_new_buffer_size;
}

uint64_t TQAudio::get_desired_buffer_size_msec() const {
	return desired_buffer_size_msec;
}

Error TQAudio::set_master_volume(float m_linear_volume) {
	MA_ERR_RET(ma_engine_set_volume(&engine, m_linear_volume), "Error setting volume");
	return OK;
}

float TQAudio::get_master_volume() {
	ma_node *endpoint = ma_engine_get_endpoint(&engine);
	return ma_node_get_output_bus_volume(endpoint, 0);
}

uint64_t TQAudio::get_dsp_time() const {
	return ma_engine_get_time(&engine) / (float)(ma_engine_get_sample_rate(&engine) / 1000.0f);
}

Error TQAudio::set_dsp_time(uint64_t m_new_time_msec) {
	ma_result result = ma_engine_set_time(&engine, m_new_time_msec * (float)(ma_engine_get_sample_rate(&engine) / 1000.0f));
	MA_ERR_RET(result, "Error setting DSP time");
	return OK;
}

String TQAudio::get_current_backend_name() const {
	return ma_get_backend_name(context.backend);
}

uint64_t TQAudio::get_actual_buffer_size() const {
	return device.playback.internalPeriodSizeInFrames / (double)(device.playback.internalSampleRate / 1000.0);
}

TQAudio::~TQAudio() 
{
    if (initialized) 
    {
        ma_engine_uninit(&engine);
		ma_resource_manager_uninit(&resource_manager);
		ma_device_uninit(&device);
		ma_context_uninit(&context);
    }
}
