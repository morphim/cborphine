#ifndef CBORPHINE_TEST_H
#define CBORPHINE_TEST_H

#include "gtest/gtest.h"
#include "cbor.h"

class CborphineTest : public ::testing::Test
{
public: // ::testing::Test 

    virtual void SetUp();

protected:

    uint8_t  _buffer[4096];
    size_t   _size;
    uint8_t* _data;

};

#endif // CBORPHINE_TEST_H
