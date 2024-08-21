#pragma once

namespace MyEngine {
using MouseCode = unsigned char;

namespace Mouse {
enum : MouseCode {
  ButtonLeft = 1,
  ButtonMiddle = 2,
  ButtonRight = 3,
  ButtonX1 = 4,
  ButtonX2 = 5
};
}
} // namespace MyEngine
