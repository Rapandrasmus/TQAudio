#include "register_types.h"
#include "core/object/class_db.h"
#include "core/config/engine.h"
#include "tqaudio.h"

static TQAudio *tqaudio_ptr = NULL;

void initialize_tqaudio_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) 
    {
        return;
    }
    GDREGISTER_CLASS(TQAudio);
    tqaudio_ptr = memnew(TQAudio);
    Engine::get_singleton()->add_singleton(Engine::Singleton("TQAudio", TQAudio::get_singleton()));
}

void uninitialize_tqaudio_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) 
    {
        return;
    }
    memdelete(tqaudio_ptr);
}