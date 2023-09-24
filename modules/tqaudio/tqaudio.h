#ifndef TQAUDIO_H
#define TQAUDIO_H

#include "core/object/ref_counted.h"
#include "miniaudio.h"

class TQAudio : public RefCounted
{
	GDCLASS(TQAudio, RefCounted);

	protected:
		static void _bind_methods();
		ma_engine *engine;
		ma_device *device;

	public:
		void play_sound(String p_file);
		TQAudio();
		~TQAudio();
};

#endif
