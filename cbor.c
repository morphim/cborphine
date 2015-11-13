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

#define CBOR_MAX_INJECTED_LIMIT 24
#define CBOR_MAX_1BYTE_LIMIT    256
#define CBOR_MAX_2BYTE_LIMIT    65536
#define CBOR_MAX_4BYTE_LIMIT    4294967296

static cbor_token_type_t cbor_internal_types_map[] =
{
    CBOR_TOKEN_TYPE_INT,    /* 0 */
    CBOR_TOKEN_TYPE_INT,    /* 1 */
    CBOR_TOKEN_TYPE_BYTES,  /* 2 */
    CBOR_TOKEN_TYPE_STRING, /* 3 */
    CBOR_TOKEN_TYPE_ARRAY,  /* 4 */
    CBOR_TOKEN_TYPE_MAP,    /* 5 */
    CBOR_TOKEN_TYPE_TAG,    /* 6 */
    CBOR_TOKEN_TYPE_SPECIAL /* 7 */
};

static cbor_bool_t cbor_internal_extract_special_value(unsigned char minor_type, cbor_token_t *token)
{
    switch (minor_type)
    {
    case 20: /* false */
        token->type = CBOR_TOKEN_TYPE_BOOLEAN;
        token->int_value = CBOR_FALSE;
        return CBOR_TRUE;
    case 21: /* true */
        token->type = CBOR_TOKEN_TYPE_BOOLEAN;
        token->int_value = CBOR_TRUE;
        return CBOR_TRUE;
    case 22: /* null */
        token->type = CBOR_TOKEN_TYPE_NULL;
        return CBOR_TRUE;
    case 23: /* undefined */
        token->type = CBOR_TOKEN_TYPE_UNDEFINED;
        return CBOR_TRUE;
    }

    return CBOR_FALSE;
}

static int cbor_internal_get_length(unsigned char minor_type)
{
    if (minor_type < 24)
        return 0;

    switch (minor_type)
    {
    case 24: return 1;
    case 25: return 2;
    case 26: return 4;
    case 27: return 8;
    }

    return -1;
}

static unsigned char *cbor_internal_write_int_value(unsigned char *data, size_t size, unsigned int type, size_t check_bytes, cbor_base_uint_t value)
{
    if (value < CBOR_MAX_INJECTED_LIMIT)
    {
        if (size < 1 + check_bytes)
            return data;

        *data++ = (unsigned char) ((type << 5) | value);

        return data;
    }
    else if (value < CBOR_MAX_1BYTE_LIMIT)
    {
        if (size < 2 + check_bytes)
            return data;

        *data++ = (unsigned char) ((type << 5) | 24);
        *data++ = (unsigned char) value;

        return data;
    }
    else if (value < CBOR_MAX_2BYTE_LIMIT)
    {
        if (size < 3 + check_bytes)
            return data;

        *data++ = (unsigned char) ((type << 5) | 25);
        *data++ = (unsigned char) (value >> 8);
        *data++ = (unsigned char) value;

        return data;
    }
#ifdef CBOR_LONGLONG_SUPPORT
    else if (value < CBOR_MAX_4BYTE_LIMIT)
#else
    else
#endif
    {
        if (size < 5 + check_bytes)
            return data;

        *data++ = (unsigned char) ((type << 5) | 26);
        *data++ = (unsigned char) (value >> 24);
        *data++ = (unsigned char) (value >> 16);
        *data++ = (unsigned char) (value >> 8);
        *data++ = (unsigned char) value;

        return data;
    }
#ifdef CBOR_LONGLONG_SUPPORT
    else
    {
        if (size < 9 + check_bytes)
            return data;

        *data++ = (unsigned char) ((type << 5) | 27);
        *data++ = (unsigned char) (value >> 56);
        *data++ = (unsigned char) (value >> 48);
        *data++ = (unsigned char) (value >> 40);
        *data++ = (unsigned char) (value >> 32);
        *data++ = (unsigned char) (value >> 24);
        *data++ = (unsigned char) (value >> 16);
        *data++ = (unsigned char) (value >> 8);
        *data++ = (unsigned char) value;

        return data;
    }
#endif
}

static unsigned char *cbor_internal_write_bytes(unsigned char *data, size_t size, unsigned int type, size_t bytes_size, const unsigned char *bytes)
{
    unsigned char *result = cbor_internal_write_int_value(data, size, type, bytes_size, bytes_size);
    if (result > data)
    {
        memcpy(result, bytes, bytes_size);
        return result + bytes_size;
    }
    return result;
}

unsigned char *cbor_read_token(unsigned char *data, unsigned char *end, cbor_token_t *token)
{
    unsigned char *current_pos;
    unsigned char type;
    unsigned char major_type;
    unsigned char minor_type;
    int type_length;
    cbor_base_uint_t length_value;

    if (data >= end)
    {
        token->type = CBOR_TOKEN_TYPE_END;
        return data;
    }

    current_pos = data;
    type = *current_pos++;
    major_type = type >> 5;
    minor_type = type & 31;
    type_length = cbor_internal_get_length(minor_type);

    if (type_length < 0)
    {
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_value = "invalid length";
        return data;
    }

    if ((size_t)(end - current_pos) < (size_t)type_length)
    {
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_value = "insufficient data";
        return data;
    }

    switch (type_length)
    {
    case 0:
        length_value = minor_type;
        break;
    case 1:
        length_value = *current_pos;
        break;
    case 2:
        length_value = ((unsigned short)*(current_pos + 0) << 8) |
                       ((unsigned short)*(current_pos + 1));
        break;
    case 4:
        length_value = ((unsigned int)*(current_pos + 0) << 24) |
                       ((unsigned int)*(current_pos + 1) << 16) |
                       ((unsigned int)*(current_pos + 2) << 8)  |
                       ((unsigned int)*(current_pos + 3));
        break;
    case 8:
#ifdef CBOR_LONGLONG_SUPPORT
        length_value = ((unsigned long long)*(current_pos + 0) << 56) |
                       ((unsigned long long)*(current_pos + 1) << 48) |
                       ((unsigned long long)*(current_pos + 2) << 40) |
                       ((unsigned long long)*(current_pos + 3) << 32) |
                       ((unsigned long long)*(current_pos + 4) << 24) |
                       ((unsigned long long)*(current_pos + 5) << 16) |
                       ((unsigned long long)*(current_pos + 6) << 8)  |
                       ((unsigned long long)*(current_pos + 7));
        break;
#else
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_value = "64 bits integers are not supported";
        return data;
#endif
    }

    current_pos += type_length;

    switch (major_type)
    {
    case 0: /* positive integer */
        token->type = CBOR_TOKEN_TYPE_INT;
        token->sign = 1;
        token->int_value = length_value;
        return current_pos;
    case 1: /* negative integer */
        token->type = CBOR_TOKEN_TYPE_INT;
        token->sign = -1;
        token->int_value = length_value + 1;
        return current_pos;
    case 2: /* bytes */
    case 3: /* string */
        if ((size_t)(end - current_pos) < length_value)
        {
            token->type = CBOR_TOKEN_TYPE_ERROR;
            token->error_value = "insufficient data";
            return data;
        }
        token->type = cbor_internal_types_map[major_type];
        token->int_value = length_value;
        token->bytes_value = current_pos;
        return current_pos + length_value;
    case 4: /* array */
    case 5: /* map */
    case 6: /* tag */
        token->type = cbor_internal_types_map[major_type];
        token->int_value = length_value;
        return current_pos;
    case 7: /* special */
        if (cbor_internal_extract_special_value(minor_type, token))
            return current_pos;

        token->type = CBOR_TOKEN_TYPE_SPECIAL;
        token->int_value = length_value;
        return current_pos;
    default:
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_value = "unknown error";
        return data;
    }
}

unsigned char *cbor_write_pint(unsigned char *data, size_t size, cbor_base_uint_t value)
{
    return cbor_internal_write_int_value(data, size, 0, 0, value);
}

unsigned char *cbor_write_nint(unsigned char *data, size_t size, cbor_base_uint_t value)
{
    return cbor_internal_write_int_value(data, size, 1, 0, value - 1);
}

unsigned char *cbor_write_uint(unsigned char *data, size_t size, cbor_base_uint_t value)
{
    return cbor_write_pint(data, size, value);
}

unsigned char *cbor_write_int(unsigned char *data, size_t size, cbor_base_int_t value)
{
    if (value < 0)
        return cbor_write_nint(data, size, (unsigned long long)(-value));
    else
        return cbor_write_pint(data, size, (unsigned long long)(value));
}

unsigned char *cbor_write_boolean(unsigned char *data, size_t size, cbor_bool_t value)
{
    if (value)
        return cbor_internal_write_int_value(data, size, 7, 0, 21); /* 21 is true */
    else
        return cbor_internal_write_int_value(data, size, 7, 0, 20); /* 20 is false */
}

unsigned char *cbor_write_null(unsigned char *data, size_t size)
{
    return cbor_internal_write_int_value(data, size, 7, 0, 22); /* 22 is null */
}

unsigned char *cbor_write_undefined(unsigned char *data, size_t size)
{
    return cbor_internal_write_int_value(data, size, 7, 0, 23); /* 23 is undefined */
}

unsigned char *cbor_write_string_with_len(unsigned char *data, size_t size, const char *str, size_t str_length)
{
    return cbor_internal_write_bytes(data, size, 3, str_length, (unsigned char *) str);
}

unsigned char *cbor_write_string(unsigned char *data, size_t size, const char *str)
{
    return cbor_internal_write_bytes(data, size, 3, strlen(str), (unsigned char *) str);
}

unsigned char *cbor_write_bytes(unsigned char *data, size_t size, const unsigned char *bytes, size_t bytes_size)
{
    return cbor_internal_write_bytes(data, size, 2, bytes_size, bytes);
}

unsigned char *cbor_write_array(unsigned char *data, size_t size, cbor_base_uint_t array_size)
{
    return cbor_internal_write_int_value(data, size, 4, 0, array_size);
}

unsigned char *cbor_write_map(unsigned char *data, size_t size, cbor_base_uint_t map_size)
{
    return cbor_internal_write_int_value(data, size, 5, 0, map_size);
}

unsigned char *cbor_write_tag(unsigned char *data, size_t size, cbor_base_uint_t tag)
{
    return cbor_internal_write_int_value(data, size, 6, 0, tag);
}

unsigned char *cbor_write_special(unsigned char *data, size_t size, cbor_base_uint_t special)
{
    return cbor_internal_write_int_value(data, size, 7, 0, special);
}
