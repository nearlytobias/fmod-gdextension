#ifndef GODOTFMOD_FMOD_DSP_H
#define GODOTFMOD_FMOD_DSP_H

#include "classes/ref_counted.hpp"
#include "fmod.hpp"
#include "variant/packed_float32_array.hpp"

namespace godot {
    class FmodDsp : public RefCounted {
        GDCLASS(FmodDsp, RefCounted);

        FMOD::DSP* _wrapped = nullptr;

    public:
        inline static Ref<FmodDsp> create_ref(FMOD::DSP* wrapped) {
            Ref<FmodDsp> ref;
            if (wrapped) {
                ref.instantiate();
                ref->_wrapped = wrapped;
            }
            return ref;
        }

        FMOD::DSP* get_wrapped() const {
            return _wrapped;
        }

        void release() const;
        void set_parameter_int(int index, int value) const;
        int get_parameter_int(int index) const;
        PackedFloat32Array get_spectrum(int channel) const;

    protected:
        static void _bind_methods();
    };
}// namespace godot

#endif// GODOTFMOD_FMOD_DSP_H
