#include "cborphine-double-test.h"

TEST_F(CborphineDoubleTest, Write_1_dot_0e_plus_300)
{
    setExpected("fb 7e 37 e4 3c 88 00 75 9c");
    ASSERT_EQ(CBOR_TRUE, cbor_write_double(&_data, _size, 1.0e+300));
}
