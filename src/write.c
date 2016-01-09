/*
Copyright (c) 2015 drugaddicted - c17h19no3 AT openmailbox DOT org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <string.h>
#include "cbor.h"
#include "internal.h"

#define CBOR_MAX_INJECTED_LIMIT 24
#define CBOR_MAX_1BYTE_LIMIT    256
#define CBOR_MAX_2BYTE_LIMIT    65536
#define CBOR_MAX_4BYTE_LIMIT    4294967296

#define CBOR_CREATE_INITIAL_BYTE(major_type, minor_type) ((uint8_t) (((major_type) << 5) | (minor_type)))

CBOR_INLINE cbor_bool_t cbor_internal_write_int_value(uint8_t **data, size_t size, unsigned int type, size_t check_bytes, cbor_base_uint_t value)
{
    uint8_t *pos = *data;

    if (value < CBOR_MAX_INJECTED_LIMIT)
    {
        if (size < 1 + check_bytes)
            return CBOR_FALSE;

        *pos++ = CBOR_CREATE_INITIAL_BYTE(type, value);

        *data = pos;
        return CBOR_TRUE;
    }
    else if (value < CBOR_MAX_1BYTE_LIMIT)
    {
        if (size < 2 + check_bytes)
            return CBOR_FALSE;

        *pos++ = CBOR_CREATE_INITIAL_BYTE(type, 24);
        *pos++ = (uint8_t) value;

        *data = pos;
        return CBOR_TRUE;
    }
    else if (value < CBOR_MAX_2BYTE_LIMIT)
    {
        uint16_t short_value;

        if (size < 3 + check_bytes)
            return CBOR_FALSE;

        *pos++ = CBOR_CREATE_INITIAL_BYTE(type, 25);

        short_value = (uint16_t)value;
        *data = cbor_internal_swap_2bytes(pos, (const uint8_t *)&short_value);
        return CBOR_TRUE;
    }
#ifdef CBOR_INT64_SUPPORT
    else if (value < CBOR_MAX_4BYTE_LIMIT)
#else
    else
#endif
    {
        uint32_t int_value;

        if (size < 5 + check_bytes)
            return CBOR_FALSE;

        *pos++ = CBOR_CREATE_INITIAL_BYTE(type, 26);

        int_value = (uint32_t)value;
        *data = cbor_internal_swap_4bytes(pos, (const uint8_t *)&int_value);
        return CBOR_TRUE;
    }
#ifdef CBOR_INT64_SUPPORT
    else
    {
        if (size < 9 + check_bytes)
            return CBOR_FALSE;

        *pos++ = CBOR_CREATE_INITIAL_BYTE(type, 27);

        *data = cbor_internal_swap_8bytes(pos, (const uint8_t *)&value);
        return CBOR_TRUE;
    }
#endif
}

CBOR_INLINE cbor_bool_t cbor_internal_write_float_value(uint8_t **data, size_t size, size_t type_length, const uint8_t *value_bytes)
{
    uint8_t *pos = *data;

    switch (type_length)
    {
    case 2:
        if (size < 3)
            return CBOR_FALSE;

        *pos++ = CBOR_CREATE_INITIAL_BYTE(7, 25);
        *data = cbor_internal_swap_2bytes(pos, value_bytes);
        return CBOR_TRUE;
    case 4:
        if (size < 5)
            return CBOR_FALSE;

        *pos++ = CBOR_CREATE_INITIAL_BYTE(7, 26);
        *data = cbor_internal_swap_4bytes(pos, value_bytes);
        return CBOR_TRUE;
    case 8:
        if (size < 9)
            return CBOR_FALSE;

        *pos++ = CBOR_CREATE_INITIAL_BYTE(7, 27);
        *data = cbor_internal_swap_8bytes(pos, value_bytes);
        return CBOR_TRUE;
    }

    return CBOR_FALSE;
}

CBOR_INLINE cbor_bool_t cbor_internal_write_bytes(uint8_t **data, size_t size, unsigned int type, size_t bytes_size, const uint8_t *bytes)
{
    if (cbor_internal_write_int_value(data, size, type, bytes_size, bytes_size) == CBOR_FALSE)
        return CBOR_FALSE;

    memcpy(*data, bytes, bytes_size);
    *data += bytes_size;
    return CBOR_TRUE;
}

cbor_bool_t cbor_write_uint(uint8_t **data, size_t size, cbor_base_uint_t value)
{
    return cbor_internal_write_int_value(data, size, 0, 0, value);
}

cbor_bool_t cbor_write_int(uint8_t **data, size_t size, cbor_base_int_t value)
{
    if (value < 0)
        return cbor_internal_write_int_value(data, size, 1, 0, (cbor_base_uint_t)(-(value + 1)));
    else
        return cbor_internal_write_int_value(data, size, 0, 0, (cbor_base_uint_t)(value));
}

cbor_bool_t cbor_write_float(uint8_t **data, size_t size, float value)
{
    return cbor_internal_write_float_value(data, size, sizeof(value), (const uint8_t *)&value);
}

cbor_bool_t cbor_write_double(uint8_t **data, size_t size, double value)
{
    return cbor_internal_write_float_value(data, size, sizeof(value), (const uint8_t *)&value);
}

cbor_bool_t cbor_write_boolean(uint8_t **data, size_t size, cbor_bool_t value)
{
    if (value)
        return cbor_internal_write_int_value(data, size, 7, 0, 21); /* 21 is true */
    else
        return cbor_internal_write_int_value(data, size, 7, 0, 20); /* 20 is false */
}

cbor_bool_t cbor_write_null(uint8_t **data, size_t size)
{
    return cbor_internal_write_int_value(data, size, 7, 0, 22); /* 22 is null */
}

cbor_bool_t cbor_write_undefined(uint8_t **data, size_t size)
{
    return cbor_internal_write_int_value(data, size, 7, 0, 23); /* 23 is undefined */
}

cbor_bool_t cbor_write_string_with_len(uint8_t **data, size_t size, const char *str, size_t str_length)
{
    return cbor_internal_write_bytes(data, size, 3, str_length, (uint8_t *) str);
}

cbor_bool_t cbor_write_string(uint8_t **data, size_t size, const char *str)
{
    return cbor_internal_write_bytes(data, size, 3, strlen(str), (uint8_t *) str);
}

cbor_bool_t cbor_write_bytes(uint8_t **data, size_t size, const uint8_t *bytes, size_t bytes_size)
{
    return cbor_internal_write_bytes(data, size, 2, bytes_size, bytes);
}

cbor_bool_t cbor_write_array(uint8_t **data, size_t size, cbor_base_uint_t array_size)
{
    return cbor_internal_write_int_value(data, size, 4, 0, array_size);
}

cbor_bool_t cbor_write_map(uint8_t **data, size_t size, cbor_base_uint_t map_size)
{
    return cbor_internal_write_int_value(data, size, 5, 0, map_size);
}

cbor_bool_t cbor_write_tag(uint8_t **data, size_t size, cbor_base_uint_t tag)
{
    return cbor_internal_write_int_value(data, size, 6, 0, tag);
}

cbor_bool_t cbor_write_special(uint8_t **data, size_t size, uint8_t special)
{
    return cbor_internal_write_int_value(data, size, 7, 0, special);
}
