#ifndef TQAUDIO_GROUP_H
#define TQAUDIO_GROUP_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "thirdparty/miniaudio/miniaudio.h"
#include <memory>

class TQAudioGroup : public RefCounted 
{
    GDCLASS(TQAudioGroup, RefCounted);
    String name;
	String error_message;
	ma_sound_group group;

    protected:
        static void _bind_methods();
    public:
        ma_sound_group *get_group();
        void set_volume(float m_linear_volume);
        float get_volume() const;

        TQAudioGroup(String m_group_name, Ref<TQAudioGroup> m_parent_group);
        ~TQAudioGroup();
};

#endif