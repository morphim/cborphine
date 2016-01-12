#include "cborphine-test.h"

TEST_F(CborphineTest, WriteFloatWithZeroBufferSize)
{
    ASSERT_EQ(CBOR_FALSE, cbor_write_float(&_data, 0, 3.4028234663852886e+38));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, DISABLED_WriteFloat_0_dot_0)
{
    _expected[0] = 0xf9;
    _expected[1] = 0x00;
    _expected[2] = 0x00;

    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 0.0));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, DISABLED_WriteFloat_minus_0_dot_0)
{
    _expected[0] = 0xf9;
    _expected[1] = 0x80;
    _expected[2] = 0x00;

    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, -0.0));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, DISABLED_WriteFloat_1_dot_0)
{
    _expected[0] = 0xf9;
    _expected[1] = 0x3c;
    _expected[2] = 0x00;

    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 1.0));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, DISABLED_WriteFloat_1_dot_5)
{
    _expected[0] = 0xf9;
    _expected[1] = 0x3e;
    _expected[2] = 0x00;

    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 1.5));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, DISABLED_WriteFloat_65504_dot_0)
{
    _expected[0] = 0xf9;
    _expected[1] = 0x7b;
    _expected[2] = 0xff;

    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 65504.0));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, DISABLED_WriteFloat_5_dot_960464477539063e_minus_8)
{
    _expected[0] = 0xf9;
    _expected[1] = 0x00;
    _expected[2] = 0x01;

    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 5.960464477539063e-8));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, DISABLED_WriteFloat_0_dot_00006103515625)
{
    _expected[0] = 0xf9;
    _expected[1] = 0x04;
    _expected[2] = 0x00;

    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 0.00006103515625));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, DISABLED_WriteFloat_minus_4_dot_0)
{
    _expected[0] = 0xf9;
    _expected[1] = 0xc4;
    _expected[2] = 0x00;

    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, -4.0));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, DISABLED_WriteFloat_minus_4_dot_1)
{
    _expected[0] = 0xf9;
    _expected[1] = 0xc4;
    _expected[2] = 0x00;

    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, -4.0));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteFloat_100000_dot_0)
{
    _expected[0] = 0xfa;
    _expected[1] = 0x47;
    _expected[2] = 0xc3;
    _expected[3] = 0x50;
    _expected[4] = 0x00;

    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 100000.0));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteFloat_3_dot_4028234663852886e_plus_38)
{
    _expected[0] = 0xfa;
    _expected[1] = 0x7f;
    _expected[2] = 0x7f;
    _expected[3] = 0xff;
    _expected[4] = 0xff;

    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 3.4028234663852886e+38));
    ASSERT_EQ(_expected, _buffer);
}