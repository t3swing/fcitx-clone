#include "utf8.h"

#define CONT(i)   ISUTF8_CB(in[i])
#define VAL(i, s) ((in[i]&0x3f) << s)

size_t
utf8_strlen(const char *s)
{
    unsigned int l = 0;
    
    while(*s)
    {
        int chr;
        
        s = utf8_get_char(s, &chr);
        l++;
    }
    
    return l;
}

int utf8_char_len(const char *in)
{
    if (!(in[0] & 0x80))
        return 1;
    /* 2-byte, 0x80-0x7ff */
    if ( (in[0]&0xe0) == 0xc0 && CONT(1) )
        return 2;
    /* 3-byte, 0x800-0xffff */
    if ( (in[0]&0xf0) == 0xe0 && CONT(1) && CONT(2) )
        return 3;
    /* 4-byte, 0x10000-0x1FFFFF */
    if ( (in[0]&0xf8) == 0xf0 && CONT(1) && CONT(2) && CONT(3) )
        return 4;
    /* 5-byte, 0x200000-0x3FFFFFF */
    if ( (in[0]&0xfc) == 0xf8 && CONT(1) && CONT(2) && CONT(3) && CONT(4) )
        return 5;
    /* 6-byte, 0x400000-0x7FFFFFF */
    if ( (in[0]&0xfe) == 0xfc && CONT(1) && CONT(2) && CONT(3) && CONT(4) && CONT(5) )
        return 6;
    
    return 1;
}

char *
utf8_get_char(const char *in, int *chr)
{
    if (!(in[0] & 0x80))
    {
        *(chr) = *(in);
        return (char *)in+1;
    }
    /* 2-byte, 0x80-0x7ff */
    if ( (in[0]&0xe0) == 0xc0 && CONT(1) )
    {
        *chr = ((in[0]&0x1f) << 6)|VAL(1,0);
        return (char *)in+2;
    }
    /* 3-byte, 0x800-0xffff */
    if ( (in[0]&0xf0) == 0xe0 && CONT(1) && CONT(2) )
    {
        *chr = ((in[0]&0xf) << 12)|VAL(1,6)|VAL(2,0);
        return (char *)in+3;
    }
    /* 4-byte, 0x10000-0x1FFFFF */
    if ( (in[0]&0xf8) == 0xf0 && CONT(1) && CONT(2) && CONT(3) )
    {
        *chr = ((in[0]&0x7) << 18)|VAL(1,12)|VAL(2,6)|VAL(3,0);
        return (char *)in+4;
    }
    /* 5-byte, 0x200000-0x3FFFFFF */
    if ( (in[0]&0xfc) == 0xf8 && CONT(1) && CONT(2) && CONT(3) && CONT(4) )
    {
        *chr = ((in[0]&0x3) << 24)|VAL(1,18)|VAL(2,12)|VAL(3,6)|VAL(4,0);
        return (char *)in+5;
    }
    /* 6-byte, 0x400000-0x7FFFFFF */
    if ( (in[0]&0xfe) == 0xfc && CONT(1) && CONT(2) && CONT(3) && CONT(4) && CONT(5) )
    {
        *chr = ((in[0]&0x1) << 30)|VAL(1,24)|VAL(2,18)|VAL(3,12)|VAL(4,6)|VAL(5,0);
        return (char *)in+4;
    }
    
    *chr = *in;
    
    return (char *)in+1;
}

int utf8_strncmp(const char *s1, const char *s2, int n) {
    // Seems to work.
    int c1,c2;
    int i;
    for (i=0;i < n;i++) {
        if (!(*s1 && 0x80))
        {
            if (*s1 != *s2)
                return 1;
            if (*s1 == 0)
                return 0;
            s1 ++;
            s2 ++;
        }
        else {
            s1 = utf8_get_char(s1, &c1);
            s2 = utf8_get_char(s2, &c2);
            if (c1 != c2)
                return 1;
        }
    }
    return 0;
}

char* utf8_get_nth_char(char* s, unsigned int n)
{
    unsigned int l = 0;
    
    while(*s && l < n)
    {
        int chr;
        
        s = utf8_get_char(s, &chr);
        l++;
    }

    return s;
}

/* Modeline for ViM {{{
 * vim: ts=4 et sw=4
 * }}} */


