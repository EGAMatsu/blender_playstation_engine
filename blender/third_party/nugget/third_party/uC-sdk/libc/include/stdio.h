#pragma once

#include <decl.h>
#include <reent.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <malloc.h>

static const int EOF = -1;

struct _FILE {
    int fd;
    int got_eof;
    int got_error;
};

BEGIN_DECL

typedef struct _FILE FILE;
extern FILE * stdin, * stdout, * stderr;

int vdprintf(int fd, const char * format, va_list ap);
int vsprintf(char * str, const char * format, va_list ap);
int vsnprintf(char * str, size_t size, const char * format, va_list ap);
int vasprintf(char ** strp, const char * format, va_list ap);
int vxprintf(void (*func)(const char *, int, void *), void * arg, const char * format, va_list ap);
static __inline__ int vfprintf(FILE * stream, const char * format, va_list ap) { return vdprintf(stream->fd, format, ap); }
static __inline__ int vprintf(const char * format, va_list ap) { return vfprintf(stdout, format, ap); }

static __inline__ int dprintf(int fd, const char * format, ...) { va_list ap; int r; va_start(ap, format); r = vdprintf(fd, format, ap); va_end(ap); return r; }
static __inline__ int sprintf(char * str, const char * format, ...) { va_list ap; int r; va_start(ap, format); r = vsprintf(str, format, ap); va_end(ap); return r; }
static __inline__ int snprintf(char * str, size_t size, const char * format, ...) { va_list ap; int r; va_start(ap, format); r = vsnprintf(str, size, format, ap); va_end(ap); return r; }
static __inline__ int asprintf(char ** strp, const char * format, ...) { va_list ap; int r; va_start(ap, format); r = vasprintf(strp, format, ap); va_end(ap); return r; }
static __inline__ int xprintf(void (*func)(const char *, int, void *), void * arg, const char * format, ...) { va_list ap; int r; va_start(ap, format); r = vxprintf(func, arg, format, ap); va_end(ap); return r; }
static __inline__ int fprintf(FILE * stream, const char * format, ...) { va_list ap; int r; va_start(ap, format); r = vfprintf(stream, format, ap); va_end(ap); return r; }
static __inline__ int printf(const char * format, ...) { va_list ap; int r; va_start(ap, format); r = vprintf(format, ap); va_end(ap); return r; }

int vdscanf(int fd, const char * format, va_list ap);
int vsscanf(const char * str, const char * format, va_list ap);
int vxscanf(int (*xgetc)(void *), void (*xungetc)(void *, int), void * opaque, const char * format, va_list args);
static __inline__ int vfscanf(FILE * stream, const char * format, va_list ap) { return vdscanf(stream->fd, format, ap); }
static __inline__ int vscanf(const char * format, va_list ap) { return vfscanf(stdin, format, ap); }

static __inline__ int dscanf(int fd, const char * format, ...) { va_list ap; int r; va_start(ap, format); r = vdscanf(fd, format, ap); va_end(ap); return r; }
static __inline__ int sscanf(const char * str, const char * format, ...)  { va_list ap; int r; va_start(ap, format); r = vsscanf(str, format, ap); va_end(ap); return r; }
static __inline__ int xscanf(int (*xgetc)(void *), void (*xungetc)(void *, int), void * opaque, const char *format, ...) { va_list ap; int r; va_start(ap, format); r = vxscanf(xgetc, xungetc, opaque, format, ap); va_end(ap); return r; }
static __inline__ int fscanf(FILE * stream, const char * format, ...) { va_list ap; int r; va_start(ap, format); r = vfscanf(stream, format, ap); va_end(ap); return r; }
static __inline__ int scanf(const char * format, ...) { va_list ap; int r; va_start(ap, format); r = vscanf(format, ap); va_end(ap); return r; }

void __sinit(struct _reent *);

END_DECL

// We don't even buffer, so...
static __inline__ int fflush(FILE *stream) { return 0; }

// hopefully, since all of the mode crap is static, gcc will optimize most of it away.
static __inline__ FILE * freopen(const char * fname, const char * mode, FILE * r) {
    int flags = 0, plus = 0, append = 0, fd;
    if (!mode || !mode[0]) {
        set_errno(EINVAL);
        return NULL;
    }

    if (mode[1] == 'b') {
        plus = mode[2] == '+';
    } else if (mode[1]) {
        if (mode[1] != '+') {
            set_errno(EINVAL);
            return NULL;
        }
        plus = 1;
    }

    switch (mode[0]) {
    case 'r':
        if (plus) {
            flags = O_RDWR;
        } else {
            flags = O_RDONLY;
        }
        break;
    case 'w':
        if (plus) {
            flags = O_RDWR | O_CREAT | O_TRUNC;
        } else {
            flags = O_WRONLY | O_CREAT | O_TRUNC;
        }
        break;
    case 'a':
        append = 1;
        if (plus) { // won't be properly supported
            flags = O_RDWR | O_CREAT;
        } else {
            flags = O_WRONLY | O_CREAT;
        }
        break;
    default:
        set_errno(EINVAL);
        return NULL;
    }

    fd = open(fname, flags);

    if (fd >= 0) {
        if (r) {
            close(r->fd);
        } else {
            r = (FILE *) malloc(sizeof(FILE));
        }
        r->fd = fd;
        r->got_eof = 0;
        r->got_error = 0;
    }

    if (append)
        lseek(r->fd, 0, SEEK_END);

    return r;
}

static __inline__ FILE * fopen(const char * fname, const char * mode) {
    return freopen(fname, mode, NULL);
}

static __inline__ FILE * fdopen(int fd, const char * mode) {
    FILE * r = (FILE *) malloc(sizeof(FILE));
    r->fd = fd;
    r->got_eof = 0;
    r->got_error = 0;

    return r;
}

static __inline__ int fclose(FILE * stream) {
    int fd;

    if (!stream) {
        set_errno(EINVAL);
        return -1;
    }

    fd = stream->fd;
    free(stream);
    return close(fd);
}

// Again, the compiler should do the right thing, and optimize depending on the values of size and nmemb.
// Luckily, we always will get into the short cases.
static __inline__ size_t fread(void * _ptr, ssize_t size, ssize_t nmemb, FILE * stream) {
    int i;
    uint8_t * ptr = (uint8_t *) _ptr;
    ssize_t r;

    if (!stream) {
        set_errno(EINVAL);
        return -1;
    }

    if (size == 1) {
        r = read(stream->fd, ptr, nmemb);
        if (r == 0) stream->got_eof = 1;
        if (r < 0) {
            stream->got_error = 1;
            return 0;
        }
        return r;
    }

    if (nmemb == 1) {
        r = read(stream->fd, ptr, size) == size ? 1 : 0;
        if (r == 0) stream->got_eof = 1;
        if (r < 0) {
            stream->got_error = 1;
            return 0;
        }
        return r;
    }

    for (i = 0; i < nmemb; i++) {
        r = read(stream->fd, ptr + size * i, size);
        if (r < 0) {
            stream->got_error = 1;
            return 0;
        }
        if (r != size) {
            stream->got_eof = 1;
            return i;
        }
    }

    return nmemb;
}

static __inline__ size_t fwrite(const void * _ptr, ssize_t size, ssize_t nmemb, FILE * stream) {
    int i;
    const uint8_t * ptr = (const uint8_t *) _ptr;
    ssize_t r;

    if (!stream) {
        set_errno(EINVAL);
        return -1;
    }

    if (size == 1) {
        r = write(stream->fd, ptr, nmemb);
        if (r < 0) {
            stream->got_error = 1;
            return 0;
        }
        return r;
    }

    if (nmemb == 1) {
        r = write(stream->fd, ptr, size);
        if (r < 0) {
            stream->got_error = 1;
            return 0;
        }
        return r == size ? 1 : 0;
    }

    for (i = 0; i < nmemb; i++) {
        r = write(stream->fd, ptr + size * i, size);
        if (r < 0) {
            stream->got_error = 1;
            return 0;
        }
        if (r != size) return i;
    }

    return nmemb;
}

static __inline__ int fgetc(FILE * stream) {
    uint8_t v;
    ssize_t r;

    if (!stream) {
        set_errno(EINVAL);
        return -1;
    }

    r = read(stream->fd, &v, 1);

    if (r != 1) {
        if (r == 0) stream->got_eof = 1;
        else stream->got_error = 1;
        return EOF;
    }

    return v;
}

static __inline__ int fseek(FILE * stream, off_t offset, int wheel) {
    int r;

    if (!stream) {
        set_errno(EINVAL);
        return -1;
    }

    r = lseek(stream->fd, offset, wheel) != -1 ? 0 : -1;
    if (!r) stream->got_eof = 0;
    if (errno != 0) stream->got_error = 1;
    return r;
}

static __inline__ char * fgets(char * s, int n, FILE * stream) {
    int r, fd;
    char c, * copy = s;

    if (!stream) {
        set_errno(EINVAL);
        return NULL;
    }

    fd = stream->fd;

    while (--n) {
        r = read(fd, &c, 1);
        switch (r) {
        case 0:
            stream->got_eof = 1;
            *s = 0;
            return copy;
        case 1:
            *s++ = c;
            if (c == '\n') {
                *s = 0;
                return copy;
            }
            break;
        case -1:
            stream->got_error = 1;
            return NULL;
            break;
        }
    };

    *s = 0;
    return copy;
}

static __inline__ int getc() { return fgetc(stdin); }
static __inline__ off_t ftell(FILE * stream) { return lseek(stream->fd, 0, SEEK_CUR); }
static __inline__ int feof(FILE * stream) { return stream->got_eof; }
static __inline__ int ferror(FILE * stream) { return stream->got_error; }
static __inline__ int fileno(FILE * stream) { return stream->fd; }
static __inline__ void rewind(FILE * stream) { fseek(stream, 0, SEEK_SET); }
