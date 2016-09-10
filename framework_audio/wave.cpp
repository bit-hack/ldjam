#include "../framework_core/file.h"
#include "wave.h"


#if !defined(_MSC_VER)
#define PACK__ __attribute__((__packed__))
#else
#define PACK__
#pragma pack(push, 1)
#endif

namespace {
    constexpr uint32_t fourcc(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
        return (d<<24)|(c<<16)|(b<<8)|a;
    }

    template <typename type_t>
    type_t endian(type_t in) {
        uint8_t * p = reinterpret_cast<uint8_t*>(&in);
        const size_t size = sizeof(type_t);
        for (int i = 0; i<size/2; ++i) {
            uint8_t & a = p[i];
            uint8_t & b = p[(size-1)-i];
            uint8_t temp = a;
            a = b;
            b = temp;
        }
        return in;
    }
} // namespace {}

bool wave_t::load_wav(const char * path, wave_t & out) {

    file_reader_t file;
    if (!file.open(path)) {
        return false;
    }

    struct PACK__ {
        uint32_t chunk_id_;
        uint32_t chunk_size_;
        uint32_t format_;
    } riff;

    struct PACK__ {
        uint32_t chunk_id_;
        uint32_t chunk_size_;
        uint16_t format_;
        uint16_t channels_;
        uint32_t sample_rate_;
        uint32_t byte_rate_;
        uint16_t block_align_;
        uint16_t bit_depth_;
    } fmt;

    struct PACK__ {
        uint32_t chunk_id_;
        uint32_t chunk_size_;
        /* after starts sample data */
    } data;

    // read riff header
    if (!file.read(riff)) {
        return false;
    }
    if (riff.chunk_id_!=fourcc('R', 'I', 'F', 'F')) {
        return false;
    }
    if (riff.format_!=fourcc('W', 'A', 'V', 'E')) {
        return false;
    }
    // read format chunk
    if (!file.read(fmt)) {
        return false;
    }
    if (fmt.chunk_id_!=fourcc('f', 'm', 't', ' ')) {
        return false;
    }
    // read data chunk
    if (!file.read(data)) {
        return false;
    }
    if (data.chunk_id_!=fourcc('d', 'a', 't', 'a')) {
        return false;
    }
    // sanity check format
    if (fmt.bit_depth_%8 /* multiple of 8 */) {
        return false;
    }
    if (fmt.channels_>2) {
        return false;
    }
    // sanity check on file size
    size_t file_size = 0;
    if (!file.size(file_size)) {
        return false;
    }
    if (data.chunk_size_>=file_size) {
        return false;
    }
    // read sample data
    out.samples_.resize(data.chunk_size_);
    if (!file.read(out.samples_.data(), data.chunk_size_)) {
        return false;
    }
    // copy into output structure
    out.bit_depth_ = fmt.bit_depth_;
    out.sample_rate_ = fmt.sample_rate_;
    out.channels_ = fmt.channels_;
    /* success */
    return true;
}

#if defined(_MSC_VER)
#pragma pack(pop)
#endif
