#include "cborphine-test.h"

TEST_F(CborphineTest, WriteDouble)
{
    ASSERT_EQ(CBOR_TRUE, cbor_write_double(&_data, _size, 2.4028234663852886e+38));
}
