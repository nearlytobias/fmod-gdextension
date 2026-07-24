#include "fmod_dsp.h"

#include "helpers/common.h"

#include <cstring>// This include is required for both Linux and MacOS targets as they don't include the necessary headers for 'memcpy' by default

using namespace godot;

void FmodDsp::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_valid"), &FmodDsp::is_valid);
    ClassDB::bind_method(D_METHOD("release"), &FmodDsp::release);
    ClassDB::bind_method(D_METHOD("set_parameter_int", "index", "value"), &FmodDsp::set_parameter_int);
    ClassDB::bind_method(D_METHOD("get_parameter_int", "index"), &FmodDsp::get_parameter_int);
    ClassDB::bind_method(D_METHOD("get_spectrum", "channel"), &FmodDsp::get_spectrum);
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

    // the pointer returned by FMOD is only valid until the next mix, so copy immediately.
    spectrum.resize(fft_data->length);
    memcpy(spectrum.ptrw(), fft_data->spectrum[channel], fft_data->length * sizeof(float));
    return spectrum;
}
