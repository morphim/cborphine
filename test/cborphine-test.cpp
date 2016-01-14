/*
Copyright (c) 2015 drugaddicted - c17h19no3 AT openmailbox DOT org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <string>
#include "cborphine-test.h"

void CborphineTest::SetUp()
{
    _buffer.resize(4096);
    _expected = _buffer;
    _data     = &_buffer[0];
    _size     = _buffer.size();
}

void CborphineTest::TearDown()
{
    ASSERT_EQ(_expected, _buffer);
}

void CborphineTest::setExpected(const std::string& value)
{
    std::string srcValue = value;
    srcValue.erase(remove_if(srcValue.begin(), srcValue.end(), isspace), srcValue.end());

    if (srcValue.length() % 2 != 0)
    {
        throw std::logic_error("Incorrect hex string");
    }

    std::string byteStr; 
    std::vector<uint8_t> bytes;

    for (size_t i = 0; i < srcValue.length(); i += 2)
    {
        byteStr = srcValue.substr(i, 2);
        uint8_t byte = (uint8_t)std::strtol(byteStr.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    
    _expected.assign(bytes.begin(), bytes.end());

    if (_expected.size() < _buffer.size())
    {
        _expected.resize(_buffer.size());
    }
}

TEST_F(CborphineTest, WriteNull)
{
    setExpected("f6");
    ASSERT_EQ(CBOR_TRUE, cbor_write_null(&_data, _size));
}

TEST_F(CborphineTest, WriteNullWithZeroBufferSize)
{
    ASSERT_EQ(CBOR_FALSE, cbor_write_null(&_data, 0));
}

TEST_F(CborphineTest, WriteUndefined)
{
    setExpected("f7");
    ASSERT_EQ(CBOR_TRUE, cbor_write_undefined(&_data, _size));
}

TEST_F(CborphineTest, WriteUndefinedWithZeroBufferSize)
{
    ASSERT_EQ(CBOR_FALSE, cbor_write_undefined(&_data, 0));
}


