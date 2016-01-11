#include "cborphine-test.h"

TEST_F(CborphineTest, WritePositiveInteger)
{
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 500));
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, 123));
}

TEST_F(CborphineTest, WriteNegativeInteger)
{
    ASSERT_EQ(CBOR_TRUE, cbor_write_int(&_data, _size, -12345));
}