/***************************************************************************
 *   Copyright (C) 2002~2005 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/**
 * @file array.h
 * @author CSSlayer wengxt@gmail.com
 * @date 2010-04-25
 *
 * @brief 动态数组
 */

#ifndef ARRAY_H
#define ARRAY_H

typedef struct ARRAY
{
    void* data;
    int len;
    int alloc;
    int eleSize;
} ARRAY;

#define MIN_ARRAY_SIZE 16

#define ArrayElementLen(array, len) ((array)->eleSize * (len))
#define ArrayPos(array, len) ((array)->data +ArrayElementLen((array),len))
#define ArrayIndex(array, cell) ((cell)?((((void*)cell) - (array)->data) / (array)->eleSize):(array)->len)
#define ArrayCell(array, len, type) (((type*)((array)->data +ArrayElementLen((array),len)))[0])
#define MAX(a,b) ((a) > (b)?(a):(b))

#define ArrayAddVals(array, d, l, s)                                              \
do {                                                                              \
    if (!(array))                                                                 \
    {                                                                             \
        (array) = malloc(sizeof(ARRAY));                                          \
        (array)->data = NULL;                                                     \
        (array)->len = 0;                                                         \
        (array)->alloc = 0;                                                       \
        (array)->eleSize = s;                                                     \
    }                                                                             \
    ArrayExpand(array, l);                                                        \
    memcpy(ArrayPos((array), (array)->len),                                       \
           d, ArrayElementLen(array , l));                                        \
    (array)->len += l;                                                            \
} while (0)

#define ArrayExpand(array, l)                                                     \
do {                                                                              \
    size_t wantAlloc = ArrayElementLen(array, array->len + l);                    \
    if ((array)->alloc < wantAlloc)                                               \
    {                                                                             \
        wantAlloc = MAX ( wantAlloc, MIN_ARRAY_SIZE);                             \
        (array)->data = realloc((array)->data, wantAlloc);                        \
        (array)->alloc = wantAlloc;                                               \
    }                                                                             \
} while (0)

#define ArrayFree(array)                                                          \
do {                                                                              \
    free((array)->data);                                                          \
    free(array);                                                                  \
    array = NULL;                                                                 \
} while(0);

#define ArrayForeach(iter, array) \
    for (iter = 0; iter < (array)->len; iter ++)

#define ArrayForeachFrom(initial , iter, array) \
    for (iter = (initial); iter < (array)->len; iter ++)

#define ArraySort(array, cmpFunc) \
    qsort((array)->data, (array)->len, (array)->eleSize, cmpFunc)

void *custom_bsearch(const void *key, const void *base,
		size_t nmemb, size_t size, int accurate,
		int (*compar)(const void *, const void *));

#define ArrayBsearch(key, array, accurate, cmpFunc) \
    custom_bsearch((void*)key, (array)->data, (array)->len, (array)->eleSize, accurate, cmpFunc)

#define ArrayAddVal(array, d, s) ArrayAddVals(array, d, 1, s)
#define ArrayAddInt(array, intdata) ArrayAddVal(array, intdata, sizeof(int))
#define ArrayAddPointer(array, pdata) ArrayAddVal(array, pdata, sizeof(void*))

#endif /* ifndef ARRAY_H */

