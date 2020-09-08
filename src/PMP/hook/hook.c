#define _GNU_SOURCE

#include <dlfcn.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define ALLOCATE_SIZE 0x400000
#define TARGET_LONG_SIZE 8
#define __NR_printmsg 328

static int (*real_open)(const char*, int, ...);
static FILE* (*real_fopen)(const char*, const char*);

static void* (*real_malloc)(size_t);
static void (*real_free)(void*);

static inline unsigned long fill_value(void) { return 0x0; }

int open(const char *filename, int flags, ...) {
  if (!real_open) real_open = dlsym(RTLD_NEXT, "open");

  int fd = real_open(filename, flags);

  if (fd == -1) {
    char redirect[128];
    if (flags == O_RDONLY) sprintf(redirect, "%s/input_scheme", getenv("WORKDIR"));
    if (flags == O_WRONLY || flags == O_RDWR) sprintf(redirect, "/dev/null");
    fd = real_open(redirect, flags);
  }

  return fd;
}

FILE* fopen(const char *filename, const char *mode) {
  if (!real_fopen) real_fopen = dlsym(RTLD_NEXT, "fopen");

  FILE *file = real_fopen(filename, mode);

  if (!file) {
    char redirect[128];
    if (strchr(mode, 'r') != NULL) sprintf(redirect, "%s/input_scheme", getenv("WORKDIR"));
    if (strchr(mode, 'w') != NULL || strchr(mode, 'a') != NULL) sprintf(redirect, "/dev/null");
    file = real_fopen(redirect, mode);
  }

  return file;
}

void* malloc(size_t size) {
  if (!real_malloc) real_malloc = dlsym(RTLD_NEXT, "malloc");

  void *memory = real_malloc(size);

//  int start_time = clock();

  for (size_t i = 0; i < size / TARGET_LONG_SIZE; i++) {
    unsigned long val = fill_value();
    memcpy((void*)(intptr_t)(memory + i * TARGET_LONG_SIZE), &val, TARGET_LONG_SIZE);
  }

//  int end_time = clock();
//  char msg[256];
//  sprintf(msg, "THI: %6f\n", ((float)(end_time-start_time))/1000);
//  syscall(__NR_printmsg, msg);

  return memory;
}

void free(void *ptr) {
  if (!real_free) real_free = dlsym(RTLD_NEXT, "free");
  if ((unsigned long)(ptr) >= ALLOCATE_SIZE) real_free(ptr);
}

void __hook_init(void) {
}

void __hook_fini(void) {
}
