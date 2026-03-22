#include "ramfs.h"
#include "string.h"
#include "io.h"

static ramfs_file_t files[RAMFS_MAX_FILES];
static int file_count = 0;

// Built-in test programs (raw x86-64 machine code for ring 3)
// Simple program: write "hello from userspace!\n" then exit
// Uses int 0x80 syscall convention:
//   rax=0 (write), rdi=1 (stdout), rsi=msg_addr, rdx=len
//   rax=1 (exit), rdi=0 (code)
static const uint8_t hello_program[] = {
    // mov rax, 0 (sys_write)
    0x48, 0xC7, 0xC0, 0x00, 0x00, 0x00, 0x00,
    // mov rdi, 1 (stdout)
    0x48, 0xC7, 0xC7, 0x01, 0x00, 0x00, 0x00,
    // lea rsi, [rip+17] (message)
    0x48, 0x8D, 0x35, 0x11, 0x00, 0x00, 0x00,
    // mov rdx, 21 (length)
    0x48, 0xC7, 0xC2, 0x15, 0x00, 0x00, 0x00,
    // int 0x80
    0xCD, 0x80,
    // mov rax, 1 (sys_exit)
    0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00,
    // mov rdi, 0 (exit code)
    0x48, 0xC7, 0xC7, 0x00, 0x00, 0x00, 0x00,
    // int 0x80
    0xCD, 0x80,
    // message: "hello from userspace!\n"
    'h', 'e', 'l', 'l', 'o', ' ', 'f', 'r', 'o', 'm',
    ' ', 'u', 's', 'e', 'r', 's', 'p', 'a', 'c', 'e', '!',
};

static const char readme_text[] =
    "=== Kernel README ===\n"
    "A minimal x86-64 kernel with:\n"
    "- Physical memory manager (bitmap)\n"
    "- Virtual memory (4-level paging)\n"
    "- Process management & scheduling\n"
    "- User-mode execution (ring 3)\n"
    "- Syscall interface (int 0x80)\n"
    "- RAM filesystem\n"
    "\nShell commands: ls, cat, exec, meminfo, ps, help\n";

static const char motd_text[] =
    "\n"
    "  _                    _  \n"
    " | |_  ___ _ _ _ _  __| | \n"
    " | / -_) '_| ' \\ / -_) | \n"
    " |_\\___|_| |_||_|\\___|\\_| \n"
    "\n"
    " x86-64  |  Limine  |  ring-3  |  no libc\n"
    " -----------------------------------------\n"
    "\n";

void ramfs_init(void) {
    memset(files, 0, sizeof(files));
    file_count = 0;

    ramfs_create("readme.txt", (const uint8_t *)readme_text, sizeof(readme_text) - 1, 0);
    ramfs_create("motd.txt", (const uint8_t *)motd_text, sizeof(motd_text) - 1, 0);
    ramfs_create("hello.bin", hello_program, sizeof(hello_program), 1);
}

int ramfs_create(const char *name, const uint8_t *data, size_t size, int executable) {
    if (file_count >= RAMFS_MAX_FILES)
        return -1;

    ramfs_file_t *f = &files[file_count];
    strncpy(f->name, name, RAMFS_MAX_FILENAME - 1);
    f->name[RAMFS_MAX_FILENAME - 1] = '\0';
    f->data = (uint8_t *)data;
    f->size = size;
    f->used = 1;
    f->executable = executable;
    file_count++;
    return 0;
}

ramfs_file_t *ramfs_open(const char *name) {
    for (int i = 0; i < file_count; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            return &files[i];
        }
    }
    return NULL;
}

int ramfs_read(ramfs_file_t *file, uint8_t *buf, size_t offset, size_t count) {
    if (!file || !file->used)
        return -1;
    if (offset >= file->size)
        return 0;
    if (offset + count > file->size)
        count = file->size - offset;
    memcpy(buf, file->data + offset, count);
    return (int)count;
}

int ramfs_list(char *buf, size_t bufsize) {
    int pos = 0;
    for (int i = 0; i < file_count && (size_t)pos < bufsize - 1; i++) {
        if (!files[i].used)
            continue;
        // Format: "name  size  [exec]\n"
        for (int j = 0; files[i].name[j] && (size_t)pos < bufsize - 2; j++)
            buf[pos++] = files[i].name[j];

        // Add padding
        buf[pos++] = ' ';
        buf[pos++] = ' ';

        // Size (simple decimal)
        size_t sz = files[i].size;
        char num[16];
        int nlen = 0;
        if (sz == 0) {
            num[nlen++] = '0';
        } else {
            while (sz > 0 && nlen < 15) {
                num[nlen++] = '0' + (sz % 10);
                sz /= 10;
            }
        }
        for (int j = nlen - 1; j >= 0 && (size_t)pos < bufsize - 2; j--)
            buf[pos++] = num[j];

        buf[pos++] = 'B';

        if (files[i].executable) {
            const char *tag = " [exec]";
            for (int j = 0; tag[j] && (size_t)pos < bufsize - 2; j++)
                buf[pos++] = tag[j];
        }

        buf[pos++] = '\n';
    }
    buf[pos] = '\0';
    return pos;
}

int ramfs_file_count(void) {
    return file_count;
}
