#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>

#include "common.h"
#include "file.h"

namespace tengu {
struct buffer_t {

    buffer_t()
        : data_()
        , size_(0)
    {}

    buffer_t(size_t size)
        : size_(size)
        , data_(new uint8_t[size])
    {}

    buffer_t(const buffer_t & buffer)
        : size_(buffer.size_)
        , data_(new uint8_t[size_])
    {
        memcpy(data_.get(), buffer.data_.get(), size_);
    }

    buffer_t(buffer_t && buffer)
        : size_(buffer.size_)
        , data_(std::move(buffer.data()))
    {}

#if 0
    explicit buffer_t(const char * src)
        : size_(strlen(src))
        , data_(new uint8_t[size_])
    {
        memcpy(data_.get(), src, size_);
    }
#endif

    template <typename type_t, size_t len>
    explicit buffer_t(const std::array<type_t, len> & in)
        : size_(len * sizeof(type_t))
        , data_(new uint8_t[size_])
    {
        memcpy(data_.get(), in.data(), size_);
    };

    template <typename type_t, size_t len>
    explicit buffer_t(const type_t (&array_in)[len])
        : size_(len * sizeof(type_t))
        , data_(new uint8_t[size_])
    {
        memcpy(data_.get(), array_in, size_);
    };

    explicit buffer_t(const void * src, size_t len)
        : size_(len)
        , data_(new uint8_t[size_])
    {
        memcpy(data_.get(), src, size_);
    }

    void resize(size_t size) {
        assert(size > 0);
        std::unique_ptr<uint8_t[]> mem(new uint8_t[size]);
        size_t copy_size = minv(size, size_);
        if (copy_size) {
            memcpy(mem.get(), data_.get(), copy_size);
        }
        size_ = size;
        data_.reset(mem.release());
    }

    void free() {
        data_.reset();
        size_ = 0;
    }

    bool load(const char * path) {
        file_reader_t file;
        if (!file.open(path)) {
            return false;
        }
        size_t size = 0;
        if (!file.size(size)) {
            return false;
        }
        if (size!=size_) {
            resize(size);
        }
        assert(size==size_);
        return file.read(data_.get(), size);
    }

    bool save(const char * path) const {
        if (!data_) {
            return false;
        }
        file_writer_t file;
        if (!file.open(path)) {
            return false;
        }
        return file.write(data_.get(), size_);
    }

    size_t size() const {
        return size_;
    }

    uint8_t * data() {
        assert(data_);
        return data_.get();
    }

    const uint8_t * data() const {
        assert(data_);
        return data_.get();
    }

    void fill(const uint8_t value) {
        if (size_ && data_) {
            memset(data_.get(), value, size_);
        }
    }

    void copy_from(uint8_t * src, size_t dst, size_t size) {
        assert(dst+size<size_);
        memcpy(data_.get()+dst, src, size);
    }

    void copy_to(size_t src, uint8_t * dst, size_t size) {
        assert(src+size<size_);
        memcpy(dst, data_.get()+src, size);
    }

    template <typename type_t = uint8_t>
    type_t * get(size_t offset = 0) {
        assert(offset+sizeof(type_t)<size_);
        return reinterpret_cast<type_t*>(data_.get()+offset);
    }

    template <typename type_t = uint8_t>
    const type_t * get(size_t offset = 0) const {
        assert(offset+sizeof(type_t)<size_);
        return reinterpret_cast<type_t*>(data_.get()+offset);
    }

    ~buffer_t() = default;

protected:
    size_t size_;
    std::unique_ptr<uint8_t[]> data_;
};
} // namespace tengu
