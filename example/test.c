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

#include <stdio.h>
#include <assert.h>
#include "cbor.h"

struct struct_type
{
    cbor_base_uint_t uint_value;
    cbor_base_int_t int_value;
    cbor_bool_t bool_value;
    uint8_t buf[100];
    double double_value;
};

void print_data(const uint8_t *data, size_t size)
{
    const uint8_t *ptr;

    for (ptr = data; ptr < data + size; ++ptr)
        printf("%02X", *ptr);
    printf("\n");
}

int enumerated_read()
{
    uint8_t buffer[4096];
    size_t size = sizeof(buffer);
    uint8_t *data = buffer;
    cbor_bool_t has_data;
    cbor_token_t token;

    assert(cbor_write_float(&data, size - (data - buffer), 3.4028234663852886e+38));
    assert(cbor_write_int(&data, size - (data - buffer), 500));
    assert(cbor_write_boolean(&data, size - (data - buffer), CBOR_TRUE));
    assert(cbor_write_boolean(&data, size - (data - buffer), CBOR_FALSE));
    assert(cbor_write_string(&data, size - (data - buffer), "hello"));
    assert(cbor_write_double(&data, size - (data - buffer), 2.4028234663852886e+38));
    assert(cbor_write_int(&data, size - (data - buffer), -12345));
    assert(cbor_write_array(&data, size - (data - buffer), 5));
    assert(cbor_write_int(&data, size - (data - buffer), 123));
    assert(cbor_write_string(&data, size - (data - buffer), "world"));
    assert(cbor_write_int(&data, size - (data - buffer), 2147483647));
    assert(cbor_write_string(&data, size - (data - buffer), "!"));
    assert(cbor_write_null(&data, size - (data - buffer)));
    assert(cbor_write_undefined(&data, size - (data - buffer)));

    print_data(buffer, data - buffer);

    has_data = cbor_init_read(&token, buffer, data - buffer, CBOR_FALSE); /* use explicit jumps to next item */
    while (has_data)
    {
        switch (token.type)
        {
        case CBOR_TOKEN_TYPE_PINT:
            printf("int %llu\n", CBOR_GET_PINT(&token));
            break;
        case CBOR_TOKEN_TYPE_NINT:
            printf("int %lli\n", CBOR_GET_NINT(&token));
            break;
        case CBOR_TOKEN_TYPE_FLOAT:
            printf("float %f\n", CBOR_GET_FLOAT(&token));
            break;
        case CBOR_TOKEN_TYPE_ARRAY:
            printf("array length %llu\n", CBOR_GET_ARRAY(&token));
            break;
        case CBOR_TOKEN_TYPE_MAP:
            printf("map length %llu\n", CBOR_GET_MAP(&token));
            break;
        case CBOR_TOKEN_TYPE_TAG:
            printf("tag %llu\n", CBOR_GET_TAG(&token));
            break;
        case CBOR_TOKEN_TYPE_SPECIAL:
            printf("special %u\n", CBOR_GET_SPECIAL(&token));
            break;
        case CBOR_TOKEN_TYPE_BOOLEAN:
            printf("boolean %s\n", CBOR_GET_BOOLEAN(&token) ? "true" : "false");
            break;
        case CBOR_TOKEN_TYPE_NULL:
            printf("null\n");
            break;
        case CBOR_TOKEN_TYPE_UNDEFINED:
            printf("undefined\n");
            break;
        case CBOR_TOKEN_TYPE_STRING:
            printf("string '%.*s'\n", (int)CBOR_GET_STRING_LENGTH(&token), CBOR_GET_STRING(&token));
            break;
        case CBOR_TOKEN_TYPE_BYTES:
            printf("bytes size %llu\n", CBOR_GET_BYTES_SIZE(&token));
            break;
        }
        has_data = cbor_read_next(&token);
    }

    if (token.type == CBOR_TOKEN_TYPE_ERROR)
    {
        printf("ERROR: %s\n", token.error_message);
        return 1;
    }

    return 0;
}

int declarative_read()
{
    uint8_t buffer[4096];
    struct struct_type src_struct;
    struct struct_type dst_struct;
    size_t size = sizeof(buffer);
    uint8_t *data = buffer;
    cbor_token_t token;

    src_struct.uint_value = 10000;
    src_struct.int_value = -23456;
    src_struct.double_value = 1234.5678;
    src_struct.bool_value = CBOR_TRUE;
    memset(src_struct.buf, 0xFF, sizeof(src_struct.buf));

    assert(cbor_write_uint(&data, size - (data - buffer), src_struct.uint_value));
    assert(cbor_write_int(&data, size - (data - buffer), src_struct.int_value));
    assert(cbor_write_double(&data, size - (data - buffer), src_struct.double_value));
    assert(cbor_write_boolean(&data, size - (data - buffer), src_struct.bool_value));
    assert(cbor_write_bytes(&data, size - (data - buffer), src_struct.buf, sizeof(src_struct.buf)));

    memset(&dst_struct, 0, sizeof(dst_struct));

    if (cbor_init_read(&token, buffer, data - buffer, CBOR_TRUE) == CBOR_FALSE) /* use implicit jumps to next item */
        return 1;

    /* we can ignore return values here as next fields won't be read on error */
    cbor_read_uint(&token, &dst_struct.uint_value);
    /* cbor_read_double(&token, &dst_struct.double_value); */
    cbor_read_int(&token, &dst_struct.int_value);
    cbor_read_double(&token, &dst_struct.double_value);
    cbor_read_boolean(&token, &dst_struct.bool_value);
    cbor_read_bytes(&token, dst_struct.buf, sizeof(dst_struct.buf));

    if (token.type == CBOR_TOKEN_TYPE_ERROR)
    {
        printf("ERROR: %s\n", token.error_message);
        return 1;
    }

    assert(src_struct.uint_value == dst_struct.uint_value);
    assert(src_struct.int_value == dst_struct.int_value);
    assert(src_struct.double_value == dst_struct.double_value);
    assert(src_struct.bool_value == dst_struct.bool_value);
    assert(memcmp(src_struct.buf, dst_struct.buf, sizeof(dst_struct.buf)) == 0);

    return 0;
}

int main(int argc, char **argv)
{
    if (enumerated_read() != 0)
        return 1;
    if (declarative_read() != 0)
        return 1;

    return 0;
}
