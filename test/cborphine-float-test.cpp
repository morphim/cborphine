#include "cborphine-test.h"

TEST_F(CborphineTest, WriteFloat_0_dot_0)
{
    std::vector<uint8_t> expected = { 0xf9, 0x00, 0x00 };
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 0.0));
    ASSERT_EQ(0, std::memcmp(&expected[0], _buffer, expected.size()));
}

TEST_F(CborphineTest, WriteFloat_minus_0_dot_0)
{
    std::vector<uint8_t> expected = { 0xf9, 0x80, 0x00 };
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, -0.0));
    ASSERT_EQ(0, std::memcmp(&expected[0], _buffer, expected.size()));
}

TEST_F(CborphineTest, WriteFloat_1_dot_0)
{
    std::vector<uint8_t> expected = { 0xf9, 0x3c, 0x00 };
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 1.0));
    ASSERT_EQ(0, std::memcmp(&expected[0], _buffer, expected.size()));
}

TEST_F(CborphineTest, WriteFloat_1_dot_5)
{
    std::vector<uint8_t> expected = { 0xf9, 0x3e, 0x00 };
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 1.5));
    ASSERT_EQ(0, std::memcmp(&expected[0], _buffer, expected.size()));
}

TEST_F(CborphineTest, WriteFloat_65504_dot_0)
{
    std::vector<uint8_t> expected = { 0xf9, 0x7b, 0xff };
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 65504.0));
    ASSERT_EQ(0, std::memcmp(&expected[0], _buffer, expected.size()));
}

TEST_F(CborphineTest, WriteFloat_5_dot_960464477539063e_minus_8)
{
    std::vector<uint8_t> expected = { 0xf9, 0x00, 0x01 };
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 5.960464477539063e-8));
    ASSERT_EQ(0, std::memcmp(&expected[0], _buffer, expected.size()));
}

TEST_F(CborphineTest, WriteFloat_0_dot_00006103515625)
{
    std::vector<uint8_t> expected = { 0xf9, 0x04, 0x00 };
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 0.00006103515625));
    ASSERT_EQ(0, std::memcmp(&expected[0], _buffer, expected.size()));
}

TEST_F(CborphineTest, WriteFloat_minus_4_dot_0)
{
    std::vector<uint8_t> expected = { 0xf9, 0xc4, 0x00 };
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, -4.0));
    ASSERT_EQ(0, std::memcmp(&expected[0], _buffer, expected.size()));
}

TEST_F(CborphineTest, WriteFloat_minus_4_dot_1)
{
    std::vector<uint8_t> expected = { 0xf9, 0xc4, 0x00 };
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, -4.0));
    ASSERT_EQ(0, std::memcmp(&expected[0], _buffer, expected.size()));
}

TEST_F(CborphineTest, WriteFloat_100000_dot_0)
{
    std::vector<uint8_t> expected = { 0xfa, 0x47, 0xc3, 0x50, 0x00 };
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 100000.0));
    ASSERT_EQ(0, std::memcmp(&expected[0], _buffer, expected.size()));
}

TEST_F(CborphineTest, WriteFloat_3_dot_4028234663852886e_plus_38)
{
    std::vector<uint8_t> expected = { 0xfa, 0x7f, 0x7f, 0xff, 0xff };
    ASSERT_EQ(CBOR_TRUE, cbor_write_float(&_data, _size, 3.4028234663852886e+38));
    ASSERT_EQ(0, std::memcmp(&expected[0], _buffer, expected.size()));
}