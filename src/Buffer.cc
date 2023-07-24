#include "Buffer.h"
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <error.h>

ssize_t Buffer::readfd(int fd, int *save_errno)
{
    char extra_buffer[65536] = {0}; 
    struct iovec vec[2];
    const size_t writeable = wirteable_bytes(); 

    vec[0].iov_base = begin() + write_index_;
    vec[0].iov_len = writeable;

    vec[1].iov_base = extra_buffer;
    vec[1].iov_len = sizeof(extra_buffer);

    const int iovcnt = writeable < sizeof(extra_buffer) ? 2 : 1;
    const ssize_t n = readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *save_errno = errno;
    }
    else if (n <= writeable)
    {
        write_index_ += n;
    }
    else 
    {
        write_index_ = buffer_.size();
        append(extra_buffer, n - writeable);
    }
    return n;
}

ssize_t Buffer::writefd(int fd, int *save_errno)
{
    ssize_t n = write(fd, peek(), readable_bytes());
    if (n < 0)
    {
        *save_errno = errno;
    }
    return n;
}
