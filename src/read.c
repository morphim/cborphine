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

#define CBOR_GET_MAJOR_TYPE(initial_byte) ((initial_byte) >> 5)
#define CBOR_GET_MINOR_TYPE(initial_byte) ((initial_byte) & 31)

static cbor_token_type_t cbor_internal_types_map[] =
{
    CBOR_TOKEN_TYPE_PINT,   /* 0 */
    CBOR_TOKEN_TYPE_NINT,   /* 1 */
    CBOR_TOKEN_TYPE_BYTES,  /* 2 */
    CBOR_TOKEN_TYPE_STRING, /* 3 */
    CBOR_TOKEN_TYPE_ARRAY,  /* 4 */
    CBOR_TOKEN_TYPE_MAP,    /* 5 */
    CBOR_TOKEN_TYPE_TAG,    /* 6 */
    CBOR_TOKEN_TYPE_SPECIAL /* 7 */
};

CBOR_INLINE int cbor_internal_get_width(unsigned int minor_type)
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

CBOR_INLINE cbor_bool_t cbor_internal_read_int_value(unsigned int minor_type, cbor_token_data_t *token)
{
    int type_width = cbor_internal_get_width(minor_type);

    if (type_width < 0)
    {
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_message = "invalid type width";
        return CBOR_FALSE;
    }

    if ((size_t)(token->end - token->pos) < (size_t)type_width)
    {
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_message = "insufficient data";
        return CBOR_FALSE;
    }

    switch (type_width)
    {
    case 0:
        token->int_value = minor_type;
        return CBOR_TRUE;
    case 1:
        token->int_value = *token->pos;
        token->pos += 1; /* bytes processed */
        return CBOR_TRUE;
    case 2:
        {
            uint16_t short_value;
            cbor_internal_swap_2bytes((uint8_t *)&short_value, token->pos);
            token->int_value = short_value;
            token->pos += 2; /* bytes processed */
            return CBOR_TRUE;
        }
    case 4:
        {
            uint32_t int_value;
            cbor_internal_swap_4bytes((uint8_t *)&int_value, token->pos);
            token->int_value = int_value;
            token->pos += 4; /* bytes processed */
            return CBOR_TRUE;
        }
    case 8:
#ifdef CBOR_INT64_SUPPORT
        {
            cbor_base_uint_t value;
            cbor_internal_swap_8bytes((uint8_t *)&value, token->pos);
            token->int_value = value;
            token->pos += 8; /* bytes processed */
            return CBOR_TRUE;
        }
#else
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_message = "64 bits integers are not supported";
        return CBOR_FALSE;
#endif
    }

    token->type = CBOR_TOKEN_TYPE_ERROR;
    token->error_message = "unknown error";
    return CBOR_FALSE;
}

CBOR_INLINE cbor_bool_t cbor_internal_extract_special_value(unsigned int minor_type, cbor_token_data_t *token)
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
        if ((size_t)(token->end - token->pos) < 4)
        {
            token->type = CBOR_TOKEN_TYPE_ERROR;
            token->error_message = "insufficient data";
            return CBOR_FALSE;
        }
        {
            float float_value;
            cbor_internal_swap_4bytes((uint8_t *)&float_value, token->pos);

            token->type = CBOR_TOKEN_TYPE_FLOAT;
            token->float_value = float_value;
            token->pos += 4; /* bytes processed */
            return CBOR_TRUE;
        }
    case 27: /* double-precision float */
        if ((size_t)(token->end - token->pos) < 8)
        {
            token->type = CBOR_TOKEN_TYPE_ERROR;
            token->error_message = "insufficient data";
            return CBOR_FALSE;
        }
        {
            double double_value;
            cbor_internal_swap_8bytes((uint8_t *)&double_value, token->pos);

            token->type = CBOR_TOKEN_TYPE_FLOAT;
            token->float_value = double_value;
            token->pos += 8; /* bytes processed */
            return CBOR_TRUE;
        }
    default:
        {
            if (cbor_internal_read_int_value(minor_type, token) == CBOR_FALSE)
                return CBOR_FALSE;

            token->type = CBOR_TOKEN_TYPE_SPECIAL;
            return CBOR_TRUE;
        }
    }
}

CBOR_INLINE cbor_bool_t cbor_internal_check_type(cbor_token_data_t *token, cbor_token_type_t expected_type)
{
    if (token->type != expected_type)
    {
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_message = "invalid data type";
        return CBOR_FALSE;
    }

    return CBOR_TRUE;
}

CBOR_INLINE cbor_bool_t cbor_internal_read_next(cbor_token_data_t *token)
{
    unsigned int major_type;
    unsigned int minor_type;
    const uint8_t *current_pos = token->pos;

    if (token->type == CBOR_TOKEN_TYPE_ERROR)
        return CBOR_FALSE; /* state is not changed */

    if (current_pos >= token->end)
    {
        token->type = CBOR_TOKEN_TYPE_END;
        return CBOR_FALSE; /* nothing to read */
    }

    major_type = CBOR_GET_MAJOR_TYPE(*current_pos);
    minor_type = CBOR_GET_MINOR_TYPE(*current_pos);
    token->pos += 1; /* initial byte is processed */

    switch (major_type)
    {
    case 0: /* positive integer */
        if (cbor_internal_read_int_value(minor_type, token))
        {
            token->type = CBOR_TOKEN_TYPE_PINT;
            return CBOR_TRUE;
        }
        break;
    case 1: /* negative integer */
        if (cbor_internal_read_int_value(minor_type, token))
        {
            token->type = CBOR_TOKEN_TYPE_NINT;
            return CBOR_TRUE;
        }
        break;
    case 2: /* bytes */
    case 3: /* string */
        if (cbor_internal_read_int_value(minor_type, token))
        {
            if ((size_t)(token->end - token->pos) < token->int_value)
            {
                token->type = CBOR_TOKEN_TYPE_ERROR;
                token->error_message = "insufficient data";
            }
            else
            {
                token->type = cbor_internal_types_map[major_type];
                token->bytes_value = token->pos; /* set data pointer */
                token->pos += token->int_value; /* skip bytes */
                return CBOR_TRUE;
            }
        }
        break;
    case 4: /* array */
    case 5: /* map */
    case 6: /* tag */
        if (cbor_internal_read_int_value(minor_type, token))
        {
            token->type = cbor_internal_types_map[major_type];
            return CBOR_TRUE;
        }
        break;
    case 7: /* special */
        if (cbor_internal_extract_special_value(minor_type, token))
            return CBOR_TRUE;
        break;
    default:
        token->type = CBOR_TOKEN_TYPE_ERROR;
        token->error_message = "unknown error";
        break;
    }

    token->pos = current_pos; /* restore original position */
    return CBOR_FALSE;
}

CBOR_INLINE void cbor_internal_try_to_read_next(cbor_token_data_t *token)
{
    if (token->next_on_read)
        cbor_internal_read_next(token);
}

cbor_bool_t cbor_init_read(cbor_token_t *token, const uint8_t *data, size_t data_size, cbor_bool_t next_on_read)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    token_data->type = CBOR_TOKEN_TYPE_END;
    token_data->pos = data;
    token_data->end = data + data_size;
    token_data->next_on_read = next_on_read;

    return cbor_internal_read_next(token_data);
}

cbor_bool_t cbor_read_next(cbor_token_t *token)
{
    return cbor_internal_read_next((cbor_token_data_t *)token);
}

cbor_bool_t cbor_read_uint(cbor_token_t *token, cbor_base_uint_t *value)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_PINT) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = CBOR_GET_PINT(token_data);

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_int(cbor_token_t *token, cbor_base_int_t *value)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    switch (token_data->type)
    {
    case CBOR_TOKEN_TYPE_PINT:
        *value = (cbor_base_int_t)CBOR_GET_PINT(token_data);
        break;
    case CBOR_TOKEN_TYPE_NINT:
        *value = CBOR_GET_NINT(token_data);
        break;
    default:
        token_data->type = CBOR_TOKEN_TYPE_ERROR;
        token_data->error_message = "invalid data type";
        return CBOR_FALSE;
    }

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_float(cbor_token_t *token, float *value)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_FLOAT) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = (float)CBOR_GET_FLOAT(token_data);

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_double(cbor_token_t *token, double *value)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_FLOAT) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = CBOR_GET_FLOAT(token_data);

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_boolean(cbor_token_t *token, cbor_bool_t *value)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_BOOLEAN) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = CBOR_GET_BOOLEAN(token_data);

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}

cbor_bool_t cbor_get_string_length(cbor_token_t *token, size_t *string_length)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_STRING) == CBOR_FALSE)
        return CBOR_FALSE;

    *string_length = (size_t)CBOR_GET_STRING_LENGTH(token_data);

    /* don't read next */
    return CBOR_TRUE;
}

cbor_bool_t cbor_get_bytes_size(cbor_token_t *token, size_t *bytes_size)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_BYTES) == CBOR_FALSE)
        return CBOR_FALSE;

    *bytes_size = (size_t)CBOR_GET_BYTES_SIZE(token_data);

    /* don't read next */
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_string(cbor_token_t *token, char *buf, size_t buf_size)
{
    size_t string_length;
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_STRING) == CBOR_FALSE)
        return CBOR_FALSE;

    string_length = (size_t)CBOR_GET_STRING_LENGTH(token_data);
    if (buf_size <= string_length) /* including null-terminating char */
        return CBOR_FALSE;

    memcpy(buf, token_data->bytes_value, string_length);
    buf[string_length] = 0;

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_raw_string(cbor_token_t *token, char *buf, size_t buf_size)
{
    size_t string_length;
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_STRING) == CBOR_FALSE)
        return CBOR_FALSE;

    string_length = (size_t)CBOR_GET_STRING_LENGTH(token_data);
    if (buf_size < string_length) /* without null-terminating char */
        return CBOR_FALSE;

    memcpy(buf, token_data->bytes_value, string_length);

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_bytes(cbor_token_t *token, uint8_t *buf, size_t buf_size)
{
    size_t bytes_size;
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_BYTES) == CBOR_FALSE)
        return CBOR_FALSE;

    bytes_size = (size_t)CBOR_GET_BYTES_SIZE(token_data);
    if (buf_size < bytes_size)
        return CBOR_FALSE;

    memcpy(buf, token_data->bytes_value, bytes_size);

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_array(cbor_token_t *token, cbor_base_uint_t *array_size)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_ARRAY) == CBOR_FALSE)
        return CBOR_FALSE;

    *array_size = CBOR_GET_ARRAY(token_data);

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_map(cbor_token_t *token, cbor_base_uint_t *map_size)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_MAP) == CBOR_FALSE)
        return CBOR_FALSE;

    *map_size = CBOR_GET_MAP(token_data);

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_tag(cbor_token_t *token, cbor_base_uint_t *value)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_TAG) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = CBOR_GET_TAG(token_data);

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}

cbor_bool_t cbor_read_special(cbor_token_t *token, uint8_t *value)
{
    cbor_token_data_t *token_data = (cbor_token_data_t *)token;

    if (cbor_internal_check_type(token_data, CBOR_TOKEN_TYPE_SPECIAL) == CBOR_FALSE)
        return CBOR_FALSE;

    *value = CBOR_GET_SPECIAL(token_data);

    cbor_internal_try_to_read_next(token_data);
    return CBOR_TRUE;
}
