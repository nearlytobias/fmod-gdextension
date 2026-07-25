#include "dsp_capture_callbacks.h"

#include <cstring>// This include is required for both Linux and MacOS targets as they don't include the necessary headers for 'memcpy' by default

namespace Callbacks {

    // ring capacity in frames: ~85ms at 48kHz, enough history to span a full cycle of
    // low-frequency content (down to ~12Hz) so a reader-side trigger search can always
    // find a zero-crossing, regardless of the mix block size.
    static constexpr size_t RING_FRAMES = 4096;

    FMOD_RESULT F_CALL dsp_capture_create(FMOD_DSP_STATE* dsp_state) {
        dsp_state->plugindata = new DspCaptureData();
        return FMOD_OK;
    }

    FMOD_RESULT F_CALL dsp_capture_release(FMOD_DSP_STATE* dsp_state) {
        delete static_cast<DspCaptureData*>(dsp_state->plugindata);
        return FMOD_OK;
    }

    FMOD_RESULT F_CALL dsp_capture_read(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels) {
        auto* data = static_cast<DspCaptureData*>(dsp_state->plugindata);

        {
            std::lock_guard<std::mutex> lock(data->mutex);
            if (data->channels != inchannels) {
                data->channels = inchannels;
                data->ring.assign(RING_FRAMES * inchannels, 0.0f);
                data->write_pos = 0;
                data->wrapped = false;
            }
            for (unsigned int frame = 0; frame < length; frame++) {
                memcpy(&data->ring[data->write_pos * inchannels], &inbuffer[frame * inchannels], inchannels * sizeof(float));
                data->write_pos = (data->write_pos + 1) % RING_FRAMES;
                if (data->write_pos == 0) { data->wrapped = true; }
            }
        }

        // pass the signal through unmodified so capture doesn't affect playback.
        memcpy(outbuffer, inbuffer, length * inchannels * sizeof(float));

        return FMOD_OK;
    }

    FMOD_RESULT F_CALL dsp_capture_get_parameter_data(FMOD_DSP_STATE* dsp_state, int index, void** data, unsigned int* length, char* valuestr) {
        if (index != 0) { return FMOD_ERR_INVALID_PARAM; }

        *data = dsp_state->plugindata;
        return FMOD_OK;
    }
}// namespace Callbacks
