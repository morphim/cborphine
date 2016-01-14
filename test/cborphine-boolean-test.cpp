#include "cborphine-boolean-test.h"

TEST_F(CborphineBooleanTest, WriteTrue)
{
    setExpected("f5");
    ASSERT_EQ(CBOR_TRUE, cbor_write_boolean(&_data, _size, CBOR_TRUE));
}

TEST_F(CborphineBooleanTest, WriteFalse)
{
    setExpected("f4");
    ASSERT_EQ(CBOR_TRUE, cbor_write_boolean(&_data, _size, CBOR_FALSE));
}