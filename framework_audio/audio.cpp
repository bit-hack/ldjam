#include "../framework_core/common.h"
#include "audio.h"

namespace tengu {
bool audio_t::render(int16_t * out, uint32_t count) {
    while (count) {
        // figure our how many samples to render
        const uint32_t samples = minv(count, C_BUFFER_SIZE);
        count -= samples;
        // clear our mix buffers
        mix_.left_.fill(0);
        mix_.right_.fill(0);
        // collect data from all sources
        mix_out_t mix = {
            mix_.left_.data(),
            mix_.right_.data(),
            samples
        };
        for (const auto itt:source_) {
            itt->render(mix);
        }
        // mix down into output stream
        _mixdown(out, samples);
        out += samples*2;
    }
    return true;
}

void audio_t::_mixdown(int16_t * out, const uint32_t count) {
    for (uint32_t i = 0; i<count; ++i) {
        out[i*2+0] = clip(mix_.left_.data()[i]);
        out[i*2+1] = clip(mix_.right_.data()[i]);
    }
}
} // namespace tengu
