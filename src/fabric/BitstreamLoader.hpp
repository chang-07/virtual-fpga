#pragma once

#include "Fabric.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace vfpga {

class BitstreamLoader {
public:
  // Load configuration from a file into the Fabric
  // For now, this is a placeholder. Real implementation will parse a custom
  // binary format.
  static bool load(const std::string &filename, Fabric &fabric);

  // Save current configuration to a file
  static bool save(const std::string &filename, const Fabric &fabric);
};

} // namespace vfpga
