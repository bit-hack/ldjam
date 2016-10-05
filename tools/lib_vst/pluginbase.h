#pragma once
#include <stdint.h>
#include <assert.h>

#include "audioeffectx.h"

class plugin_base_t: public AudioEffectX {
public:
    enum : uint8_t {
        c_note_off       = 0x80,
        c_note_on        = 0x90,
        c_aftertouch     = 0xa0,
        c_control_change = 0xb0,
        c_pitch_bend     = 0xe0,
    };

    struct config_t {
        uint32_t uuid_;
        uint32_t audio_inputs_;
        uint32_t audio_outputs_;
        uint32_t midi_inputs_;
        uint32_t midi_outputs_;
        bool     is_synth_;
    };

    struct params_t {
        uint32_t count_;
        float * value_;
        const char * const * name_;
    };

    struct programs_t {
        uint32_t count_;
    };

    const config_t & vst_config_;
    params_t vst_param_;
    programs_t vst_prog_;

    float sample_rate_;

    plugin_base_t(const config_t & config,
                  params_t params,
                  programs_t progs,
                  audioMasterCallback master)
        : vst_config_(config)
        , vst_param_(params)
        , vst_prog_(progs)
        , sample_rate_(44100.f)
        , AudioEffectX(master, progs.count_, params.count_)
    {
        on_reset();
        setNumInputs(config.audio_inputs_);
        setNumOutputs(config.audio_outputs_);
        setUniqueID(config.uuid_);
        canProcessReplacing();
        if (config.is_synth_) {
            isSynth();
        }
    }

    virtual void on_reset() {}
    virtual void event_note_on(uint8_t channel, uint8_t note, uint8_t velocity) {}
    virtual void event_note_off(uint8_t channel, uint8_t note, uint8_t velocity) {}
    virtual void event_pitch_bend(uint8_t channel, int32_t pitch) {}
    virtual void event_control_change(uint8_t channel, uint8_t control, uint8_t value) {}

    virtual VstIntPtr dispatcher(VstInt32 opcode,
                                 VstInt32 index,
                                 VstIntPtr value,
                                 void* ptr,
                                 float opt) final override {
        return AudioEffectX::dispatcher(opcode, index, value, ptr, opt);
    }

    virtual void setParameter(VstInt32 index,
                              float value) final override {
        assert(uint32_t(index)<vst_param_.count_);
        vst_param_.value_[index] = value;
    }

    virtual float getParameter(VstInt32 index) final override {
        assert(uint32_t(index)<vst_param_.count_);
        return vst_param_.value_[index];
    }

    virtual void getParameterLabel(VstInt32 index,
                                   char* label) final override {
        assert(uint32_t(index)<vst_param_.count_);
        *label = 0;
    }

    virtual void getParameterDisplay(VstInt32 index,
                                     char* text) final override {
        assert(uint32_t(index)<vst_param_.count_);
        *text = 0;
    }

    virtual void getParameterName(VstInt32 index,
                                  char* text) final override {
        assert(uint32_t(index)<vst_param_.count_);
        vst_strncpy(text, vst_param_.name_[index], kVstMaxParamStrLen);
    }

    virtual void getProgramName(char* name) final override {
        vst_strncpy(name, "test", kVstMaxProgNameLen);
    }

    virtual VstInt32 getNumMidiInputChannels() final override {
        return vst_config_.midi_inputs_;
    }

    virtual VstInt32 getNumMidiOutputChannels() final override {
        return vst_config_.midi_outputs_;
    }

    void sendMidiEvent(uint8_t * msg) {
        if (!getNumMidiInputChannels()) {
            return;
        }
        VstEvents event;
        assert(!"todo");
        sendVstEventsToHost(&event);
    }

    virtual VstInt32 canDo(char* text) final override {
        if (!strcmp(text, "receiveVstEvents"))
            return 1;
        if (!strcmp(text, "receiveVstMidiEvent"))
            return 1;
        return -1;
    }

    virtual void setSampleRate(float value) final override {
        sample_rate_ = value;
    }

    virtual VstInt32 processEvents(VstEvents* ev) final override {
        for (VstInt32 i = 0; i < ev->numEvents; i++) {
            if ((ev->events[i])->type==kVstMidiType) {
                const VstMidiEvent * event = (VstMidiEvent*)ev->events[i];
                assert(event);
                onMidiEvent(event->midiData);
            }
        }
        return 1;
    }

    void onMidiEvent(const char * data) {
        const uint8_t  channel = data[0]&0x0f;
        const VstInt32 message = data[0]&0xf0;

        switch (message) {
        case(c_note_on) :
            event_note_on(channel,
                          data[1] /*note*/,
                          data[2] /*vel*/);
            break;
        case(c_note_off) :
            event_note_off(channel,
                           data[1] /*note*/,
                           data[2] /*vel*/);
            break;
        case(c_pitch_bend) : {
            const int32_t pitch = int32_t((data[2]<<7)|data[1])-0x2000;
            event_pitch_bend(channel, pitch);
            }
            break;
        case (c_control_change) :
            event_control_change(channel,
                                 data[1] /*control*/,
                                 data[2] /*value*/);
            break;
        }
    }
};
