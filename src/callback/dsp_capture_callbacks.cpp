#include "dsp_capture_callbacks.h"

#include <algorithm>
#include <cstring>// This include is required for both Linux and MacOS targets as they don't include the necessary headers for 'memcpy' by default

namespace Callbacks {

    FMOD_RESULT F_CALL dsp_capture_create(FMOD_DSP_STATE* dsp_state) {
        dsp_state->plugindata = new DspCaptureData();
        return FMOD_OK;
    }

    FMOD_RESULT F_CALL dsp_capture_release(FMOD_DSP_STATE* dsp_state) {
        delete static_cast<DspCaptureData*>(dsp_state->plugindata);
        dsp_state->plugindata = nullptr;
        return FMOD_OK;
    }

    FMOD_RESULT F_CALL dsp_capture_read(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels) {
        auto* data = static_cast<DspCaptureData*>(dsp_state->plugindata);

        // Pass the signal through unmodified so capture doesn't affect playback.
        memcpy(outbuffer, inbuffer, length * inchannels * sizeof(float));

        if (inchannels <= 0 || inchannels > DSP_CAPTURE_MAX_CHANNELS) { return FMOD_OK; }

        // A channel count change invalidates the history, as it was written with a different stride.
        size_t total = data->total_frames.load(std::memory_order_relaxed);
        if (data->channels.load(std::memory_order_relaxed) != inchannels) {
            data->channels.store(inchannels, std::memory_order_relaxed);
            total = 0;
        }

        // Skip any frames a larger block would immediately overwrite.
        const float* src = inbuffer;
        unsigned int remaining = length;
        if (remaining > DSP_CAPTURE_RING_FRAMES) {
            const unsigned int skipped = remaining - static_cast<unsigned int>(DSP_CAPTURE_RING_FRAMES);
            src += static_cast<size_t>(skipped) * inchannels;
            total += skipped;
            remaining = static_cast<unsigned int>(DSP_CAPTURE_RING_FRAMES);
        }

        // Copy in at most two contiguous runs so the wrap costs no per-frame work.
        while (remaining > 0) {
            const size_t offset = total & (DSP_CAPTURE_RING_FRAMES - 1);
            const size_t run = std::min(static_cast<size_t>(remaining), DSP_CAPTURE_RING_FRAMES - offset);
            memcpy(&data->ring[offset * inchannels], src, run * inchannels * sizeof(float));
            src += run * inchannels;
            total += run;
            remaining -= static_cast<unsigned int>(run);
        }

        // Publishes the frames and channel count above to the reader.
        data->total_frames.store(total, std::memory_order_release);

        return FMOD_OK;
    }

    FMOD_RESULT F_CALL dsp_capture_get_parameter_data(FMOD_DSP_STATE* dsp_state, int index, void** data, unsigned int* length, char* valuestr) {
        if (index != DSP_CAPTURE_PARAM_DATA) { return FMOD_ERR_INVALID_PARAM; }

        *data = dsp_state->plugindata;
        if (length) { *length = sizeof(DspCaptureData); }
        return FMOD_OK;
    }
}// namespace Callbacks
