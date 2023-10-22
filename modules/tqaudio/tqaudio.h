#ifndef TQAUDIO_H
#define TQAUDIO_H

#include <memory>
#include <vector>

#include "core/object/object.h"
#include "thirdparty/miniaudio/miniaudio.h"
#include "tqaudio_source.h"
#include "tqaudio_group.h"
#include "tqaudio_clock.h"

class TQAudio : public Object
{
	GDCLASS(TQAudio, Object);

	private:
		static SafeNumeric<uint64_t> sound_source_uid;

		Ref<TQAudioClock> clock;

		Vector<Ref<TQAudioGroup>> groups;

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
		_FORCE_INLINE_ static uint64_t get_inc_sound_source_uid() { return sound_source_uid.postincrement(); };

		Ref<TQAudioClock> get_clock();
		ma_engine *get_engine();

		String get_initialization_error() const;

		Ref<TQAudioSourceMemory> register_sound_from_memory(String m_name_hint, PackedByteArray m_data);
		Ref<TQAudioGroup> create_group(String m_group_name, Ref<TQAudioGroup> m_parent_group = nullptr);

		Error initialize(ma_backend forced_backend);
		Error godot_initialize();

		void set_desired_buffer_size_msec(uint64_t m_new_buffer_size);
		uint64_t get_desired_buffer_size_msec() const;

		Error set_master_volume(float m_linear_volume);
		float get_master_volume();

		uint64_t get_dsp_time() const;
		Error set_dsp_time(uint64_t m_new_time_msec);

		uint64_t get_actual_buffer_size() const;
		String get_current_backend_name() const;

		TQAudio();
		~TQAudio();
};

#endif
