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

int main(int argc, char **argv)
{
    uint8_t buffer[4096];
    size_t size = sizeof(buffer);
    uint8_t *data = buffer;
    const uint8_t *ptr = buffer;
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

    for (ptr = buffer; ptr < data; ++ptr)
        printf("%02X", *ptr);
    printf("\n");

    ptr = buffer;
    while (1)
    {
        if (cbor_read_token(&ptr, data, &token) == CBOR_FALSE)
        {
            printf("ERROR: %s\n", token.error_value);
            break;
        }

        if (token.type == CBOR_TOKEN_TYPE_END)
            break;

        switch (token.type)
        {
        case CBOR_TOKEN_TYPE_INT:
            printf("int %s%llu\n", token.sign < 0 ? "-" : "", token.int_value);
            break;
        case CBOR_TOKEN_TYPE_FLOAT:
            printf("float %f\n", token.float_value);
            break;
        case CBOR_TOKEN_TYPE_ARRAY:
            printf("array length %llu\n", token.int_value);
            break;
        case CBOR_TOKEN_TYPE_MAP:
            printf("map length %llu\n", token.int_value);
            break;
        case CBOR_TOKEN_TYPE_TAG:
            printf("tag %llu\n", token.int_value);
            break;
        case CBOR_TOKEN_TYPE_SPECIAL:
            printf("special %llu\n", token.int_value);
            break;
        case CBOR_TOKEN_TYPE_BOOLEAN:
            printf("boolean %s\n", token.int_value > 0 ? "true" : "false");
            break;
        case CBOR_TOKEN_TYPE_NULL:
            printf("null\n");
            break;
        case CBOR_TOKEN_TYPE_UNDEFINED:
            printf("undefined\n");
            break;
        case CBOR_TOKEN_TYPE_STRING:
            printf("string '%.*s'\n", (int)token.int_value, (const char *)token.bytes_value);
            break;
        case CBOR_TOKEN_TYPE_BYTES:
            printf("bytes size %llu\n", token.int_value);
            break;
        }
    }

    return 0;
}
