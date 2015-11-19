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
#include "cbor.h"

int main(int argc, char **argv)
{
    uint8_t buffer[4096];
    size_t size = sizeof(buffer);
    uint8_t *data = buffer;
    const uint8_t *ptr = buffer;
    cbor_token_t token;

    data = cbor_write_float(data, size, 3.4028234663852886e+38);
    data = cbor_write_int(data, size, 500);
    data = cbor_write_boolean(data, size, CBOR_TRUE);
    data = cbor_write_boolean(data, size, CBOR_FALSE);
    data = cbor_write_string(data, size, "hello");
    data = cbor_write_int(data, size, -12345);
    data = cbor_write_array(data, size, 5);
    data = cbor_write_int(data, size, 123);
    data = cbor_write_string(data, size, "world");
    data = cbor_write_int(data, size, 2147483647);
    data = cbor_write_string(data, size, "!");
    data = cbor_write_null(data, size);
    data = cbor_write_undefined(data, size);

    for (ptr = buffer; ptr < data; ++ptr)
        printf("%02X", *ptr);
    printf("\n");

    ptr = buffer;
    while (1)
    {
        ptr = cbor_read_token(ptr, data, &token);

        if (token.type == CBOR_TOKEN_TYPE_END)
            break;

        if (token.type == CBOR_TOKEN_TYPE_ERROR)
        {
            printf("ERROR: %s\n", token.error_value);
            break;
        }

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
            printf("special %u\n", token.int_value);
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
