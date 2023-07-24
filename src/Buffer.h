#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include "NonCopyable.h"

using namespace std;


class Buffer : NonCopyable
{
public:
    explicit Buffer(size_t init_size = k_init_size)
        : buffer_(k_cheap_prepend + init_size), read_index_(k_cheap_prepend), write_index_(k_cheap_prepend)
    {
    }

    size_t readable_bytes() const { return write_index_ - read_index_; }
    size_t wirteable_bytes() const { return buffer_.size() - write_index_; }
    size_t prependable_bytes() const { return read_index_; }
    const char *peek() const { return begin() + read_index_; }

    void retrieve(size_t len)
    {
        if (len < readable_bytes())
        {
            read_index_ += len;
        } else {
            retrieve_all();
        }
    }

    void retrieve_all()
    {
        read_index_ = write_index_ = k_cheap_prepend;
    }

    string retrieve_all_asString()
    {
        return retrieve_as_string(readable_bytes()); 
    }

    string retrieve_as_string(size_t len)
    {
        string result(peek(), len);
        retrieve(len); 
        return result;
    }

    void ensure_writeable_bytes(size_t len)
    {
        if (wirteable_bytes() < len)
        {
            makespace(len);
        }
    }

    char *begin_write()
    {
        return begin() + write_index_;
    }

    void append(const char *data, size_t len)
    {
        ensure_writeable_bytes(len);
        copy(data, data + len, begin_write());

        write_index_ += len;
    }

    ssize_t readfd(int fd, int *save_errno);
    ssize_t writefd(int fd, int *save_errno);

public:
    static const size_t k_cheap_prepend = 8;
    static const size_t k_init_size = 1024;

    char *begin()
    {
        return &*buffer_.begin();
    }

    const char *begin() const
    {
        return &*buffer_.begin();
    }

    void makespace(size_t len)
    {
        if (wirteable_bytes() + prependable_bytes() < len + k_cheap_prepend)
        {
            buffer_.resize(write_index_ + len);
        }
        else
        {
            size_t readable = readable_bytes();
            copy(begin() + read_index_, begin() + write_index_, begin() + k_cheap_prepend);

            read_index_ = k_cheap_prepend;
            write_index_ = read_index_ + readable;
        }
    }

private:
    vector<char> buffer_;
    size_t read_index_;
    size_t write_index_;
};
