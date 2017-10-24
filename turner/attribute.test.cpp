#include <turner/common.test.hpp>
#include <turner/attribute.hpp>


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


TEST_F(any_attribute, size)
{
  data[3] = 0; EXPECT_EQ(4U, as_attribute(data)->size());

  data[3] = 1; EXPECT_EQ(8U, as_attribute(data)->size());
  data[3] = 2; EXPECT_EQ(8U, as_attribute(data)->size());
  data[3] = 3; EXPECT_EQ(8U, as_attribute(data)->size());
  data[3] = 4; EXPECT_EQ(8U, as_attribute(data)->size());

  data[3] = 5; EXPECT_EQ(12U, as_attribute(data)->size());
  data[3] = 6; EXPECT_EQ(12U, as_attribute(data)->size());
  data[3] = 7; EXPECT_EQ(12U, as_attribute(data)->size());
  data[3] = 8; EXPECT_EQ(12U, as_attribute(data)->size());
}


TEST_F(any_attribute, next)
{
  auto expected = as_attribute(&data[4]);
  data[3] = 0; EXPECT_EQ(expected, as_attribute(data)->next());

  expected = as_attribute(&data[8]);
  data[3] = 1; EXPECT_EQ(expected, as_attribute(data)->next());
  data[3] = 2; EXPECT_EQ(expected, as_attribute(data)->next());
  data[3] = 3; EXPECT_EQ(expected, as_attribute(data)->next());
  data[3] = 4; EXPECT_EQ(expected, as_attribute(data)->next());

  expected = as_attribute(&data[12]);
  data[3] = 5; EXPECT_EQ(expected, as_attribute(data)->next());
  data[3] = 6; EXPECT_EQ(expected, as_attribute(data)->next());
  data[3] = 7; EXPECT_EQ(expected, as_attribute(data)->next());
  data[3] = 8; EXPECT_EQ(expected, as_attribute(data)->next());
}


}} // namespace turner_test
