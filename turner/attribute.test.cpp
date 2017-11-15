#include <turner/common.test.hpp>


namespace turner_test { namespace {


using any_attribute = turner_test::fixture;


// each test modifies it before checks
static uint8_t data[] =
{
  0x00, 0x01, // Type
  0x00, 0x00, // Length
  0x01, 0x02, // Data
  0xcc, 0xcc,

  // next attribute (not actually used)
  0x00, 0x01,
  0x00, 0x00,
};


inline auto as_attribute (const uint8_t *p) noexcept
{
  return reinterpret_cast<const turner::any_attribute_t *>(p);
}


TEST_F(any_attribute, type)
{

  for (uint8_t i = 1;  i != 5;  ++i)
  {
    data[1] = i;
    auto attribute = as_attribute(data);
    EXPECT_EQ(i, attribute->type()) << uint32_t(i);
  }
}


TEST_F(any_attribute, length)
{
  for (uint8_t i = 0;  i != 4;  ++i)
  {
    data[3] = i;
    auto attribute = as_attribute(data);
    EXPECT_EQ(i, attribute->length()) << uint32_t(i);
  }
}


TEST_F(any_attribute, data)
{
  for (uint8_t i = 0;  i != 4;  ++i)
  {
    data[3] = i;
    auto attribute = as_attribute(data);
    EXPECT_EQ(&data[4], attribute->data());
  }
}


}} // namespace turner_test
