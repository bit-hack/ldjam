#pragma once
#include <memory>
#include "../../framework_core/file.h"

struct vorbis_t {

    vorbis_t() 
        : data_()
        , size_(0)
    {
    }

    bool open(const char * path) {
        file_reader_t file;
        if (!file.open(path)) {
            return false;
        }
        if (!file.size(size_)) {
            return false;
        }
        data_.reset(new uint8_t[size_]);
        return file.read(data_.get(), size_);
    }

    void close() {
        data_.reset();
        size_ = 0;
    }

    const uint8_t * data() const {
        return data_.get();
    }

    size_t size() const {
        return size_;
    }

    bool valid() const {
        return data_.get()!=nullptr && size_;
    }

protected:
    std::unique_ptr<uint8_t[]> data_;
    size_t size_;
};
