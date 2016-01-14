#include "cborphine-string-test.h"

TEST_F(CborphineStringTest, Write_a)
{
    setExpected("61 61");
    ASSERT_EQ(CBOR_TRUE, cbor_write_string(&_data, _size, "a"));
}
