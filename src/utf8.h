#ifndef UTF8_H
#  define UTF8_H

#include <stdlib.h>

#define ISUTF8_CB(c)  (((c)&0xc0) == 0x80)

#define UTF8_MAX_LENGTH 4

extern size_t utf8_strlen(const char *s);
extern char* utf8_get_char(const char *in, int *chr);
extern int utf8_strncmp(const char *s1, const char *s2, int n);
extern int utf8_char_len(const char *in);

#endif /* ifndef UTF8_H */

