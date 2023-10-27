#ifndef TQAUDIO_SOUND_DATA
#define TQAUDIO_SOUND_DATA

#include "thirdparty/miniaudio/miniaudio.h"
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "core/string/ustring.h"
#include "tqaudio_group.h"
#include "tqaudio_player.h"

class TQAudioPlayer;

class TQAudioSource : public RefCounted 
{
    GDCLASS(TQAudioSource, RefCounted);

    protected:
        String error_message;
        String name;
        ma_result result;

        static void _bind_methods();
    public:
        virtual const String get_name() const;

        virtual const ma_result get_result() const;
        TQAudioPlayer *instantiate(Ref<TQAudioGroup> m_group, bool m_use_source_channel_count = false);

        TQAudioSource(String m_name);

        virtual Error instantiate_sound(Ref<TQAudioGroup> m_group, bool use_source_channel_count, ma_sound *p_sound) = 0;

        virtual ~TQAudioSource();
};

class TQAudioSourceMemory : public TQAudioSource
{
    GDCLASS(TQAudioSourceMemory, TQAudioSource);
    PackedByteArray data;

    public:
        virtual Error instantiate_sound(Ref<TQAudioGroup> m_group, bool use_source_channel_count, ma_sound *p_sound) override;
        TQAudioSourceMemory(String p_name, PackedByteArray p_in_data, bool encoded);
        ~TQAudioSourceMemory();
        friend class TQAudioPlayer;
};


#endif