#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <queue>

#include "random.h"
#include "rect.h"

namespace tengu {
namespace anim {
struct sequence_t {

    enum end_type_t {
        e_end_hold,
        e_end_loop,
        e_end_pop
    };

    sequence_t(const std::string& name, end_type_t end_type)
        : name_(name)
        , end_type_(end_type)
    {
    }

    sequence_t& op_interval(int32_t speed) {
        opcodes_.push_back(opcode_t{e_op_interval, speed, 0});
        return *this;
    }

    sequence_t& op_frame(int32_t frame) {
        opcodes_.push_back(opcode_t{e_op_frame, frame, 0});
        return *this;
    }

    sequence_t& op_delay(int32_t ms) {
        opcodes_.push_back(opcode_t{e_op_delay, ms, 0});
        return *this;
    }

    sequence_t& op_event(int32_t id) {
        opcodes_.push_back(opcode_t{e_op_event, id, 0});
        return *this;
    }

    sequence_t& op_jmp(int32_t opcode) {
        opcodes_.push_back(opcode_t{e_op_jmp, opcode, 0});
        return *this;
    }

    sequence_t& op_rand_frame(int32_t min, int32_t max) {
        opcodes_.push_back(opcode_t{e_op_rand_frame, min, max});
        return *this;
    }

    sequence_t& op_hotspot(int32_t x, int32_t y) {
        opcodes_.push_back(opcode_t{e_op_hotspot, x, y});
        return *this;
    }

    sequence_t& op_offset(int32_t x, int32_t y) {
        opcodes_.push_back(opcode_t{e_op_offset, x, y});
        return *this;
    }

    void clear() {
        opcodes_.clear();
    }

    int32_t size() const {
        return int32_t(opcodes_.size());
    }

    enum opcode_type_t {
        e_op_interval,
        e_op_frame,
        e_op_delay,
        e_op_event,
        e_op_jmp,
        e_op_rand_frame,
        e_op_hotspot,
        e_op_offset,
    };

    struct opcode_t {
        opcode_type_t type_;
        int32_t x_;
        int32_t y_;
    };

    const opcode_t& get_opcode(size_t index) const {
        assert(index<opcodes_.size());
        return opcodes_.at(index);
    }

    const std::string name_;
    const end_type_t end_type_;

protected:
    std::vector<opcode_t> opcodes_;
};

struct sheet_t {

    sheet_t(int32_t width, int32_t height)
        : width_(width)
        , height_(height)
    {
    }

    void add_frame(const recti_t& frame) {
        frame_.push_back(frame);
    }

    void add_grid(const int32_t cell_width,
                  const int32_t cell_height) {

        for (int32_t y = 0; y<height_; y += cell_height) {
            for (int32_t x = 0; x<width_; x += cell_width) {
                frame_.push_back(
                    recti_t(x, y, cell_width, cell_height, recti_t::e_relative));
            }
        }
    }

    void clear() {
        frame_.clear();
    }

    const recti_t& get_frame(size_t index) const {
        assert((index<frame_.size())&&"no frame at this index");
        return frame_.at(index);
    }

protected:
    const int32_t width_;
    const int32_t height_;
    std::vector<recti_t> frame_;
};

struct controller_t {

    controller_t()
        : sheet_(nullptr)
        , prng_(0xcafebabe)
    {
    }

    void push_sequence(const sequence_t* sequence) {
        state_.push_back(state_t(sequence));
    }

    void set_sequence(const sequence_t* sequence) {
        pop_sequence();
        push_sequence(sequence);
    }

    void pop_sequence() {
        if (state_.size()) {
            state_.pop_back();
        }
    }

    bool tick(int32_t delta) {
        assert(sheet_);
        while (state_.size()) {

            state_t& state = state_.back();
            assert(state.sequence_);
            const sequence_t& seq = *state.sequence_;

            // remove any spent time
            state.delay_ -= delta;
            delta = 0;

            // if we are to soon for our next frame
            if (state.delay_>=0) {
                break;
            }

            // if we have reached the end of a sequence
            if (state.pc_>=seq.size()) {
                switch (seq.end_type_) {
                case (sequence_t::e_end_hold):
                    return true;
                case (sequence_t::e_end_loop):
                    state.pc_ = 0;
                    break;
                case (sequence_t::e_end_pop):
                    state_.pop_back();
                    continue;
                }
            }

            // dispatch based on opcode
            const sequence_t::opcode_t& op = seq.get_opcode(state.pc_);
            const auto old_pc = state.pc_;
            switch (op.type_) {
            case (sequence_t::e_op_interval):
                state.interval_ = op.x_;
                break;
            case (sequence_t::e_op_frame):
                state.rect_ = sheet_->get_frame(op.x_);
                state.delay_ += state.interval_;
                break;
            case (sequence_t::e_op_delay):
                state.delay_ += op.x_;
                break;
            case (sequence_t::e_op_event):
                event_.push(op.x_);
                break;
            case (sequence_t::e_op_jmp):
                assert(op.x_>=0&&op.x_<seq.size());
                state.pc_ = op.x_;
                break;
            case (sequence_t::e_op_rand_frame):
            {
                const int32_t frame = prng_.rand_range(op.x_, op.y_);
                state.rect_ = sheet_->get_frame(frame);
                state.delay_ += state.interval_;
            }
            break;
            case (sequence_t::e_op_hotspot):
                state.hotspot_x_ = op.x_;
                state.hotspot_y_ = op.y_;
                break;
            case (sequence_t::e_op_offset):
                state.offset_x_ = op.x_;
                state.offset_y_ = op.y_;
                break;
            default:
                assert(!"unknown opcode");
            }

            // if the pc has not changed do normal increment
            if (old_pc==state.pc_) {
                ++state.pc_;
            }
        }

        return true;
    }

    bool retrigger() {
        if (state_.size()==0) {
            return false;
        }
        state_t& state = state_.back();
        state.pc_ = 0;
        return true;
    }

    bool get_sequence(const sequence_t*& out) const {
        if (state_.size()) {
            out = state_.back().sequence_;
            return true;
        }
        return false;
    }

    bool get_frame(recti_t& out) const {
        if (state_.size()==0) {
            return false;
        }
        const state_t& state = state_.back();
        out = state.rect_;
        return true;
    }

    bool get_event(uint32_t& out) {
        if (event_.size()) {
            out = event_.front();
            event_.pop();
            return true;
        }
        return false;
    }

    bool get_hotspot(int32_t& x_out, int32_t& y_out) {
        if (state_.size()) {
            x_out = state_.back().hotspot_x_;
            y_out = state_.back().hotspot_y_;
            return true;
        }
        return false;
    }

    bool get_offset(int32_t& x_out, int32_t& y_out) {
        if (state_.size()) {
            x_out = state_.back().offset_x_;
            y_out = state_.back().offset_y_;
            return true;
        }
        return false;
    }

    void set_sheet(const sheet_t* sheet) {
        sheet_ = sheet;
    }

    bool is_playing(const sequence_t* seq) const {
        if (state_.size())
            return (state_.back().sequence_==seq);
        else
            return false;
    }

protected:
    struct state_t {

        state_t(const sequence_t* seq)
            : sequence_(seq)
            , rect_{0, 0, 0, 0}
            , interval_(1)
            , pc_(0)
            , delay_(0)
            , offset_x_(0)
            , offset_y_(0)
            , hotspot_x_(0)
            , hotspot_y_(0)
        {
        }

        const sequence_t* sequence_;
        recti_t rect_;
        int32_t interval_;
        int32_t pc_;
        int32_t delay_;
        int32_t offset_x_;
        int32_t offset_y_;
        int32_t hotspot_x_;
        int32_t hotspot_y_;
    };

    random_t prng_;
    const sheet_t* sheet_;
    std::vector<state_t> state_;
    std::queue<uint32_t> event_;
};

} // namespace anim
} // namespace tengu