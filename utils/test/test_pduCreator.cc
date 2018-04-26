#include <gtest/gtest.h>


class PduCreatorTest : public testing::Test
{
  void SetUp(){}

  void TearDown(){}
};

TEST_F(PduCreatorTest, creatingReqPacket){
  EXPECT_EQ(15, 13);

}