#pragma once

#include <array>
#include <string>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <cstring>

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
        , path_()
    {
        open(path.c_str());
    }

    // constructor with open
    explicit file_reader_t(const char * path)
        : file_(nullptr)
        , pos_()
        , path_()
    {
        open(path);
    }

    // copy constructor
    file_reader_t(const file_reader_t & other)
    {
        if (!copy(other)) {
            close();
        }
    }

    // move constructor
    file_reader_t(file_reader_t && other)
        : file_(other.file_)
        , pos_(std::move(other.pos_))
        , path_(std::move(other.path_))
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
    bool open(const char * path)
    {
        if (file_) {
            close();
        }
#if defined(_MSC_VER)
        if (fopen_s(&file_, path, "rb")) {
            file_ = nullptr;
            return false;
        }
#else
        file_ = fopen(path, "rb");
#endif
        if (file_) {
            path_ = path;
        }
        return file_ != nullptr;
    }

    // close a file
    bool close()
    {
        path_.clear();
        if (file_) {
            fclose(file_);
            file_ = nullptr;
            return true;
        }
        return true;
    }

    // read type from file
    template <typename type_t>
    bool read(type_t & out)
    {
        return read(&out, sizeof(type_t));
    }

    // read c style array from file
    template <typename type_t, size_t c_size>
    bool read(type_t (&out)[c_size])
    {
        for (size_t i=0; i<c_size; ++i) {
            if (!read<type_t>(out[i])) {
                return false;
            }
        }
        return true;
    }

    // read c++11 array from file
    template <typename type_t, size_t c_size>
    bool read(std::array<type_t, c_size> & out)
    {
        for (type_t & item : out) {
            if (!read<type_t>(item)) {
                return false;
            }
        }
        return true;
    }

    // read c string (null terminated)
    bool read_cstr(std::string & out)
    {
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
    bool read_pstr(std::string & out)
    {
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
    bool read(void * out, size_t size)
    {
        if (file_) {
            return fread(out, size, 1, file_) == 1;
        }
        return false;
    }

    // relative seek
    bool jump(int32_t relative) {
        if (!file_) {
            return false;
        }
        if (relative != 0) {
            if (fseek(file_, relative, SEEK_CUR)) {
                return false;
            }
        }
        return true;
    }

    // seek to specific file position
    bool seek(size_t pos)
    {
        if (file_) {
            if (fseek(file_, long(pos), SEEK_SET)) {
                return false;
            }
            return true;
        }
        return false;
    }

    // get current file read position
    bool get_pos(size_t & pos) const
    {
        if (file_) {
            pos = ftell(file_);
            return true;
        }
        return false;
    }

    // push current file read position onto internal stack
    bool push_pos()
    {
        size_t pos = 0;
        if (get_pos(pos)) {
            pos_.push_back(pos);
            return true;
        }
        return false;
    }

    // pop current file read position from internal stack
    bool pop_pos()
    {
        assert(pos_.size() && "array empty");
        size_t pos = pos_.back();
        pos_.pop_back();
        return seek(pos);
    }

    // clear internal file position stack
    bool clear_pos()
    {
        pos_.clear();
        return true;
    }

    // check if file is currently open
    bool is_open() const
    {
        return file_ != nullptr;
    }

    // return the source file path
    bool get_path(std::string & out) const
    {
        if (file_) {
            out = path_;
            return true;
        }
        return false;
    }

    // return the total file size in bytes
    bool size(size_t & out) const
    {
        auto pos = ftell(file_);
        if (fseek(file_, 0, SEEK_END))
            return false;
        auto end = ftell(file_);
        if (fseek(file_, pos, SEEK_SET))
            return false;
        out = size_t(end);
        return true;
    }

protected:
    // copy another file_reader_t
    bool copy(const file_reader_t & other)
    {
        // close any already open file
        close();
        // copy position stack
        if (other.pos_.size()) {
            pos_ = other.pos_;
        }
        if (!other.is_open()) {
            // easy copy since not open
            return true;
        }
        if (!open(other.path_.c_str())) {
            return false;
        }
        size_t pos = 0;
        if (!other.get_pos(pos)) {
            return false;
        }
        if (!seek(pos)) {
            return false;
        }
        return true;
    }

    std::string path_;
    std::vector<size_t> pos_;
    FILE * file_;
};

struct file_writer_t {

    file_writer_t()
        : file_(nullptr)
        , path_()
    {
    }

    // constructor with open
    file_writer_t(const char * path) {
        open(path);
    }

    // copy constructor
    file_writer_t(const file_writer_t &) = delete;

    // move constructor
    file_writer_t(file_writer_t && other)
        : file_(other.file_)
        , path_(std::move(other.path_))
    {
        other.file_ = nullptr;
    }

    // assignment operator
    void operator = (const file_writer_t &) = delete;

    // destructor
    ~file_writer_t() {
        close();
    }

    bool open(const char * path) {
        if (file_) {
            close();
        }
#if defined(_MSC_VER)
        if (fopen_s(&file_, path, "wb")) {
            file_ = nullptr;
            return false;
        }
#else
        file_ = fopen(path, "wb");
#endif
        if (file_) {
            path_ = path;
            return true;
        }
        return false;
    }

    // close opened file handle
    bool close() {
        if (file_) {
            path_.clear();
            fclose(file_);
            file_=nullptr;
            return true;
        }
        return false;
    }

    // write c-string type
    bool write(const char * & str) {
        size_t str_size = strlen(str);
        return write((void*)str, str_size+1);
    }

    // write p-string type
    template<typename type_t>
    bool write_pstr(const char * str) {
        type_t str_size = type_t(strlen(str));
        write<type_t>(str_size);
        return write((void*)str, str_size);
    }

    // write p-string type
    template<typename type_t>
    bool write_pstr(const std::string & str) {
        type_t str_size = type_t(str.size());
        write<type_t>(str_size);
        return write((void*)str.c_str(), str_size);
    }

    // write specific type
    template <typename type_t>
    bool write(const type_t & in) {
        return write(&in, sizeof(type_t));
    }

    // write c++ string as cstring
    bool write(const std::string & str) {
        const size_t str_size = str.size();
        return write((void*)str.c_str(), str_size+1);
    }

    // write anonymous memory allocation
    bool write(const void * src, const size_t size) {
        if (file_) {
            return fwrite(src, size, 1, file_)==1;
        }
        return false;
    }

    // write c++11 style array
    template <typename type_t, size_t c_size>
    bool write(const std::array<type_t, c_size> & array) {
        for (auto & item:array) {
            if (!write<type_t>(item)) {
                return false;
            }
        }
        return true;
    }

    // write c style array
    template <typename type_t, size_t c_size>
    bool write(const type_t(&in)[c_size]) {
        for (size_t i = 0; i<c_size; ++i) {
            if (!write<type_t>(in[i])) {
                return false;
            }
        }
        return true;
    }

    // repeat output a set number of times
    template <typename type_t>
    bool fill(const type_t & item, size_t count) {
        for (size_t i = 0; i<count; ++i) {
            if (!write(item)) {
                return false;
            }
        }
        return true;
    }

    // check if the file is open
    bool is_open() const {
        return file_!=nullptr;
    }

    // return the current file size
    bool size(size_t & out) const {
        if (file_) {
            out = ftell(file_);
            return true;
        }
        return false;
    }

    // return the source file path
    bool get_path(std::string & out) const
    {
        if (file_) {
            out = path_;
            return true;
        }
        return false;
    }

protected:
    std::string path_;
    FILE * file_;
};
