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

    uint64_t start_time_msec = 0;
	uint64_t cached_length = -1;
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

        void schedule_start_time(uint64_t m_global_time_msec);
        void schedule_stop_time(uint64_t m_global_time_msec);

        int64_t get_playback_position_nsec();
        int64_t get_playback_position_msec();
        bool is_at_stream_end() const;
        bool is_playing() const;

        void set_volume(float m_linear_volume);
        float get_volume() const;
        Error seek(int64_t to_time_msec);
        uint64_t get_length_msec();

        void set_looping_enabled(bool m_looping);
        bool is_looping_enabled() const;

        Error connect_sound_to_group(Ref<TQAudioGroup> m_group);

        void fade(int p_duration_ms, float p_volume_begin, float p_volume_end);

	    uint64_t get_channel_count();

        TQAudioPlayer(Ref<TQAudioSource> m_sound_source, Ref<TQAudioGroup> m_group, bool m_use_source_channel_count);
	    ~TQAudioPlayer();
};

#endif