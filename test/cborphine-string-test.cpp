#include "cborphine-test.h"

TEST_F(CborphineTest, WriteString)
{
    ASSERT_EQ(CBOR_TRUE, cbor_write_string(&_data, _size, "hello"));
}
