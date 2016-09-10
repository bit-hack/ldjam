#include "../framework_core/file.h"
#include "bitmap.h"


#if !defined(_MSC_VER)
#define PACK__ __attribute__((__packed__))
#else
#define PACK__
#pragma pack(push, 1)
#endif

bool bitmap_t::create(const uint32_t width,
                      const uint32_t height,
                      bitmap_t & out) {
    // allocate space for all the pixels
//    std::unique_ptr<uint32_t> bmp(new uint32_t[width * height]);
    out.pix_.resize(width * height * sizeof(uint32_t));
//    out.pix_.reset(bmp.release());
    out.width_ = width;
    out.height_ = height;
    return true;
}

bool bitmap_t::load(const char * path, bitmap_t & out) {
    // open bitmap file
    file_reader_t file;
    if (!file.open(path)) {
        return false;
    }
    // bitmap header structure
    struct PACK__ {
        uint16_t magic_;
        uint32_t bmp_size_;
        uint16_t reserved_1_;
        uint16_t reserved_2_;
        uint32_t pix_offset_;
    } header;
    // read the bitmap file header
    if (!file.read(header)) {
        return false;
    }
    // check for bitmap magic number
    if (header.magic_!=0x4d42 /*'BM'*/) {
        return false;
    }
    // read dib v1 section
    struct PACK__ {
        uint32_t size_; // <-- can tell us the header type
        uint32_t width_;
        uint32_t height_;
        uint16_t planes_;
        uint16_t bpp_;
    } dib_v1_;
    if (!file.read(dib_v1_)) {
        return false;
    }
    // check bpp is multiple of 8
    if (dib_v1_.bpp_&0x7) {
        return false;
    }
    const uint32_t image_size = (dib_v1_.width_ * dib_v1_.height_ * dib_v1_.bpp_) / 8;
    const uint32_t free_space = header.bmp_size_-header.pix_offset_;
    // check reported size can fit in our file
    if (image_size > free_space) {
        return false;
    }
    // allocate space for all the pixels
//    std::unique_ptr<uint32_t> bmp(new uint32_t[dib_v1_.width_ * dib_v1_.height_]);
    out.pix_.resize(dib_v1_.width_ * dib_v1_.height_ * sizeof(uint32_t));
    // seek to the start of pixel data
    if (!file.seek(header.pix_offset_)) {
        return false;
    }
    // parse based on pixel type
    switch (dib_v1_.bpp_) {
    case (24):
        // traverse with vflip
        for (int32_t y = int32_t(dib_v1_.height_)-1; y>=0; --y) {
            uint32_t * dst_pix = out.pix_.get<uint32_t>()+(y * dib_v1_.width_);
            // read row
            for (uint32_t x = 0; x<dib_v1_.width_; ++x) {
                // read row pixel
                uint8_t src[3];
                if (!file.read(src)) {
                    return false;
                }
                // repack as 32bit pixel
                dst_pix[x] = (src[2]<<16)|(src[1]<<8)|(src[0]<<0);
            }
            // jump since row size is rounted to multiple of 4
            if (!file.jump((3*dib_v1_.width_)&0x3)) {
                return false;
            }
        }
        break;
    case (32):
        // traverse with vflip
        for (int32_t y = int32_t(dib_v1_.height_)-1; y>=0; ++y) {
            uint32_t * dst_pix = out.pix_.get<uint32_t>()+(y * dib_v1_.width_);
            if (!file.read(dst_pix, sizeof(uint32_t) * dib_v1_.width_)) {
                return false;
            }
        }
        break;
    default:
        // unsupported bitdepth
        return false;
    }
    // write out to bitmap
    out.width_ = dib_v1_.width_;
    out.height_ = dib_v1_.height_;
//    out.pix_.reset(bmp.release());
    // success
    return true;
}

#if _MSC_VER
#pragma pack(pop)
#endif
