#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include <array>
#include <assert.h>
#include <math.h>

#include "../lib_vst/pluginbase.h"
#include "../lib_vst/utility.h"
#include "../../framework_audio/source/chip/source_chip.h"

namespace {
enum {
    e_gain,
    e_source,
    e_param_count__
};

const std::array<const char *, e_param_count__> g_param_names = {
    "gain",
    "source"
};

const std::array<float, e_param_count__> g_default = {
    0.5f,
    0.f,
};

plugin_base_t::config_t g_config = {
    0x12345678,    // uint32_t uuid_;
    0,             // uint32_t audio_inputs_;
    1,             // uint32_t audio_outputs_;
    1,             // uint32_t midi_inputs_;
    0,             // uint32_t midi_outputs_;
    true           // bool     is_synth_;
};
} // namespace {}

class vst_chip_t: public plugin_base_t {
public:

    tengu::source_chip::audio_source_chip_t source_chip_;

    std::array<float, e_param_count__> params_;
    programs_t progs_;

    virtual void on_reset() override {
        sample_rate_ = 44100.f;
        for (size_t i = 0; i<e_param_count__; ++i) {
            params_[i] = g_default[i];
        }
    }

    vst_chip_t(audioMasterCallback audioMaster)
        : plugin_base_t(
            g_config,
            plugin_base_t::params_t{
                e_param_count__, 
                &params_[0],
                &g_param_names[0]
            },
            plugin_base_t::programs_t{0},
            audioMaster
        )
        , source_chip_(sample_rate_)
    {
        on_reset();
        onParamChange(e_source);
    }

    virtual void onParamChange(VstInt32 index) override {
        using namespace tengu;

        if (index==e_source) {
            source_chip::config_t config;
            source_chip::config_t::entry_t entry {
                source_chip::config_t::e_pulse, 0
            };
            const uint32_t source = clampv(0u, uint32_t(params_[e_source]*3), 2u);
            switch (source) {
            case (0):
                entry.type_ = source_chip::config_t::e_pulse;
                break;
            case (1):
                entry.type_ = source_chip::config_t::e_nestr;
                break;
            case (2):
                entry.type_ = source_chip::config_t::e_noise;
                break;
            }
            config += entry;
            source_chip_.init(config);
        }
    };

    virtual void processReplacing(float** inputs, 
                                  float** outputs, 
                                  VstInt32 samples) override {
        float * left = outputs[0];
        source_chip_.render(left, samples);
    }

    virtual void event_note_on(uint8_t channel,
                               uint8_t note,
                               uint8_t velocity) override {
        using namespace tengu;

        source_chip::event_t event;
        event.type_ = event.e_note_on;
        event.data_[0] = channel;
        event.data_[1] = note;
        event.data_[2] = velocity;
        source_chip_.push(event);
    }

    virtual void event_note_off(uint8_t channel,
                                uint8_t note,
                                uint8_t velocity) override {
        using namespace tengu;

        source_chip::event_t event;
        event.type_ = event.e_note_off;
        event.data_[0] = channel;
        event.data_[1] = note;
        event.data_[2] = velocity;
        source_chip_.push(event);
    }

    virtual void event_control_change(uint8_t channel,
                                      uint8_t control,
                                      uint8_t value) override {
        using namespace tengu;

        source_chip::event_t event;
        event.type_ = event.e_cc;
        event.data_[0] = channel;
        event.data_[1] = control;
        event.data_[2] = value * 2 /* orig. range: 127 */;
        source_chip_.push(event);
    }
};

extern AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
    return new vst_chip_t(audioMaster);
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
