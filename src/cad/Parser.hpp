#pragma once

#include "Netlist.hpp"
#include <optional>
#include <string>

namespace vfpga {

class Parser {
public:
  // Parse a JSON file (Yosys compatible format) and return a Netlist
  static std::optional<Netlist> from_json(const std::string &filename);
};

} // namespace vfpga
