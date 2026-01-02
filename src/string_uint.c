#include "string_uint.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

int readstring(int fd, struct string *sbuf) {
    uint32_t length_be;
    if (read(fd, &length_be, sizeof(uint32_t)) < 0) return 1;
    sbuf->length = be32toh(length_be);

    sbuf->data = malloc((sbuf->length + 1) * sizeof(uint8_t));
    if(!sbuf->data) return 1;

    if (read(fd, sbuf->data, sbuf->length * sizeof(uint8_t)) != (ssize_t)sbuf->length * sizeof(uint8_t)) return 1;
    sbuf->data[sbuf->length] ='\0';
    return 0;
}

int writestring(int fd, struct string *sbuf) {
    uint32_t length_be = htobe32(sbuf->length);
    if (write(fd, &length_be, sizeof(uint32_t)) != sizeof(uint32_t)) return 1;

    if (write(fd, sbuf->data,sbuf->length * sizeof(uint8_t)) != (ssize_t)sbuf->length * sizeof(uint8_t)) return 1;
    return 0;
}

void freestring(struct string *sbuf) {
    if (sbuf->data != NULL) {
        free(sbuf->data);
    }
}

int catstring(struct string *newstring, struct string oldstring) {
    if (newstring->length == 0) {
        newstring->data = NULL;
    }
    newstring->length += oldstring.length;
    uint8_t *tmp = realloc(newstring->data, (newstring->length + 1) * sizeof(uint8_t));
    if (!tmp) return 1;
    newstring->data = tmp;
    for (int i = 0; i < oldstring.length; i++) {
        newstring->data[newstring->length - oldstring.length + i] = oldstring.data[i];
    }
    newstring->data[newstring->length] = '\0';
    return 0;
}

int uint_to_string(uint64_t n, struct string *s) {
    if (!s) {
        s->length = 0;
        s->data = NULL;
    }
    uint32_t len = snprintf(NULL, 0,"%zu", n);
    if (len < 0) return 1;

    uint8_t *tmp = realloc(s->data ,(s->length + len + 1) * sizeof(uint8_t));
    if (!tmp) return 1;

    s->data = tmp;
    snprintf((char*)(s->data + s->length), len + 1,"%zu", n);
    s->length +=len;
    return 0;
}

void insertion_sort_strings(struct string *arr, uint32_t n) {
    for (uint32_t i = 1; i < n; i++) {
        struct string key = arr[i];
        int32_t j = i - 1;

        while (j >= 0 && arr[j].length > key.length) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}


void string_to_uint64(struct string s, uint64_t *n) {
    *n = 0;
    for (int i = 0; i < s.length; i++) {
        *n = (*n) * 10 + (s.data[i] - '0');
    }
}