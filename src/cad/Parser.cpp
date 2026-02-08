#include "Parser.hpp"
#include "../utils/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace vfpga {

std::optional<Netlist> Parser::from_json(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    return std::nullopt;
  }

  try {
    json j;
    file >> j;

    Netlist netlist;

    // Yosys JSONs usually have a "modules" object at the top level
    if (!j.contains("modules")) {
      std::cerr << "Error: Invalid JSON netlist (missing 'modules')"
                << std::endl;
      return std::nullopt;
    }

    // We assume a single top-level module for now, or take the first one
    // Typically the top module name is the key.
    auto modules = j["modules"];
    if (modules.empty()) {
      std::cerr << "Error: No modules found in netlist" << std::endl;
      return std::nullopt;
    }

    // Just take the first module found
    auto module_it = modules.begin();
    std::string module_name = module_it.key();
    auto module_data = module_it.value();

    // 1. Parse Ports (Top-level inputs/outputs)
    if (module_data.contains("ports")) {
      for (auto &[port_name, port_data] : module_data["ports"].items()) {
        std::string direction = port_data["direction"];
        if (direction == "input") {
          netlist.inputs.push_back(port_name);
          // Create a net for this input so cells can connect to it
          // The bits of the port are often listed in "bits".
          // For simplicity, handle single-bit ports or name them by index.
        } else if (direction == "output") {
          netlist.outputs.push_back(port_name);
        }

        // netlist.add_net(port_name); // Create net for valid top-level IO
      }
    }

    // 2. Parse Cells
    if (module_data.contains("cells")) {
      for (auto &[cell_name, cell_data] : module_data["cells"].items()) {
        std::string cell_type = cell_data["type"];
        auto cell = netlist.add_cell(cell_name, cell_type);

        // Parameters (e.g. LUT mask)
        if (cell_data.contains("parameters")) {
          for (auto &[param_name, param_val] :
               cell_data["parameters"].items()) {
            // Some params are strings, some ints. Convert to string for
            // storage.
            if (param_val.is_string()) {
              cell->parameters[param_name] = param_val;
            } else {
              cell->parameters[param_name] = param_val.dump();
            }
          }
        }

        // Port Connections
        if (cell_data.contains("connections")) {
          for (auto &[port_name, conn_bits] :
               cell_data["connections"].items()) {
            // Determine direction based on cell type knowledge or assume
            // generic? For generic parser, we just need to know which net
            // connects to which port
            PortDirection dir = PortDirection::INOUT; // Placeholder
            cell->add_port(port_name, dir);

            // conn_bits is usually an array of integers (net IDs).
            // For simplicity, we'll map bit indices to string names.
            for (auto &bit : conn_bits) {
              if (bit.is_number_integer()) {
                std::string net_name = "net_" + std::to_string((int)bit);
                netlist.connect(cell_name, port_name, net_name);
              } else if (bit.is_string()) {
                // Constants like "0", "1", "x"?
                // Yosys uses integers for nets. strings might be constants.
              }
            }
          }
        }
      }
    }

    // 3. Parse Netnames (Optional, mapped net names to bit indices)
    // If we want human-readable net names instead of "net_123"
    if (module_data.contains("netnames")) {
      // TODO: Mapping logic
    }

    return netlist;

  } catch (const json::parse_error &e) {
    std::cerr << "JSON Parse Error: " << e.what() << std::endl;
    return std::nullopt;
  }
}

} // namespace vfpga
