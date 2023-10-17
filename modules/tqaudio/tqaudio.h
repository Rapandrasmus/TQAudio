#ifndef TQAUDIO_H
#define TQAUDIO_H

#include "core/object/ref_counted.h"
#include "core/object/object.h"
#include "thirdParty/miniaudio/miniaudio.h"

class TQAudio : public Object
{
	GDCLASS(TQAudio, Object);

	private:
		static TQAudio *singleton;
		static ma_backend string_to_backend(String str);

		ma_engine engine;
		ma_device device;
		ma_resource_manager resource_manager;
		ma_context context;
		String error_message;
		uint64_t desired_buffer_size_msec = 10;

		static void ma_data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount);

		float master_volume = 1.0f;
		bool initialized = false;

	protected:
		static void _bind_methods();

	public:
		_FORCE_INLINE_ static TQAudio *get_singleton() { return singleton; }

		String get_initialization_error() const;

		Error initialize(ma_backend forced_backend);
		Error godot_initialize();

		TQAudio();
		~TQAudio();
};

#endif
