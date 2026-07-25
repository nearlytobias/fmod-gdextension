#ifndef GODOTFMOD_DSP_CAPTURE_CALLBACKS_H
#define GODOTFMOD_DSP_CAPTURE_CALLBACKS_H

#include <fmod.hpp>
#include <fmod_dsp.h>

#include <atomic>
#include <cstddef>

namespace Callbacks {
    // ~85ms of history at 48kHz. Must be a power of two and larger than the DSP block size.
    inline constexpr size_t DSP_CAPTURE_RING_FRAMES = 4096;

    // Sized for the widest speaker mode so the audio thread never allocates.
    inline constexpr int DSP_CAPTURE_MAX_CHANNELS = 8;

    inline constexpr int DSP_CAPTURE_PARAM_DATA = 0;

    // Interleaved ring buffer written by the audio thread and read by the game thread.
    struct DspCaptureData {
        float ring[DSP_CAPTURE_RING_FRAMES * DSP_CAPTURE_MAX_CHANNELS] = {};
        // Total frames ever written. Masked with RING_FRAMES - 1 to get the write offset.
        std::atomic<size_t> total_frames {0};
        std::atomic<int> channels {0};
    };

    // The audio thread must not block, so these cannot fall back to a mutex.
    static_assert(std::atomic<size_t>::is_always_lock_free);
    static_assert(std::atomic<int>::is_always_lock_free);

    FMOD_RESULT F_CALL dsp_capture_create(FMOD_DSP_STATE* dsp_state);
    FMOD_RESULT F_CALL dsp_capture_release(FMOD_DSP_STATE* dsp_state);
    FMOD_RESULT F_CALL dsp_capture_read(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels);
    FMOD_RESULT F_CALL dsp_capture_get_parameter_data(FMOD_DSP_STATE* dsp_state, int index, void** data, unsigned int* length, char* valuestr);
}// namespace Callbacks

#endif// GODOTFMOD_DSP_CAPTURE_CALLBACKS_H
