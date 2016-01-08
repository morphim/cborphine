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

#define CBOR_CREATE_INITIAL_BYTE(major_type, minor_type) ((uint8_t) (((major_type) << 5) | (minor_type)))
#define CBOR_GET_MAJOR_TYPE(initial_byte) ((initial_byte) >> 5)
#define CBOR_GET_MINOR_TYPE(initial_byte) ((initial_byte) & 31)

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

static uint8_t *cbor_internal_swap_2bytes(uint8_t *dest, const uint8_t *src)
{
#ifdef CBOR_BIGENDIAN_PLATFORM
    memcpy(dest, src, 2);
#else
    uint16_t orig_value = *(uint16_t *)src;
    *(uint16_t *)dest = (orig_value << 8) |
                        (orig_value >> 8);
#endif

    return dest + 2;
}

static uint8_t *cbor_internal_swap_4bytes(uint8_t *dest, const uint8_t *src)
{
#ifdef CBOR_BIGENDIAN_PLATFORM
    memcpy(dest, src, 4);
#else
    uint32_t orig_value = *(uint32_t *)src;
    *(uint32_t *)dest = ((orig_value & 0x000000FF) << 24) |
                        ((orig_value & 0x0000FF00) << 8)  |
                        ((orig_value & 0x00FF0000) >> 8)  |
                        ((orig_value & 0xFF000000) >> 24);
#endif

    return dest + 4;
}

static uint8_t *cbor_internal_swap_8bytes(uint8_t *dest, const uint8_t *src)
{
#ifdef CBOR_BIGENDIAN_PLATFORM
    memcpy(dest, src, 8);
    return dest + 8;
#else
    return cbor_internal_swap_4bytes(cbor_internal_swap_4bytes(dest, src + 4), src);
#endif
}

static int cbor_internal_get_width(unsigned int minor_type)
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

static cbor_bool_t cbor_internal_read_int_value(unsigned int minor_type, const uint8_t **pos, const uint8_t *end, cbor_token_t *token)
{
    const uint8_t *current_pos = *pos;
    int type_width = cbor_internal_get_width(minor_type);

    if (type_width < 0)
    {
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_value = "invalid length";
        return CBOR_FALSE;
    }

    if ((size_t)(end - current_pos) < (size_t)type_width)
    {
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_value = "insufficient data";
        return CBOR_FALSE;
    }

    switch (type_width)
    {
    case 0:
        token->int_value = minor_type;
        return CBOR_TRUE;
    case 1:
        token->int_value = *current_pos;
        *pos = current_pos + 1;
        return CBOR_TRUE;
    case 2:
        {
            uint16_t short_value;
            cbor_internal_swap_2bytes((uint8_t *)&short_value, current_pos);
            token->int_value = short_value;
            *pos = current_pos + 2;
            return CBOR_TRUE;
        }
    case 4:
        {
            uint32_t int_value;
            cbor_internal_swap_4bytes((uint8_t *)&int_value, current_pos);
            token->int_value = int_value;
            *pos = current_pos + 4;
            return CBOR_TRUE;
        }
    case 8:
#ifdef CBOR_INT64_SUPPORT
        {
            cbor_base_uint_t value;
            cbor_internal_swap_8bytes((uint8_t *)&value, current_pos);
            token->int_value = value;
            *pos = current_pos + 8;
            return CBOR_TRUE;
        }
#else
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_value = "64 bits integers are not supported";
        return CBOR_FALSE;
#endif
    }

    token->type = CBOR_TOKEN_TYPE_ERROR;
    token->error_value = "unknown error";
    return CBOR_FALSE;
}

static cbor_bool_t cbor_internal_extract_special_value(unsigned int minor_type, const uint8_t **pos, const uint8_t *end, cbor_token_t *token)
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
    case 26: /* single-precision float */
        {
            float float_value;
            const uint8_t *current_pos = *pos;

            if ((size_t)(end - current_pos) < 4)
            {
                token->type = CBOR_TOKEN_TYPE_ERROR;
                token->error_value = "insufficient data";
                return CBOR_FALSE;
            }

            cbor_internal_swap_4bytes((uint8_t *)&float_value, current_pos);

            token->type = CBOR_TOKEN_TYPE_FLOAT;
            token->float_value = float_value;
            *pos = current_pos + 4;
            return CBOR_TRUE;
        }
    case 27: /* double-precision float */
        {
            double double_value;
            const uint8_t *current_pos = *pos;

            if ((size_t)(end - current_pos) < 8)
            {
                token->type = CBOR_TOKEN_TYPE_ERROR;
                token->error_value = "insufficient data";
                return CBOR_FALSE;
            }

            cbor_internal_swap_8bytes((uint8_t *)&double_value, current_pos);

            token->type = CBOR_TOKEN_TYPE_FLOAT;
            token->float_value = double_value;
            *pos = current_pos + 8;
            return CBOR_TRUE;
        }
    default:
        {
            if (cbor_internal_read_int_value(minor_type, pos, end, token) == CBOR_FALSE)
                return CBOR_FALSE;

            token->type = CBOR_TOKEN_TYPE_SPECIAL;
            return CBOR_TRUE;
        }
    }
}

static uint8_t *cbor_internal_write_int_value(uint8_t *data, size_t size, unsigned int type, size_t check_bytes, cbor_base_uint_t value)
{
    if (value < CBOR_MAX_INJECTED_LIMIT)
    {
        if (size < 1 + check_bytes)
            return data;

        *data++ = CBOR_CREATE_INITIAL_BYTE(type, value);

        return data;
    }
    else if (value < CBOR_MAX_1BYTE_LIMIT)
    {
        if (size < 2 + check_bytes)
            return data;

        *data++ = CBOR_CREATE_INITIAL_BYTE(type, 24);
        *data++ = (uint8_t) value;

        return data;
    }
    else if (value < CBOR_MAX_2BYTE_LIMIT)
    {
        uint16_t short_value;

        if (size < 3 + check_bytes)
            return data;

        *data++ = CBOR_CREATE_INITIAL_BYTE(type, 25);

        short_value = (uint16_t)value;
        return cbor_internal_swap_2bytes(data, (const uint8_t *)&short_value);
    }
#ifdef CBOR_INT64_SUPPORT
    else if (value < CBOR_MAX_4BYTE_LIMIT)
#else
    else
#endif
    {
        uint32_t int_value;

        if (size < 5 + check_bytes)
            return data;

        *data++ = CBOR_CREATE_INITIAL_BYTE(type, 26);

        int_value = (uint32_t)value;
        return cbor_internal_swap_4bytes(data, (const uint8_t *)&int_value);
    }
#ifdef CBOR_INT64_SUPPORT
    else
    {
        if (size < 9 + check_bytes)
            return data;

        *data++ = CBOR_CREATE_INITIAL_BYTE(type, 27);

        return cbor_internal_swap_8bytes(data, (const uint8_t *)&value);
    }
#endif
}

static uint8_t *cbor_internal_write_float_value(uint8_t *data, size_t size, size_t type_length, const uint8_t *value_bytes)
{
    switch (type_length)
    {
    case 2:
        if (size < 3)
            return data;

        *data++ = CBOR_CREATE_INITIAL_BYTE(7, 25);
        return cbor_internal_swap_2bytes(data, value_bytes);
    case 4:
        if (size < 5)
            return data;

        *data++ = CBOR_CREATE_INITIAL_BYTE(7, 26);
        return cbor_internal_swap_4bytes(data, value_bytes);
    case 8:
        if (size < 9)
            return data;

        *data++ = CBOR_CREATE_INITIAL_BYTE(7, 27);
        return cbor_internal_swap_8bytes(data, value_bytes);
    }

    return data;
}

static uint8_t *cbor_internal_write_bytes(uint8_t *data, size_t size, unsigned int type, size_t bytes_size, const uint8_t *bytes)
{
    uint8_t *result = cbor_internal_write_int_value(data, size, type, bytes_size, bytes_size);
    if (result > data)
    {
        memcpy(result, bytes, bytes_size);
        return result + bytes_size;
    }
    return result;
}

static cbor_bool_t cbor_internal_read_and_check_type(const uint8_t **data, size_t data_size, cbor_token_type_t expected_type, cbor_token_t *token)
{
    const uint8_t *end = *data + data_size;

    if (cbor_read_token(data, end, token) == CBOR_FALSE)
        return CBOR_FALSE;

    if (token->type != expected_type)
        return CBOR_FALSE;

    return CBOR_TRUE;
}

cbor_bool_t cbor_read_token(const uint8_t **data, const uint8_t *end, cbor_token_t *token)
{
    uint8_t initial_byte;
    unsigned int major_type;
    unsigned int minor_type;
    const uint8_t *current_pos = *data;

    if (current_pos >= end)
    {
        token->type = CBOR_TOKEN_TYPE_END;
        return CBOR_TRUE;
    }

    initial_byte = *current_pos++;
    major_type = CBOR_GET_MAJOR_TYPE(initial_byte);
    minor_type = CBOR_GET_MINOR_TYPE(initial_byte);

    switch (major_type)
    {
    case 0: /* positive integer */
        if (cbor_internal_read_int_value(minor_type, &current_pos, end, token) == CBOR_FALSE)
            return CBOR_FALSE;

        token->type = CBOR_TOKEN_TYPE_INT;
        token->sign = 1;
        *data = current_pos;
        return CBOR_TRUE;
    case 1: /* negative integer */
        if (cbor_internal_read_int_value(minor_type, &current_pos, end, token) == CBOR_FALSE)
            return CBOR_FALSE;

        token->type = CBOR_TOKEN_TYPE_INT;
        token->int_value = token->int_value + 1;
        token->sign = -1;
        *data = current_pos;
        return CBOR_TRUE;
    case 2: /* bytes */
    case 3: /* string */
        if (cbor_internal_read_int_value(minor_type, &current_pos, end, token) == CBOR_FALSE)
            return CBOR_FALSE;

        if ((size_t)(end - current_pos) < token->int_value)
        {
            token->type = CBOR_TOKEN_TYPE_ERROR;
            token->error_value = "insufficient data";
            return CBOR_FALSE;
        }

        token->type = cbor_internal_types_map[major_type];
        token->bytes_value = current_pos;
        *data = current_pos + token->int_value;
        return CBOR_TRUE;
    case 4: /* array */
    case 5: /* map */
    case 6: /* tag */
        if (cbor_internal_read_int_value(minor_type, &current_pos, end, token) == CBOR_FALSE)
            return CBOR_FALSE;

        token->type = cbor_internal_types_map[major_type];
        *data = current_pos;
        return CBOR_TRUE;
    case 7: /* special */
        if (cbor_internal_extract_special_value(minor_type, &current_pos, end, token) == CBOR_FALSE)
            return CBOR_FALSE;

        *data = current_pos;
        return CBOR_TRUE;
    default:
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_value = "unknown error";
        return CBOR_FALSE;
    }
}

uint8_t *cbor_write_uint(uint8_t *data, size_t size, cbor_base_uint_t value)
{
    return cbor_internal_write_int_value(data, size, 0, 0, value);
}

uint8_t *cbor_write_int(uint8_t *data, size_t size, cbor_base_int_t value)
{
    if (value < 0)
        return cbor_internal_write_int_value(data, size, 1, 0, (cbor_base_uint_t)(-value) - 1);
    else
        return cbor_internal_write_int_value(data, size, 0, 0, (cbor_base_uint_t)(value));
}

uint8_t *cbor_write_float(uint8_t *data, size_t size, float value)
{
    return cbor_internal_write_float_value(data, size, sizeof(value), (const uint8_t *)&value);
}

uint8_t *cbor_write_double(uint8_t *data, size_t size, double value)
{
    return cbor_internal_write_float_value(data, size, sizeof(value), (const uint8_t *)&value);
}

uint8_t *cbor_write_boolean(uint8_t *data, size_t size, cbor_bool_t value)
{
    if (value)
        return cbor_internal_write_int_value(data, size, 7, 0, 21); /* 21 is true */
    else
        return cbor_internal_write_int_value(data, size, 7, 0, 20); /* 20 is false */
}

uint8_t *cbor_write_null(uint8_t *data, size_t size)
{
    return cbor_internal_write_int_value(data, size, 7, 0, 22); /* 22 is null */
}

uint8_t *cbor_write_undefined(uint8_t *data, size_t size)
{
    return cbor_internal_write_int_value(data, size, 7, 0, 23); /* 23 is undefined */
}

uint8_t *cbor_write_string_with_len(uint8_t *data, size_t size, const char *str, size_t str_length)
{
    return cbor_internal_write_bytes(data, size, 3, str_length, (uint8_t *) str);
}

uint8_t *cbor_write_string(uint8_t *data, size_t size, const char *str)
{
    return cbor_internal_write_bytes(data, size, 3, strlen(str), (uint8_t *) str);
}

uint8_t *cbor_write_bytes(uint8_t *data, size_t size, const uint8_t *bytes, size_t bytes_size)
{
    return cbor_internal_write_bytes(data, size, 2, bytes_size, bytes);
}

uint8_t *cbor_write_array(uint8_t *data, size_t size, cbor_base_uint_t array_size)
{
    return cbor_internal_write_int_value(data, size, 4, 0, array_size);
}

uint8_t *cbor_write_map(uint8_t *data, size_t size, cbor_base_uint_t map_size)
{
    return cbor_internal_write_int_value(data, size, 5, 0, map_size);
}

uint8_t *cbor_write_tag(uint8_t *data, size_t size, cbor_base_uint_t tag)
{
    return cbor_internal_write_int_value(data, size, 6, 0, tag);
}

uint8_t *cbor_write_special(uint8_t *data, size_t size, uint8_t special)
{
    return cbor_internal_write_int_value(data, size, 7, 0, special);
}

cbor_bool_t cbor_read_uint(const uint8_t **data, size_t data_size, cbor_base_uint_t *value)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_INT, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = token.int_value;
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_int(const uint8_t **data, size_t data_size, cbor_base_int_t *value)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_INT, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    if (token.sign < 0)
        *value = -((cbor_base_int_t)token.int_value);
    else
        *value = token.int_value;

    return CBOR_TRUE;
}

cbor_bool_t cbor_read_float(const uint8_t **data, size_t data_size, float *value)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_FLOAT, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = (float)token.float_value;
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_double(const uint8_t **data, size_t data_size, double *value)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_FLOAT, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = token.float_value;
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_boolean(const uint8_t **data, size_t data_size, cbor_bool_t *value)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_BOOLEAN, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = (cbor_bool_t)token.int_value;
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_null(const uint8_t **data, size_t data_size)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_NULL, &token) == CBOR_FALSE)
        return CBOR_FALSE;
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_undefined(const uint8_t **data, size_t data_size)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_UNDEFINED, &token) == CBOR_FALSE)
        return CBOR_FALSE;
    return CBOR_TRUE;
}

cbor_bool_t cbor_get_string_length(const uint8_t *data, size_t data_size, size_t *string_length)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(&data, data_size, CBOR_TOKEN_TYPE_STRING, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    *string_length = (size_t)token.int_value;
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_string(const uint8_t **data, size_t data_size, char *buf, size_t buf_size)
{
    cbor_token_t token;
    size_t string_length;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_STRING, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    string_length = (size_t)token.int_value;
    if (buf_size <= string_length) /* including null-terminating char */
        return CBOR_FALSE;

    memcpy(buf, token.bytes_value, string_length);
    buf[string_length] = 0;
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_raw_string(const uint8_t **data, size_t data_size, char *buf, size_t buf_size)
{
    cbor_token_t token;
    size_t string_length;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_STRING, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    string_length = (size_t)token.int_value;
    if (buf_size < string_length) /* without null-terminating char */
        return CBOR_FALSE;

    memcpy(buf, token.bytes_value, string_length);
    return CBOR_TRUE;
}

cbor_bool_t cbor_get_bytes_size(const uint8_t *data, size_t data_size, size_t *bytes_size)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(&data, data_size, CBOR_TOKEN_TYPE_BYTES, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    *bytes_size = (size_t)token.int_value;
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_bytes(const uint8_t **data, size_t data_size, uint8_t *buf, size_t buf_size)
{
    cbor_token_t token;
    size_t bytes_size;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_BYTES, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    bytes_size = (size_t)token.int_value;
    if (buf_size < bytes_size)
        return CBOR_FALSE;

    memcpy(buf, token.bytes_value, bytes_size);
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_array(const uint8_t **data, size_t data_size, cbor_base_uint_t *array_size)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_ARRAY, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    *array_size = token.int_value;
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_map(const uint8_t **data, size_t data_size, cbor_base_uint_t *map_size)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_MAP, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    *map_size = token.int_value;
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_tag(const uint8_t **data, size_t data_size, cbor_base_uint_t *value)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_TAG, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = token.int_value;
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_special(const uint8_t **data, size_t data_size, uint8_t *value)
{
    cbor_token_t token;
    if (cbor_internal_read_and_check_type(data, data_size, CBOR_TOKEN_TYPE_INT, &token) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = (uint8_t)token.int_value;
    return CBOR_TRUE;
}
