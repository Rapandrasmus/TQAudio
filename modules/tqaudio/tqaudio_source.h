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
        bool is_pitchable;

        static void _bind_methods();
    public:
        virtual const String get_name() const;
        virtual const bool get_is_pitchable() const;
        virtual const ma_result get_result() const;
        TQAudioPlayer *instantiate(Ref<TQAudioGroup> m_group, bool m_use_source_channel_count = false);

        TQAudioSource(String m_name, bool m_is_pitchable);

        virtual Error instantiate_sound(Ref<TQAudioGroup> m_group, bool use_source_channel_count, ma_sound *p_sound) = 0;

        virtual ~TQAudioSource();
};

class TQAudioSourceEncodedMemory : public TQAudioSource
{
    GDCLASS(TQAudioSourceEncodedMemory, TQAudioSource);
    PackedByteArray data;

    public:
        virtual Error instantiate_sound(Ref<TQAudioGroup> m_group, bool use_source_channel_count, ma_sound *p_sound) override;
        TQAudioSourceEncodedMemory(String p_name, bool m_is_pitchable, PackedByteArray p_in_data);
        ~TQAudioSourceEncodedMemory();
        friend class TQAudioPlayer;
};

class TQAudioSourceDecodedMemory : public TQAudioSource
{
    GDCLASS(TQAudioSourceDecodedMemory, TQAudioSource);
    PackedByteArray data;
    ma_uint64 frameCount;
    ma_format format;
    ma_uint32 channels;
    ma_uint32 sampleRate;

    public:
        virtual Error instantiate_sound(Ref<TQAudioGroup> m_group, bool use_source_channel_count, ma_sound *p_sound) override;
        TQAudioSourceDecodedMemory(String p_name, bool m_is_pitchable, PackedByteArray p_in_data, ma_uint64 m_frameCount, ma_format m_format, ma_uint32 m_channels, ma_uint32 m_sampleRate);
        ~TQAudioSourceDecodedMemory();
        friend class TQAudioPlayer;
};
#endif