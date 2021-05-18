#include <iostream>

#include <gtest/gtest.h>

#if 1
#include "event_loop.h"
#include "epoller.h"
#include "channel.h"
#endif

namespace wuduo {

TEST(EventLoopTest, SimpleTest) {
  std::cout << "EventLoopTest\n";
  EXPECT_EQ(1, 2);
}

}
