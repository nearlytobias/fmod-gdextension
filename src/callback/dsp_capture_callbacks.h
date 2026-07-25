#ifndef GODOTFMOD_DSP_CAPTURE_CALLBACKS_H
#define GODOTFMOD_DSP_CAPTURE_CALLBACKS_H

#include <fmod.hpp>
#include <fmod_dsp.h>

#include <mutex>
#include <vector>

namespace Callbacks {
    struct DspCaptureData {
        std::mutex mutex;
        std::vector<float> ring;// interleaved ring buffer of the most recent frames
        size_t write_pos = 0;// next frame index to write, in frames (not samples)
        int channels = 0;
        bool wrapped = false;// true once the ring has been fully written at least once
    };

    FMOD_RESULT F_CALL dsp_capture_create(FMOD_DSP_STATE* dsp_state);
    FMOD_RESULT F_CALL dsp_capture_release(FMOD_DSP_STATE* dsp_state);
    FMOD_RESULT F_CALL dsp_capture_read(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels);
    FMOD_RESULT F_CALL dsp_capture_get_parameter_data(FMOD_DSP_STATE* dsp_state, int index, void** data, unsigned int* length, char* valuestr);
}// namespace Callbacks

#endif// GODOTFMOD_DSP_CAPTURE_CALLBACKS_H
