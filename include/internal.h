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

#ifndef INTERNAL_H
#define INTERNAL_H

#ifdef _MSC_VER
#define CBOR_INLINE static __inline
#else
#define CBOR_INLINE static inline
#endif

CBOR_INLINE uint8_t *cbor_internal_swap_2bytes(uint8_t *dest, const uint8_t *src)
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

CBOR_INLINE uint8_t *cbor_internal_swap_4bytes(uint8_t *dest, const uint8_t *src)
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

CBOR_INLINE uint8_t *cbor_internal_swap_8bytes(uint8_t *dest, const uint8_t *src)
{
#ifdef CBOR_BIGENDIAN_PLATFORM
    memcpy(dest, src, 8);
    return dest + 8;
#else
    return cbor_internal_swap_4bytes(cbor_internal_swap_4bytes(dest, src + 4), src);
#endif
}

#endif
