#include "tqaudio_player.h"
#include "tqaudio_macros.h"
#include "tqaudio.h"

void TQAudioPlayer::_bind_methods() 
{
    ClassDB::bind_method(D_METHOD("start"), &TQAudioPlayer::start);
	ClassDB::bind_method(D_METHOD("stop"), &TQAudioPlayer::stop);
	ClassDB::bind_method(D_METHOD("set_pitch_scale", "pitch_scale"), &TQAudioPlayer::set_pitch_scale);
	ClassDB::bind_method(D_METHOD("get_pitch_scale"), &TQAudioPlayer::get_pitch_scale);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "pitch_scale"), "set_pitch_scale", "get_pitch_scale");
	ClassDB::bind_method(D_METHOD("schedule_start_time", "global_time_sec"), &TQAudioPlayer::schedule_start_time);
	ClassDB::bind_method(D_METHOD("schedule_stop_time", "global_time_sec"), &TQAudioPlayer::schedule_stop_time);
	ClassDB::bind_method(D_METHOD("get_playback_position_nsec"), &TQAudioPlayer::get_playback_position_nsec);
	ClassDB::bind_method(D_METHOD("get_playback_position_sec"), &TQAudioPlayer::get_playback_position_sec);
	ClassDB::bind_method(D_METHOD("is_at_stream_end"), &TQAudioPlayer::is_at_stream_end);
	ClassDB::bind_method(D_METHOD("is_playing"), &TQAudioPlayer::is_playing);
	ClassDB::bind_method(D_METHOD("set_volume", "linear_volume"), &TQAudioPlayer::set_volume);
	ClassDB::bind_method(D_METHOD("get_volume"), &TQAudioPlayer::get_volume);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "volume"), "set_volume", "get_volume");
	ClassDB::bind_method(D_METHOD("set_looping_enabled", "looping"), &TQAudioPlayer::set_looping_enabled);
	ClassDB::bind_method(D_METHOD("is_looping_enabled"), &TQAudioPlayer::is_looping_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "looping_enabled"), "set_looping_enabled", "is_looping_enabled");
	ClassDB::bind_method(D_METHOD("connect_sound_to_group", "group"), &TQAudioPlayer::connect_sound_to_group);
	ClassDB::bind_method(D_METHOD("get_channel_count"), &TQAudioPlayer::get_channel_count);
	ClassDB::bind_method(D_METHOD("seek", "to_time_sec"), &TQAudioPlayer::seek);
	ClassDB::bind_method(D_METHOD("get_length_sec"), &TQAudioPlayer::get_length_sec);
	ClassDB::bind_method(D_METHOD("fade", "fade_duration", "volume_begin", "volume_end"), &TQAudioPlayer::fade);
}

Error TQAudioPlayer::start() {
	MA_ERR_RET(ma_sound_start(&sound), "Error starting sound");
	return OK;
}

Error TQAudioPlayer::stop() {
	MA_ERR_RET(ma_sound_stop(&sound), "Error stopping sound");
	return OK;
}

void TQAudioPlayer::set_pitch_scale(float m_pitch_scale) {
	if (sound_source->get_is_pitchable()) {
		ma_sound_set_pitch(&sound, m_pitch_scale);
	}
}

float TQAudioPlayer::get_pitch_scale() {
	if (sound_source->get_is_pitchable()) {
		return ma_sound_get_pitch(&sound);
	}

	return 1.0f;
}

void TQAudioPlayer::schedule_start_time(double m_global_time_sec) {
	start_time_sec = m_global_time_sec;
	ma_engine *engine = TQAudio::get_singleton()->get_engine();
	ma_sound_set_start_time_in_pcm_frames(&sound, m_global_time_sec * ma_engine_get_sample_rate(engine));
}

void TQAudioPlayer::schedule_stop_time(double m_global_time_sec) {
	ma_engine *engine = TQAudio::get_singleton()->get_engine();
	ma_sound_set_stop_time_in_pcm_frames(&sound, m_global_time_sec * ma_engine_get_sample_rate(engine));
}

int64_t TQAudioPlayer::get_playback_position_nsec() {
	Ref<TQAudioClock> clock = TQAudio::get_singleton()->get_clock();
	ma_engine *engine = TQAudio::get_singleton()->get_engine();

	ma_uint64 pos_frames = 0;
	ma_result result = ma_sound_get_cursor_in_pcm_frames(&sound, &pos_frames);
	int64_t out_pos = 0;
	uint32_t sample_rate;
	if (result == MA_SUCCESS) {
		result = ma_sound_get_data_format(&sound, NULL, NULL, &sample_rate, NULL, 0);
		if (result == MA_SUCCESS) {
			out_pos = (pos_frames * 1e+9) / sample_rate;
		}
	}

	// This allows the return of negative playback time
	// seconds to nanoseconds = x * 1_000_000_000
	// milliseconds to nanoseconds = x * 1_000_000
	uint64_t dsp_time_nsec = (ma_engine_get_time(engine) * 1e+9) / ma_engine_get_sample_rate(engine);
	uint64_t start_time_nsec = start_time_sec * 1e+9;

	if (!is_playing() && start_time_nsec > dsp_time_nsec) {
		return dsp_time_nsec - start_time_nsec + out_pos;
	}

	if (is_playing()) {
		out_pos += clock->get_current_offset_nsec();
	}

	return out_pos;
}

double TQAudioPlayer::get_playback_position_sec() {
	return get_playback_position_nsec() / 1e+9;
}

bool TQAudioPlayer::is_at_stream_end() const {
	return (bool)ma_sound_at_end(&sound);
}

bool TQAudioPlayer::is_playing() const {
	return (bool)ma_sound_is_playing(&sound);
}

void TQAudioPlayer::set_volume(float m_linear_volume) {
	ma_sound_set_volume(&sound, m_linear_volume);
}

float TQAudioPlayer::get_volume() const {
	return ma_sound_get_volume(&sound);
}

void TQAudioPlayer::set_looping_enabled(bool m_looping) {
	ma_sound_set_looping(&sound, m_looping);
}

bool TQAudioPlayer::is_looping_enabled() const {
	return (bool)ma_sound_is_looping(&sound);
}

uint64_t TQAudioPlayer::get_channel_count() {
	ma_uint32 channel_count;
	ma_sound_get_data_format(&sound, NULL, &channel_count, NULL, NULL, 0);
	return channel_count;
}

Error TQAudioPlayer::connect_sound_to_group(Ref<TQAudioGroup> m_group) {
	MA_ERR_RET(ma_node_attach_output_bus(&sound, 0, m_group->get_group(), 0), "Error attaching sound to group");
	return OK;
}

void TQAudioPlayer::fade(double p_duration_s, float p_volume_begin, float p_volume_end) {
	ma_engine *engine = TQAudio::get_singleton()->get_engine();
	ma_sound_set_fade_in_pcm_frames(&sound, p_volume_begin, p_volume_end, p_duration_s * ma_engine_get_sample_rate(engine));
}

void TQAudioPlayer::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PAUSED: {
			if (!can_process()) {
				was_playing_before_pause = is_playing();
				ma_sound_stop(&sound);
			}
		} break;
		case NOTIFICATION_UNPAUSED: {
			if (was_playing_before_pause && !is_at_stream_end()) {
				ma_sound_start(&sound);
			}
		} break;
	}
}

Error TQAudioPlayer::seek(double to_time_sec) {
	// Sound MUST be stopped before seeking or we crash
	if (ma_sound_is_playing(&sound) == MA_TRUE) {
		ma_sound_stop(&sound);
	}
	uint32_t sample_rate;
	ma_sound_get_data_format(&sound, NULL, NULL, &sample_rate, NULL, 0);
	ma_result result = ma_sound_seek_to_pcm_frame(&sound, MAX(0.0, to_time_sec * (double)sample_rate));
	MA_ERR_RET(result, "Error seeking sound");
	return OK;
}

double TQAudioPlayer::get_length_sec() {
	if (cached_length != -1) {
		return cached_length;
	}

	ma_uint64 p_length = 0;
	ma_sound_get_length_in_pcm_frames(&sound, &p_length);
	uint32_t sample_rate;
	ma_format format;
	ma_sound_get_data_format(&sound, &format, NULL, &sample_rate, NULL, 0);
	cached_length = p_length / (double)sample_rate;

	return p_length / (double)sample_rate;
}

TQAudioPlayer::TQAudioPlayer(Ref<TQAudioSource> m_sound_source, Ref<TQAudioGroup> m_group, bool m_use_source_channel_count) {
	m_sound_source->instantiate_sound(m_group, m_use_source_channel_count, &sound);
	sound_source = m_sound_source;
}

TQAudioPlayer::~TQAudioPlayer() {
	ma_sound_uninit(&sound);
}