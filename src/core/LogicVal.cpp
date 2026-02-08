#include "LogicVal.hpp"
#include <stdexcept>

namespace vfpga {

LogicVal::LogicVal(char c) {
  switch (c) {
  case '0':
    state = LogicState::L0;
    break;
  case '1':
    state = LogicState::L1;
    break;
  case 'x':
  case 'X':
    state = LogicState::LX;
    break;
  case 'z':
  case 'Z':
    state = LogicState::LZ;
    break;
  default:
    state = LogicState::LX;
    break;
  }
}

LogicVal LogicVal::operator~() const {
  switch (state) {
  case LogicState::L0:
    return LogicState::L1;
  case LogicState::L1:
    return LogicState::L0;
  default:
    return LogicState::LX;
  }
}

LogicVal LogicVal::operator&(const LogicVal &other) const {
  // 0 & anything = 0
  if (state == LogicState::L0 || other.state == LogicState::L0)
    return LogicState::L0;
  // 1 & 1 = 1
  if (state == LogicState::L1 && other.state == LogicState::L1)
    return LogicState::L1;
  // Otherwise X
  return LogicState::LX;
}

LogicVal LogicVal::operator|(const LogicVal &other) const {
  // 1 | anything = 1
  if (state == LogicState::L1 || other.state == LogicState::L1)
    return LogicState::L1;
  // 0 | 0 = 0
  if (state == LogicState::L0 && other.state == LogicState::L0)
    return LogicState::L0;
  // Otherwise X
  return LogicState::LX;
}

LogicVal LogicVal::operator^(const LogicVal &other) const {
  // If either is X or Z, result is X
  if (state == LogicState::LX || state == LogicState::LZ ||
      other.state == LogicState::LX || other.state == LogicState::LZ) {
    return LogicState::LX;
  }
  // 0^0=0, 1^1=0, 0^1=1, 1^0=1
  return (state != other.state) ? LogicState::L1 : LogicState::L0;
}

char LogicVal::to_char() const {
  switch (state) {
  case LogicState::L0:
    return '0';
  case LogicState::L1:
    return '1';
  case LogicState::LX:
    return 'X';
  case LogicState::LZ:
    return 'Z';
  }
  return '?';
}

std::string LogicVal::to_string() const { return std::string(1, to_char()); }

std::ostream &operator<<(std::ostream &os, const LogicVal &val) {
  os << val.to_char();
  return os;
}

} // namespace vfpga
