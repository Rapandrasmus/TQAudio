#include "register_types.h"
#include "core/object/class_db.h"
#include "core/config/engine.h"
#include "tqaudio.h"

void initialize_tqaudio_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) 
    {
        return;
    }
    ClassDB::register_class<TQAudio>();
}

void uninitialize_tqaudio_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) 
    {
        return;
    }
}