#include "cborphine-uint-test.h"

TEST_F(CborphineUIntTest, WriteWithZeroBufferSize)
{
    ASSERT_EQ(CBOR_FALSE, cbor_write_uint(&_data, 0, 1));
}

TEST_F(CborphineUIntTest, Write_0)
{
    _buffer[0] = 0xff;
    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 0));
}

TEST_F(CborphineUIntTest, Write_1)
{
    setExpected("01");
    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 1));
}

TEST_F(CborphineUIntTest, Write_10)
{
    setExpected("0a");
    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 10));
}

TEST_F(CborphineUIntTest, Write_23)
{
    setExpected("17");
    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 23));
}

TEST_F(CborphineUIntTest, Write_24)
{
    setExpected("18 18");
    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 24));
}

TEST_F(CborphineUIntTest, Write_25)
{
    setExpected("18 19");
    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 25));
}

TEST_F(CborphineUIntTest, Write_100)
{
    setExpected("18 64");
    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 100));
}

TEST_F(CborphineUIntTest, Write_1000)
{
    setExpected("19 03 e8");
    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 1000));
}

TEST_F(CborphineUIntTest, Write_1000000)
{
    setExpected("1a 00 0f 42 40");
    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 1000000));
}

TEST_F(CborphineUIntTest, Write_1000000000000)
{
    setExpected("1b 00 00 00 e8 d4 a5 10 00");
    _buffer[8] = 0xff;

    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 1000000000000));
}

TEST_F(CborphineUIntTest, Write_18446744073709551615)
{
    setExpected("1b ff ff ff ff ff ff ff ff");
    ASSERT_EQ(CBOR_TRUE, cbor_write_uint(&_data, _size, 18446744073709551615ULL));
}
