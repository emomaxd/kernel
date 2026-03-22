#ifndef RAMFS_H
#define RAMFS_H

#include <stdint.h>
#include <stddef.h>

#define RAMFS_MAX_FILES    32
#define RAMFS_MAX_FILENAME 64

typedef struct {
    char name[RAMFS_MAX_FILENAME];
    uint8_t *data;
    size_t size;
    int used;
    int executable;  // can be exec'd as a user program
} ramfs_file_t;

void ramfs_init(void);
int ramfs_create(const char *name, const uint8_t *data, size_t size, int executable);
ramfs_file_t *ramfs_open(const char *name);
int ramfs_read(ramfs_file_t *file, uint8_t *buf, size_t offset, size_t count);
int ramfs_list(char *buf, size_t bufsize);
int ramfs_file_count(void);

#endif // RAMFS_H
