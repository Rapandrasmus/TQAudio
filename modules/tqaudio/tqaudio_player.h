#ifndef TQAUDIO_PLAYER_H
#define TQAUDIO_PLAYER_H

#include "scene/main/node.h"
#include "thirdparty/miniaudio/miniaudio.h"
#include <memory>
#include "tqaudio_source.h"
#include "tqaudio_group.h"

class TQAudioSource;

class TQAudioPlayer : public Node 
{
    GDCLASS(TQAudioPlayer, Node);

    Ref<TQAudioSource> sound_source;
    ma_sound sound;
    String error_message;

    double start_time_sec = 0.0;
	double cached_length = -1.0;
	// HACK-ish way of dealing with tree pauses
	bool was_playing_before_pause = false;

    protected:
        void _notification(int p_notification);
        static void _bind_methods();

    public:
        Error start();
        Error stop();
        void set_pitch_scale(float m_pitch_scale);
        float get_pitch_scale();

        void schedule_start_time(double m_global_time_sec);
        void schedule_stop_time(double m_global_time_sec);

        int64_t get_playback_position_nsec();
        double get_playback_position_sec();
        bool is_at_stream_end() const;
        bool is_playing() const;

        void set_volume(float m_linear_volume);
        float get_volume() const;
        Error seek(double to_time_sec);
        double get_length_sec();

        void set_looping_enabled(bool m_looping);
        bool is_looping_enabled() const;

        Error connect_sound_to_group(Ref<TQAudioGroup> m_group);

        void fade(double p_duration_s, float p_volume_begin, float p_volume_end);

	    uint64_t get_channel_count();

        TQAudioPlayer(Ref<TQAudioSource> m_sound_source, Ref<TQAudioGroup> m_group, bool m_use_source_channel_count);
	    ~TQAudioPlayer();
};

#endif