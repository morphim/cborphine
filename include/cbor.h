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

#ifndef CBOR_H
#define CBOR_H

#ifdef CBOR_USE_STANDARD_STDINT
#include <stdint.h>
#else
#include "pstdint.h"
#endif

typedef enum
{
    CBOR_TOKEN_TYPE_END,
    CBOR_TOKEN_TYPE_PINT, /* positive integer */
    CBOR_TOKEN_TYPE_NINT, /* negative integer */
    CBOR_TOKEN_TYPE_ARRAY,
    CBOR_TOKEN_TYPE_STRING,
    CBOR_TOKEN_TYPE_BYTES,
    CBOR_TOKEN_TYPE_MAP,
    CBOR_TOKEN_TYPE_TAG,
    CBOR_TOKEN_TYPE_SPECIAL,
    CBOR_TOKEN_TYPE_BOOLEAN,
    CBOR_TOKEN_TYPE_NULL,
    CBOR_TOKEN_TYPE_UNDEFINED,
    CBOR_TOKEN_TYPE_FLOAT,

    CBOR_TOKEN_TYPE_ERROR = 100
} cbor_token_type_t;

#ifdef CBOR_INT64_SUPPORT
typedef int64_t  cbor_base_int_t;
typedef uint64_t cbor_base_uint_t;
#else
typedef int32_t  cbor_base_int_t;
typedef uint32_t cbor_base_uint_t;
#endif

typedef unsigned int cbor_bool_t;
#define CBOR_TRUE 1
#define CBOR_FALSE 0

typedef struct
{
    cbor_token_type_t type;
    const char *error_message;
    /* read options */
    const uint8_t *pos;
    const uint8_t *end;
    cbor_bool_t next_on_read;
    /* data values */
    cbor_base_uint_t int_value; /* used as a simple value or length of data */
    double float_value;         /* used with CBOR_TOKEN_TYPE_FLOAT type only */
    const uint8_t *bytes_value; /* used with CBOR_TOKEN_TYPE_BYTES and CBOR_TOKEN_TYPE_STRING types */
} cbor_token_data_t;

#define CBOR_TOKEN_INFO_SIZE \
    (sizeof(cbor_token_data_t) - sizeof(cbor_token_type_t) - sizeof (const char *))

typedef struct
{
    cbor_token_type_t type;
    const char *error_message;
    uint8_t x_files[CBOR_TOKEN_INFO_SIZE]; /* options and data are hidden */
} cbor_token_t;

#define CBOR_GET_PINT(token) (cbor_base_uint_t)(((cbor_token_data_t *)(token))->int_value)
#define CBOR_GET_NINT(token) (cbor_base_int_t)(-((cbor_base_int_t)(((cbor_token_data_t *)(token))->int_value)) - 1)
#define CBOR_GET_ARRAY(token) (cbor_base_uint_t)(((cbor_token_data_t *)(token))->int_value)
#define CBOR_GET_STRING_LENGTH(token) (cbor_base_uint_t)(((cbor_token_data_t *)(token))->int_value)
#define CBOR_GET_BYTES_SIZE(token) (cbor_base_uint_t)(((cbor_token_data_t *)(token))->int_value)
#define CBOR_GET_STRING(token) (const char *)(((cbor_token_data_t *)(token))->bytes_value)
#define CBOR_GET_BYTES(token) (const uint8_t *)(((cbor_token_data_t *)(token))->bytes_value)
#define CBOR_GET_MAP(token) (cbor_base_uint_t)(((cbor_token_data_t *)(token))->int_value)
#define CBOR_GET_TAG(token) (cbor_base_uint_t)(((cbor_token_data_t *)(token))->int_value)
#define CBOR_GET_SPECIAL(token) (uint8_t)(((cbor_token_data_t *)(token))->int_value)
#define CBOR_GET_BOOLEAN(token) (cbor_bool_t)(((cbor_token_data_t *)(token))->int_value)
#define CBOR_GET_FLOAT(token) (double)(((cbor_token_data_t *)(token))->float_value)

#ifdef __cplusplus
extern "C"
{
#endif

/* write data */

cbor_bool_t cbor_write_uint(uint8_t **data, size_t size, cbor_base_uint_t value);
cbor_bool_t cbor_write_int(uint8_t **data, size_t size, cbor_base_int_t value);

cbor_bool_t cbor_write_float(uint8_t **data, size_t size, float value);
cbor_bool_t cbor_write_double(uint8_t **data, size_t size, double value);

cbor_bool_t cbor_write_boolean(uint8_t **data, size_t size, cbor_bool_t value);
cbor_bool_t cbor_write_null(uint8_t **data, size_t size);
cbor_bool_t cbor_write_undefined(uint8_t **data, size_t size);

cbor_bool_t cbor_write_string_with_len(uint8_t **data, size_t size, const char *str, size_t str_length);
cbor_bool_t cbor_write_string(uint8_t **data, size_t size, const char *str);
cbor_bool_t cbor_write_bytes(uint8_t **data, size_t size, const uint8_t *bytes, size_t bytes_size);

cbor_bool_t cbor_write_array(uint8_t **data, size_t size, cbor_base_uint_t array_size);
cbor_bool_t cbor_write_map(uint8_t **data, size_t size, cbor_base_uint_t map_size);
cbor_bool_t cbor_write_tag(uint8_t **data, size_t size, cbor_base_uint_t tag);
cbor_bool_t cbor_write_special(uint8_t **data, size_t size, uint8_t special);

/* read data */

cbor_bool_t cbor_init_read(cbor_token_t *token, const uint8_t *data, size_t data_size, cbor_bool_t next_on_read);
cbor_bool_t cbor_read_next(cbor_token_t *token); /* used when we don't need to read anything */

cbor_bool_t cbor_read_uint(cbor_token_t *token, cbor_base_uint_t *value);
cbor_bool_t cbor_read_int(cbor_token_t *token, cbor_base_int_t *value);

cbor_bool_t cbor_read_float(cbor_token_t *token, float *value);
cbor_bool_t cbor_read_double(cbor_token_t *token, double *value);

cbor_bool_t cbor_read_boolean(cbor_token_t *token, cbor_bool_t *value);

cbor_bool_t cbor_get_string_length(cbor_token_t *token, size_t *string_length);
cbor_bool_t cbor_get_bytes_size(cbor_token_t *token, size_t *bytes_size);

cbor_bool_t cbor_read_string(cbor_token_t *token, char *buf, size_t buf_size);
cbor_bool_t cbor_read_raw_string(cbor_token_t *token, char *buf, size_t buf_size);
cbor_bool_t cbor_read_bytes(cbor_token_t *token, uint8_t *buf, size_t buf_size);

cbor_bool_t cbor_read_array(cbor_token_t *token, cbor_base_uint_t *array_size);
cbor_bool_t cbor_read_map(cbor_token_t *token, cbor_base_uint_t *map_size);
cbor_bool_t cbor_read_tag(cbor_token_t *token, cbor_base_uint_t *tag);
cbor_bool_t cbor_read_special(cbor_token_t *token, uint8_t *special);

#ifdef __cplusplus
}
#endif

#endif
