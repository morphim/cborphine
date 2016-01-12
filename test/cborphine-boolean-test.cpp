#include "cborphine-test.h"

TEST_F(CborphineTest, WriteBooleanTrue)
{
    _expected[0] = 0xf5;

    ASSERT_EQ(CBOR_TRUE, cbor_write_boolean(&_data, _size, CBOR_TRUE));
    ASSERT_EQ(_expected, _buffer);
}

TEST_F(CborphineTest, WriteBooleanFalse)
{
    _expected[0] = 0xf4;

    ASSERT_EQ(CBOR_TRUE, cbor_write_boolean(&_data, _size, CBOR_FALSE));
    ASSERT_EQ(_expected, _buffer);
}