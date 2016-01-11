#include "cborphine-test.h"

TEST_F(CborphineTest, WriteBooleanTrue)
{
    ASSERT_EQ(CBOR_TRUE, cbor_write_boolean(&_data, _size, CBOR_TRUE));
}

TEST_F(CborphineTest, WriteBooleanFalse)
{
    ASSERT_EQ(CBOR_TRUE, cbor_write_boolean(&_data, _size, CBOR_FALSE));
}