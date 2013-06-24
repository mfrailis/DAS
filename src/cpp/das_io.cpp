#include "internal/das_io.hpp"
#include <cstdio>
#include <cerrno>


void
das_io_read(const char *path, size_t offset, size_t count, void* buffer){
    errno = 0;
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        if (errno)
            throw das::das_io_exception(strerror(errno));
        else
            throw das::das_io_exception("error occured while opening file");
    }

    if (fseek(f, offset, SEEK_SET)) {
        fclose(f);
        throw das::das_io_exception("error occured while seeking the file");
    }

    size_t result = fread(buffer, 1, count, f);
    fclose(f);
    if (result != count) {
        if (feof(f))
            throw das::das_io_exception("end of file reached while reading the file");
        else
            throw das::das_io_exception("error occured while reading the file");
    }
}
