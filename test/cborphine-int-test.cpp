#include "cborphine-int-test.h"

TEST_F(CborphineIntTest, WriteWithZeroBufferSize)
{
    ASSERT_EQ(CBOR_FALSE, cbor_write_int(&_data, 0, 1));
}

TEST_F(CborphineIntTest, Write_0)
{
    _buffer[0] = 0xff;
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 0));
}

TEST_F(CborphineIntTest, Write_1)
{
    setExpected("01");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 1));
}

TEST_F(CborphineIntTest, Write_10)
{
    setExpected("0a");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 10));
}

TEST_F(CborphineIntTest, Write_23)
{
    setExpected("17");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 23));
}

TEST_F(CborphineIntTest, Write_24)
{
    setExpected("18 18");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 24));
}

TEST_F(CborphineIntTest, Write_25)
{
    setExpected("18 19");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 25));
}

TEST_F(CborphineIntTest, Write_100)
{
    setExpected("18 64");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 100));
}

TEST_F(CborphineIntTest, Write_1000)
{
    setExpected("19 03 e8");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 1000));
}

TEST_F(CborphineIntTest, Write_1000000)
{
    setExpected("1a 00 0f 42 40");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 1000000));
}

TEST_F(CborphineIntTest, Write_1000000000000)
{
    setExpected("1b 00 00 00 e8 d4 a5 10 00");
    _buffer[8] = 0xff;
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 1000000000000LL));
}

TEST_F(CborphineIntTest, Write_minus_1)
{
    setExpected("20");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, -1));
}

TEST_F(CborphineIntTest, Write_minus_10)
{
    setExpected("29");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, -10));
}

TEST_F(CborphineIntTest, Write_minus_100)
{
    setExpected("38 63");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, -100));
}

TEST_F(CborphineIntTest, Write_minus_1000)
{
    setExpected("39 03 e7");
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, -1000));
}
