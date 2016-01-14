#include "cborphine-float-test.h"

TEST_F(CborphineFloatTest, WriteWithZeroBufferSize)
{
    ASSERT_EQ(CBOR_FALSE, cbor_write_float(&_data, 0, 3.4028234663852886e+38));
}

TEST_F(CborphineFloatTest, DISABLED_Write_0_dot_0)
{
    setExpected("f9 00 00");
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 0.0));
}

TEST_F(CborphineFloatTest, DISABLED_Write_minus_0_dot_0)
{
    setExpected("f9 80 00");
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, -0.0));
}

TEST_F(CborphineFloatTest, DISABLED_Write_1_dot_0)
{
    setExpected("f9 3c 00");
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 1.0));
}

TEST_F(CborphineFloatTest, DISABLED_Write_1_dot_5)
{
    setExpected("f9 3e 00");
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 1.5));
}

TEST_F(CborphineFloatTest, DISABLED_Write_65504_dot_0)
{
    setExpected("f9 7b ff");
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 65504.0));
}

TEST_F(CborphineFloatTest, DISABLED_Write_5_dot_960464477539063e_minus_8)
{
    setExpected("f9 00 01");
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 5.960464477539063e-8));
}

TEST_F(CborphineFloatTest, DISABLED_Write_0_dot_00006103515625)
{
    setExpected("f9 04 00");
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 0.00006103515625));
}

TEST_F(CborphineFloatTest, DISABLED_Write_minus_4_dot_0)
{
    setExpected("f9 c4 00");
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, -4.0));
}

TEST_F(CborphineFloatTest, DISABLED_Write_minus_4_dot_1)
{
    setExpected("f9 c4 00");
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, -4.0));
}

TEST_F(CborphineFloatTest, Write_100000_dot_0)
{
    setExpected("fa 47 c3 50 00");
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 100000.0));
}

TEST_F(CborphineFloatTest, Write_3_dot_4028234663852886e_plus_38)
{
    setExpected("fa 7f 7f ff ff");
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 3.4028234663852886e+38));
}
