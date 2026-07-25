#include "core/fmod_dsp.h"

#include "callback/dsp_capture_callbacks.h"
#include "helpers/common.h"

#include <algorithm>
#include <cstring>// This include is required for both Linux and MacOS targets as they don't include the necessary headers for 'memcpy' by default

using namespace godot;

void FmodDsp::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_valid"), &FmodDsp::is_valid);
    ClassDB::bind_method(D_METHOD("release"), &FmodDsp::release);
    ClassDB::bind_method(D_METHOD("set_parameter_int", "index", "value"), &FmodDsp::set_parameter_int);
    ClassDB::bind_method(D_METHOD("get_parameter_int", "index"), &FmodDsp::get_parameter_int);
    ClassDB::bind_method(D_METHOD("get_spectrum", "channel"), &FmodDsp::get_spectrum);
    ClassDB::bind_method(D_METHOD("get_capture_channels"), &FmodDsp::get_capture_channels);
    ClassDB::bind_method(D_METHOD("get_capture_data", "channel"), &FmodDsp::get_capture_data);
}

void FmodDsp::release() const {
    ERROR_CHECK(_wrapped->release());
}

bool FmodDsp::is_valid() const {
    bool active = false;
    FMOD_RESULT result = _wrapped->getActive(&active);
    return result != FMOD_ERR_INVALID_HANDLE;
}

void FmodDsp::set_parameter_int(int index, int value) const {
    ERROR_CHECK_WITH_REASON(_wrapped->setParameterInt(index, value), vformat("Cannot set dsp parameter %d to %d", index, value));
}

int FmodDsp::get_parameter_int(int index) const {
    int value = 0;
    ERROR_CHECK_WITH_REASON(_wrapped->getParameterInt(index, &value, nullptr, 0), vformat("Cannot get dsp parameter %d", index));
    return value;
}

PackedFloat32Array FmodDsp::get_spectrum(int channel) const {
    PackedFloat32Array spectrum;
    FMOD_DSP_PARAMETER_FFT* fft_data = nullptr;
    if (!ERROR_CHECK_WITH_REASON(_wrapped->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void**) &fft_data, nullptr, nullptr, 0),
                                 vformat("Cannot get spectrum data"))) {
        return spectrum;
    }

    if (!fft_data || channel < 0 || channel >= fft_data->numchannels || !fft_data->spectrum[channel]) { return spectrum; }

    // The pointer returned by FMOD is only valid until the next mix, so copy immediately.
    spectrum.resize(fft_data->length);
    memcpy(spectrum.ptrw(), fft_data->spectrum[channel], fft_data->length * sizeof(float));
    return spectrum;
}

int FmodDsp::get_capture_channels() const {
    Callbacks::DspCaptureData* capture_data = nullptr;
    if (!ERROR_CHECK_WITH_REASON(_wrapped->getParameterData(Callbacks::DSP_CAPTURE_PARAM_DATA, (void**) &capture_data, nullptr, nullptr, 0),
                                 vformat("Cannot get capture data"))) {
        return 0;
    }

    if (!capture_data) { return 0; }

    // 0 until the first mix reaches this DSP.
    return capture_data->channels.load(std::memory_order_relaxed);
}

PackedFloat32Array FmodDsp::get_capture_data(int channel) const {
    PackedFloat32Array samples;
    Callbacks::DspCaptureData* capture_data = nullptr;
    if (!ERROR_CHECK_WITH_REASON(_wrapped->getParameterData(Callbacks::DSP_CAPTURE_PARAM_DATA, (void**) &capture_data, nullptr, nullptr, 0),
                                 vformat("Cannot get capture data"))) {
        return samples;
    }

    if (!capture_data) { return samples; }

    // Read channels after this load so both are ordered against the audio thread's release store.
    const size_t total_frames = capture_data->total_frames.load(std::memory_order_acquire);
    const int channels = capture_data->channels.load(std::memory_order_relaxed);
    if (channel < 0 || channel >= channels) { return samples; }

    const size_t valid_frames = std::min(total_frames, Callbacks::DSP_CAPTURE_RING_FRAMES);
    if (valid_frames == 0) { return samples; }

    // Read the ring back in chronological order, de-interleaving the requested channel.
    const size_t oldest_frame = total_frames - valid_frames;
    samples.resize(static_cast<int64_t>(valid_frames));
    float* dst = samples.ptrw();
    for (size_t i = 0; i < valid_frames; i++) {
        const size_t frame = (oldest_frame + i) & (Callbacks::DSP_CAPTURE_RING_FRAMES - 1);
        dst[i] = capture_data->ring[(frame * channels) + channel];
    }
    return samples;
}
