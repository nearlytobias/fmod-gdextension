#ifndef GODOTFMOD_FMOD_CHANNEL_GROUP_H
#define GODOTFMOD_FMOD_CHANNEL_GROUP_H

#include "classes/ref_counted.hpp"
#include "fmod.hpp"
#include "core/fmod_dsp.h"

namespace godot {
    class FmodChannelGroup : public RefCounted {
        GDCLASS(FmodChannelGroup, RefCounted);

        FMOD::ChannelGroup* _wrapped = nullptr;

    public:
        inline static Ref<FmodChannelGroup> create_ref(FMOD::ChannelGroup* wrapped) {
            Ref<FmodChannelGroup> ref;
            if (wrapped) {
                ref.instantiate();
                ref->_wrapped = wrapped;
                wrapped->setUserData(ref.ptr());
            }
            return ref;
        }

        uint64_t get_dsp_clock() const;
        uint64_t get_parent_dsp_clock() const;
        void add_dsp(int index, const Ref<FmodDsp>& dsp) const;
        void remove_dsp(const Ref<FmodDsp>& dsp) const;

    protected:
        static void _bind_methods();
    };
}// namespace godot

#endif// GODOTFMOD_FMOD_CHANNEL_GROUP_H
