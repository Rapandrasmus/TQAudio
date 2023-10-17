#ifndef TQAUDIO_H
#define TQAUDIO_H

#include "core/object/ref_counted.h"
#include "thirdParty/miniaudio/miniaudio.h"

class TQAudio : public RefCounted
{
	GDCLASS(TQAudio, RefCounted);

	protected:
		static void _bind_methods();
		ma_engine *engine;
		ma_device *device;

	public:
		TQAudio();
		~TQAudio();
};

#endif
