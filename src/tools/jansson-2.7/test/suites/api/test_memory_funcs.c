/*
** Copyright 2005-2018  Solarflare Communications Inc.
**                      7505 Irvine Center Drive, Irvine, CA 92618, USA
** Copyright 2002-2005  Level 5 Networks Inc.
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of version 2 of the GNU General Public License as
** published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/

#include <string.h>
#include <jansson.h>

#include "util.h"

static int malloc_called = 0;
static int free_called = 0;

/* helper */
static void create_and_free_complex_object()
{
    json_t *obj;

    obj = json_pack("{s:i,s:n,s:b,s:b,s:{s:s},s:[i,i,i]",
                    "foo", 42,
                    "bar",
                    "baz", 1,
                    "qux", 0,
                    "alice", "bar", "baz",
                    "bob", 9, 8, 7);

    json_decref(obj);
}

static void *my_malloc(size_t size)
{
    malloc_called = 1;
    return malloc(size);
}

static void my_free(void *ptr)
{
    free_called = 1;
    free(ptr);
}

static void test_simple()
{
    json_set_alloc_funcs(my_malloc, my_free);
    create_and_free_complex_object();

    if(malloc_called != 1 || free_called != 1)
        fail("Custom allocation failed");
}


/*
  Test the secure memory functions code given in the API reference
  documentation, but by using plain memset instead of
  guaranteed_memset().
*/

static void *secure_malloc(size_t size)
{
    /* Store the memory area size in the beginning of the block */
    void *ptr = malloc(size + 8);
    *((size_t *)ptr) = size;
    return (char *)ptr + 8;
}

static void secure_free(void *ptr)
{
    size_t size;

    ptr = (char *)ptr - 8;
    size = *((size_t *)ptr);

    /*guaranteed_*/memset(ptr, 0, size + 8);
    free(ptr);
}

static void test_secure_funcs(void)
{
    json_set_alloc_funcs(secure_malloc, secure_free);
    create_and_free_complex_object();
}

static void run_tests()
{
    test_simple();
    test_secure_funcs();
}
