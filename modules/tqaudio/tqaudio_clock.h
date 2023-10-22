#ifndef TQAUDIO_CLOCK_H
#define TQAUDIO_CLOCK_H

#include <atomic>
#include <chrono>

#include "core/object/ref_counted.h"
#include "core/templates/safe_refcount.h"

typedef std::chrono::nanoseconds nanoseconds;

class TQAudioClock : public RefCounted
{
    SafeNumeric<uint64_t> last_recorded_time;

    public:
        TQAudioClock() {
		    last_recorded_time.set(0);
        }

        uint64_t get_current_offset_nsec() {
            std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
            std::chrono::high_resolution_clock::time_point lrt(nanoseconds(last_recorded_time.get()));
            auto diff = std::chrono::duration_cast<nanoseconds>(now - lrt);
            return diff.count();
        }

        void measure() {
            uint64_t ns_count = std::chrono::time_point_cast<nanoseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
            last_recorded_time.set(ns_count);
        }
};

#endif