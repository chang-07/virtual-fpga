#pragma once

#include <iostream>
#include <string>
#include <cstdint>

namespace vfpga {

enum class LogicState : uint8_t {
    L0 = 0, // Low
    L1 = 1, // High
    LX = 2, // Unknown/Contention
    LZ = 3  // High Impedance
};

class LogicVal {
public:
    LogicState state;

    // Constructors
    constexpr LogicVal() : state(LogicState::LX) {}
    constexpr LogicVal(LogicState s) : state(s) {}
    constexpr LogicVal(bool val) : state(val ? LogicState::L1 : LogicState::L0) {}
    explicit LogicVal(char c); // '0', '1', 'x', 'X', 'z', 'Z'

    // Operators
    bool operator==(const LogicVal& other) const { return state == other.state; }
    bool operator!=(const LogicVal& other) const { return state != other.state; }

    LogicVal operator~() const;
    LogicVal operator&(const LogicVal& other) const;
    LogicVal operator|(const LogicVal& other) const;
    LogicVal operator^(const LogicVal& other) const;

    // Helpers
    bool is_0() const { return state == LogicState::L0; }
    bool is_1() const { return state == LogicState::L1; }
    bool is_X() const { return state == LogicState::LX; }
    bool is_Z() const { return state == LogicState::LZ; }

    char to_char() const;
    std::string to_string() const;

    friend std::ostream& operator<<(std::ostream& os, const LogicVal& val);
};

} // namespace vfpga
