#include <stddef.h>
#include <sys/reent.h>

__attribute__((weak)) int _close_r(struct _reent *ptr, int fd) { return 0; }
__attribute__((weak)) int _lseek_r(int fd, int offset, int whence) { return 0; }
__attribute__((weak)) int _read_r(struct _reent *r, int fd, void *buf, size_t nbytes) { return 0; }
__attribute__((weak)) int _write_r(void *reent, int fd, const char *ptr, int len) { return 0; }
