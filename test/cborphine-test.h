#ifndef CBORPHINE_TEST_H
#define CBORPHINE_TEST_H

#include "gtest/gtest.h"
#include "cbor.h"

class CborphineTest : public ::testing::Test
{
public: // ::testing::Test 

    virtual void SetUp();

protected:

    std::vector<uint8_t> _buffer;
    std::vector<uint8_t> _expected;
    uint8_t*             _data;
    size_t               _size;
};

#endif // CBORPHINE_TEST_H
