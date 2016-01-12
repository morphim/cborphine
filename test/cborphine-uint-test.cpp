#include "cborphine-test.h"

TEST_F(CborphineTest, WriteUnsignedIntegerWithZeroBufferSize)
{
    ASSERT_EQ(CBOR_FALSE, cbor_write_uint(&_data, 0, 1));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteUnsignedInteger_0)
{
    _buffer[0] = 0xff;
    _expected[0] = 0x00;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 0));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteUnsignedInteger_1)
{
    _expected[0] = 0x01;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 1));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteUnsignedInteger_10)
{
    _expected[0] = 0x0a;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 10));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteUnsignedInteger_23)
{
    _expected[0] = 0x17;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 23));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteUnsignedInteger_24)
{
    _expected[0] = 0x18;
    _expected[1] = 0x18;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 24));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteUnsignedInteger_25)
{
    _expected[0] = 0x18;
    _expected[1] = 0x19;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 25));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteUnsignedInteger_100)
{
    _expected[0] = 0x18;
    _expected[1] = 0x64;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 100));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteUnsignedInteger_1000)
{
    _expected[0] = 0x19;
    _expected[1] = 0x03;
    _expected[2] = 0xe8;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 1000));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteUnsignedInteger_1000000)
{
    _expected[0] = 0x1a;
    _expected[1] = 0x00;
    _expected[2] = 0x0f;
    _expected[3] = 0x42;
    _expected[4] = 0x40;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 1000000));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteUnsignedInteger_1000000000000)
{
    _expected[0] = 0x1b;
    _expected[1] = 0x00;
    _expected[2] = 0x00;
    _expected[3] = 0x00;
    _expected[4] = 0xe8;
    _expected[5] = 0xd4;
    _expected[6] = 0xa5;
    _expected[7] = 0x10;
    _expected[8] = 0x00;

    _buffer[8] = 0xff;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 1000000000000));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteUnsignedInteger_18446744073709551615)
{
    _expected[0] = 0x1b;
    _expected[1] = 0xff;
    _expected[2] = 0xff;
    _expected[3] = 0xff;
    _expected[4] = 0xff;
    _expected[5] = 0xff;
    _expected[6] = 0xff;
    _expected[7] = 0xff;
    _expected[8] = 0xff;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 18446744073709551615ULL));
    ASSERT_EQ(_expected, _buffer);
}
