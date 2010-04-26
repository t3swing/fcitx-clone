/*
 * $Id$
 */

#include <stdlib.h>
#include "array.h"

void *custom_bsearch(const void *key, const void *base,
        size_t nmemb, size_t size, int accurate,
        int (*compar)(const void *, const void *))
{
    if (accurate)
        return bsearch(key, base, nmemb, size, compar);
    else
    {
        size_t l, u, idx;
        const void *p;
        int comparison;
        
        l = 0;
        u = nmemb;
        while (l < u)
        {
            idx = (l + u) / 2;
            p = (void *) (((const char *) base) + (idx * size));
            comparison = (*compar) (key, p);
            if (comparison <= 0)
                u = idx;
            else if (comparison > 0)
                l = idx + 1;
        }

        if (u >= nmemb)
            return NULL;
        else
            return (void *) (((const char *) base) + (l * size));
    }
}

