#pragma once

#include <decl.h>
#include <stdio.h>

#define MAX_FDS 32

BEGIN_DECL

typedef ssize_t (*fdread_t)(void * opaque, void * buf, size_t count);
typedef ssize_t (*fdwrite_t)(void * opaque, const void * buf, size_t count);
typedef off_t (*fdseek_t)(void * opaque, off_t offset, int whence);
typedef int (*fdclose_t)(void * opaque);

int fio_is_open(int fd);
int fio_open(fdread_t, fdwrite_t, fdseek_t, fdclose_t, void * opaque);
ssize_t fio_read(int fd, void * buf, size_t count);
ssize_t fio_write(int fd, const void * buf, size_t count);
off_t fio_seek(int fd, off_t offset, int whence);
int fio_close(int fd);
void fio_set_opaque(int fd, void * opaque);

void register_stdio_devices();
void register_custom_stdin(fdread_t custom_stdin_read, void * opaque);
void register_custom_stdout(fdwrite_t custom_stdout_write, void * opaque);

END_DECL
