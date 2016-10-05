#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include <array>
#include <assert.h>
#include <math.h>

#include "lib_vst/pluginbase.h"
#include "lib_vst/utility.h"

namespace {
enum {
    e_channel,
    e_param_count__
};

const std::array<const char *, e_param_count__> g_param_names = {
    "channel",
};

const std::array<float, e_param_count__> g_default = {
    0.0f,
};

plugin_base_t::config_t g_config = {
    0x12345678,    // uint32_t uuid_;
    0,             // uint32_t audio_inputs_;
    0,             // uint32_t audio_outputs_;
    1,             // uint32_t midi_inputs_;
    1,             // uint32_t midi_outputs_;
    false          // bool     is_synth_;
};
} // namespace {}

class vst_channel_t: public plugin_base_t {
public:

    source_chip::audio_source_chip_t source_chip_;

    std::array<float, e_param_count__> params_;
    programs_t progs_;

    virtual void on_reset() override {
        sample_rate_ = 44100.f;
        for (size_t i = 0; i<e_param_count__; ++i) {
            params_[i] = g_default[i];
        }
    }

    vst_chip_t(audioMasterCallback audioMaster)
        : plugin_base_t(g_config,
                        plugin_base_t::params_t{e_param_count__, 
                                                &params_[0],
                                                &g_param_names[0]},
                        plugin_base_t::programs_t{0},
                        audioMaster)
    {
        on_reset();
    }

    uint8_t getOuputChannel() const {
        return uint8_t(params_[e_channel] * 16) & 0xf;
    }

    virtual void event_note_on(uint8_t channel,
                               uint8_t note,
                               uint8_t velocity) override {

        const uint8_t out_chan = getOutputChannel();
    }

    virtual void event_note_off(uint8_t channel,
                                uint8_t note,
                                uint8_t velocity) override {
    }

    virtual void event_control_change(uint8_t channel,
                                      uint8_t control,
                                      uint8_t value) override {
    }
};

extern AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
    return new vst_channel_t(audioMaster);
}

extern "C" {
__declspec(dllexport) AEffect* VSTPluginMain(audioMasterCallback audioMaster) {
    if (!audioMaster(0, audioMasterVersion, 0, 0, 0, 0)) {
        return nullptr;
    }
    AudioEffect* effect = createEffectInstance(audioMaster);
    if (!effect) {
        return nullptr;
    }
    return effect->getAeffect();
}
}; // extern "C"
