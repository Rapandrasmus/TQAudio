#include "tqaudio_source.h"
#include "tqaudio_macros.h"
#include "tqaudio.h"

void TQAudioSource::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("instantiate", "group", "use_source_channel_count"), &TQAudioSource::instantiate, DEFVAL(false));
}

TQAudioSource::TQAudioSource(String m_name) 
{
    name = m_name;
}

const String TQAudioSource::get_name() const {
	return name;
}

const ma_result TQAudioSource::get_result() const {
	return result;
}

TQAudioPlayer *TQAudioSource::instantiate(Ref<TQAudioGroup> m_group, bool m_use_source_channel_count) {
	return memnew(TQAudioPlayer(this, m_group, m_use_source_channel_count));
}

TQAudioSource::~TQAudioSource(){};

Error TQAudioSourceEncodedMemory::instantiate_sound(Ref<TQAudioGroup> m_group, bool use_source_channel_count, ma_sound *p_sound) {
	ma_sound_config config = ma_sound_config_init();
	config.pFilePath = name.utf8();
	config.flags = config.flags | MA_SOUND_FLAG_NO_SPATIALIZATION;
	if (use_source_channel_count) {
		config.flags = config.flags | MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT;
		config.channelsOut = MA_SOUND_SOURCE_CHANNEL_COUNT;
	} else {
		config.pInitialAttachment = m_group->get_group();
	}

	ma_engine *engine = TQAudio::get_singleton()->get_engine();

	MA_ERR_RET(ma_sound_init_ex(engine, &config, p_sound), "Error initializing sound");
	return OK;
}

TQAudioSourceEncodedMemory::TQAudioSourceEncodedMemory(String m_name, PackedByteArray m_in_data) :
TQAudioSource(m_name) 
{
	data = m_in_data;
	ma_engine *engine = TQAudio::get_singleton()->get_engine();
	name = vformat("%s_%d", name, TQAudio::get_singleton()->get_inc_sound_source_uid());
	result = ma_resource_manager_register_encoded_data(ma_engine_get_resource_manager(engine), name.utf8(), (void *)data.ptr(), data.size());
}

TQAudioSourceEncodedMemory::~TQAudioSourceEncodedMemory() {
	ma_engine *engine = TQAudio::get_singleton()->get_engine();
	ma_resource_manager_unregister_data(ma_engine_get_resource_manager(engine), name.utf8());
}

Error TQAudioSourceDecodedMemory::instantiate_sound(Ref<TQAudioGroup> m_group, bool use_source_channel_count, ma_sound *p_sound) {
	ma_sound_config config = ma_sound_config_init();
	config.pFilePath = name.utf8();
	config.flags = config.flags | MA_SOUND_FLAG_NO_SPATIALIZATION;
	if (use_source_channel_count) {
		config.flags = config.flags | MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT;
		config.channelsOut = MA_SOUND_SOURCE_CHANNEL_COUNT;
	} else {
		config.pInitialAttachment = m_group->get_group();
	}

	ma_engine *engine = TQAudio::get_singleton()->get_engine();

	MA_ERR_RET(ma_sound_init_ex(engine, &config, p_sound), "Error initializing sound");
	return OK;
}

TQAudioSourceDecodedMemory::TQAudioSourceDecodedMemory(String m_name, PackedByteArray m_in_data, ma_uint32 sample_rate, ma_uint32 channels, ma_format format) :
TQAudioSource(m_name)
{
	data = m_in_data;
	ma_engine *engine = TQAudio::get_singleton()->get_engine();
	name = vformat("%s_%d", name, TQAudio::get_singleton()->get_inc_sound_source_uid());

	ma_uint32 ma_bytes_per_pcm_frame;
	switch (format) 
	{
		case ma_format_u8:
			ma_bytes_per_pcm_frame = 1;
			break;
		case ma_format_s16:
			ma_bytes_per_pcm_frame = 2;
			break;
		case ma_format_s24:
			ma_bytes_per_pcm_frame = 3;
			break;
		case ma_format_s32:
		case ma_format_f32:
			ma_bytes_per_pcm_frame = 4;
			break;
	}

	ma_resource_manager_register_decoded_data(ma_engine_get_resource_manager(engine), name.utf8(), (void *)data.ptr(), data.size() / (ma_bytes_per_pcm_frame * channels), format, channels, sample_rate);
}

TQAudioSourceDecodedMemory::~TQAudioSourceDecodedMemory() {
	ma_engine *engine = TQAudio::get_singleton()->get_engine();
	ma_resource_manager_unregister_data(ma_engine_get_resource_manager(engine), name.utf8());
}