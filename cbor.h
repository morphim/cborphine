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

typedef enum
{
    CBOR_TOKEN_TYPE_END,
    CBOR_TOKEN_TYPE_INT,
    CBOR_TOKEN_TYPE_ARRAY,
    CBOR_TOKEN_TYPE_STRING,
    CBOR_TOKEN_TYPE_BYTES,
    CBOR_TOKEN_TYPE_MAP,
    CBOR_TOKEN_TYPE_TAG,
    CBOR_TOKEN_TYPE_SPECIAL,
    CBOR_TOKEN_TYPE_BOOLEAN,
    CBOR_TOKEN_TYPE_NULL,
    CBOR_TOKEN_TYPE_UNDEFINED,

    CBOR_TOKEN_TYPE_ERROR = 100
} cbor_token_type_t;

#ifdef CBOR_LONGLONG_SUPPORT
typedef long long          cbor_base_int_t;
typedef unsigned long long cbor_base_uint_t;
#else
typedef int          cbor_base_int_t;
typedef unsigned int cbor_base_uint_t;
#endif

typedef unsigned int cbor_bool_t;
#define CBOR_TRUE 1
#define CBOR_FALSE 0

typedef struct
{
    cbor_token_type_t type;
    cbor_base_uint_t int_value;
    int sign;
    union
    {
        char *string_value;
        unsigned char *bytes_value;
        const char *error_value;
    };
} cbor_token_t;

unsigned char *cbor_read_token(unsigned char *data, unsigned char *end, cbor_token_t *token);

unsigned char *cbor_write_pint(unsigned char *data, size_t size, cbor_base_uint_t value);
unsigned char *cbor_write_nint(unsigned char *data, size_t size, cbor_base_uint_t value);
unsigned char *cbor_write_uint(unsigned char *data, size_t size, cbor_base_uint_t value);
unsigned char *cbor_write_int(unsigned char *data, size_t size, cbor_base_int_t value);

unsigned char *cbor_write_boolean(unsigned char *data, size_t size, cbor_bool_t value);
unsigned char *cbor_write_null(unsigned char *data, size_t size);
unsigned char *cbor_write_undefined(unsigned char *data, size_t size);

unsigned char *cbor_write_string(unsigned char *data, size_t size, const char *str);
unsigned char *cbor_write_string_with_len(unsigned char *data, size_t size, const char *str, size_t str_length);
unsigned char *cbor_write_bytes(unsigned char *data, size_t size, const unsigned char *bytes, size_t bytes_size);

unsigned char *cbor_write_array(unsigned char *data, size_t size, cbor_base_uint_t array_size);
unsigned char *cbor_write_map(unsigned char *data, size_t size, cbor_base_uint_t map_size);
unsigned char *cbor_write_tag(unsigned char *data, size_t size, cbor_base_uint_t tag);
unsigned char *cbor_write_special(unsigned char *data, size_t size, cbor_base_uint_t special);

#endif
