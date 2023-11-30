#include "tqaudio.h"
#include "tqaudio_group.h"
#include "tqaudio_macros.h"

void TQAudioGroup::_bind_methods() 
{
	ClassDB::bind_method(D_METHOD("set_volume", "linear_volume"), &TQAudioGroup::set_volume);
	ClassDB::bind_method(D_METHOD("get_volume"), &TQAudioGroup::get_volume);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "volume"), "set_volume", "get_volume");
}

ma_sound_group *TQAudioGroup::get_group() {
	return &group;
}

void TQAudioGroup::set_volume(float m_linear_volume) {
	ma_sound_group_set_volume(&group, m_linear_volume);
}

float TQAudioGroup::get_volume() const {
	return ma_sound_group_get_volume(&group);
}

TQAudioGroup::TQAudioGroup(String m_group_name, Ref<TQAudioGroup> m_parent_group)
{
    ma_engine *engine = TQAudio::get_singleton()->get_engine();

    ma_sound_group *parent_group = nullptr;

    if (!m_parent_group.is_null() && m_parent_group.is_valid()) 
    {
        parent_group = m_parent_group->get_group();
	}

    MA_ERR(ma_sound_group_init(engine, 0, parent_group, &group), "Error creating group.");
}

TQAudioGroup::~TQAudioGroup() 
{
    ma_sound_group_uninit(&group);
}