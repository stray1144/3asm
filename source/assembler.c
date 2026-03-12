// Copyright (C) 2026 Stray1144
// SPDX-License-Identifier: GPL-3.0-or-later

#include "3asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

size_t file_size_get(char *path) {
        struct stat data;
        stat(path, &data);

        return data.st_size;
}

char *file_read(char *path) {
        size_t size = file_size_get(path);
        char *buffer = calloc(size, 1);

        FILE *handle = fopen(path, "r");
        if(handle == nullptr) return nullptr;

        fread(buffer, 1, size, handle);
        fclose(handle);

        return buffer;
}

