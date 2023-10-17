#include "tqaudio.h"

#define MINIAUDIO_IMPLEMENTATION
#include "thirdParty/miniaudio/miniaudio.h"

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    // In playback mode copy data to pOutput. In capture mode read data from pInput. In full-duplex mode, both
    // pOutput and pInput will be valid and you can move data from pInput into pOutput. Never process more than
    // frameCount frames.
}

void TQAudio::_bind_methods()
{
    
}

//Just testing miniaudio rn, make this shit better when i figure out what the hell i am doing
TQAudio::TQAudio() 
{
    device = new ma_device;
    engine = new ma_engine;

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = ma_format_f32;   
    config.playback.channels = 2;               
    config.sampleRate        = 48000;           
    config.dataCallback      = data_callback;

    if (ma_device_init(NULL, &config, device) != MA_SUCCESS) 
    {
        printf("Device initialization failed.");
        return;
    }

    if (ma_engine_init(NULL, engine) != MA_SUCCESS) 
    {
        printf("Engine initialization failed.");
        return;
    }

    ma_device_start(device);
}

TQAudio::~TQAudio() 
{
    ma_device_uninit(device);
    ma_engine_uninit(engine);
    delete device;
    delete engine;
}
