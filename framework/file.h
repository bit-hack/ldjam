#pragma once

#include <array>
#include <string>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>

struct file_reader_t {

    // constructor
    file_reader_t()
        : file_(nullptr)
        , pos_()
    {
    }

    // constructor with open
    explicit file_reader_t(const std::string & path)
        : file_(nullptr)
        , pos_()
    {
        open(path.c_str());
    }

    // constructor with open
    explicit file_reader_t(const char * path)
        : file_(nullptr)
        , pos_()
    {
        open(path);
    }

    // copy constructor
    file_reader_t(const file_reader_t & other)
    {
        // copy position stack
        if (other.pos_.size()) {
            pos_ = other.pos_;
        }
        // duplicate file handle
        assert(!"todo: duplicate file handle");
    };

    // move constructor
    file_reader_t(file_reader_t && other)
        : file_(other.file_)
        , pos_(std::move(other.pos_))
    {
        other.file_ = nullptr;
    }

    // assignment operator
    void operator = (const file_reader_t & rhs)
    {
        assert(!"todo, same as copy constructor");
    }

    // destructor
    ~file_reader_t()
    {
        close();
    }

    // open file from specific path
    bool open(const char * & path) {
        if (file_) {
            close();
        }
#if defined(_MSC_VER)
        fopen_s(&file_, path, "rb");
#else
        file_ = fopen(path, "rb");
#endif
        return file_ != nullptr;
    }

    //
    bool open(const std::string & path) {
        return open(path.c_str());
    }

    bool close() {
        if (file_) {
            fclose(file_);
            file_ = nullptr;
            return true;
        }
        return false;
    }

    // read type from file
    template <typename type_t>
    bool read(type_t & out) {
        return read(&out, sizeof(type_t));
    }

    // read c style array from file
    template <typename type_t>
    bool read(type_t * out, const size_t count) {
        for (size_t i=0; i<count; ++i) {
            if (!read<type_t>(out[i])) {
                return false;
            }
        }
        return true;
    }

    // read c++11 array from file
    template <typename type_t, size_t c_size>
    bool read(std::array<type_t, c_size> & out) {
        for (type_t & item : out) {
            if (!read<type_t>(item)) {
                return false;
            }
        }
        return true;
    }

    // read c string (null terminated)
    bool read_cstr(std::string & out) {
        out.clear();
        while (true) {
            char ch = 0;
            if (!read<char>(ch)) {
                return false;
            }
            if (ch == 0) {
                break;
            }
            out += ch;
        }
        return true;
    }

    // read pascal string (size type specific)
    template <typename type_t>
    bool read_pstr(std::string & out) {
        out.clear();
        type_t count = 0;
        if (!read<type_t>(count)) {
            return false;
        }
        for (type_t i = 0; i<count; ++i) {
            char ch = 0;
            if (!read<char>(ch)) {
                return false;
            }
            out += ch;
        }
        return true;
    }

    // read data into memory
    bool read(void * out, size_t size) {
        if (file_) {
            return fread(out, size, 1, file_) == 1;
        }
        return false;
    }

    // seek to specific file position
    bool seek(size_t pos) {
        if (file_) {
            fseek(file_, pos, SEEK_SET);
            return true;
        }
        return false;
    }

    // get current file read position
    bool pos_get(size_t & pos) const {
        if (file_) {
            pos = ftell(file_);
            return true;
        }
        return false;
    }

    // push current file read position onto internal stack
    bool pos_push() {
        size_t pos = 0;
        if (pos_get(pos)) {
            pos_.push_back(pos);
            return true;
        }
        return false;
    }

    // pop current file read position from internal stack
    bool pos_pop() {
        assert(pos_.size() && "array empty");
        size_t pos = pos_.back();
        pos_.pop_back();
        return seek(pos);
    }

    // clear internal file position stack
    bool pos_clear() {
        pos_.clear();
        return true;
    }

    // check if file is currently open
    bool is_open() const {
        return file_ != nullptr;
    }

protected:
    std::vector<size_t> pos_;
    FILE * file_;
};

struct file_writer_t {

    file_writer_t()
        : file_(nullptr)
    {
    }

protected:
    FILE * file_;
};