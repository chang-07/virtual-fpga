#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace vfpga {

// Forward declarations
class Cell;
class Net;

enum class PortDirection { INPUT, OUTPUT, INOUT };

struct Port {
  std::string name;
  PortDirection direction;
  std::shared_ptr<Net> connected_net;

  Port(std::string n, PortDirection d)
      : name(n), direction(d), connected_net(nullptr) {}
};

class Cell {
public:
  std::string name;
  std::string type; // e.g., "$lut", "$dff", "AND2"
  std::map<std::string, Port> ports;
  std::map<std::string, std::string> parameters; // e.g., LUT_MASK -> "0xF"

  Cell(std::string n, std::string t) : name(n), type(t) {}

  void add_port(std::string port_name, PortDirection dir) {
    ports.emplace(port_name, Port(port_name, dir));
  }
};

class Net {
public:
  std::string name;
  // We could store list of connected ports here for faster traversal
  // but for now, connectivity is stored in Port::connected_net

  Net(std::string n) : name(n) {}
};

class Netlist {
public:
  std::map<std::string, std::shared_ptr<Cell>> cells;
  std::map<std::string, std::shared_ptr<Net>> nets;
  std::vector<std::string> inputs;  // Top-level inputs
  std::vector<std::string> outputs; // Top-level outputs

  std::shared_ptr<Cell> add_cell(std::string name, std::string type) {
    auto cell = std::make_shared<Cell>(name, type);
    cells[name] = cell;
    return cell;
  }

  std::shared_ptr<Net> add_net(std::string name) {
    if (nets.find(name) == nets.end()) {
      nets[name] = std::make_shared<Net>(name);
    }
    return nets[name];
  }

  // Connect a cell port to a net
  void connect(std::string cell_name, std::string port_name,
               std::string net_name) {
    auto cell_it = cells.find(cell_name);
    auto net = add_net(net_name);

    if (cell_it != cells.end()) {
      auto &port_map = cell_it->second->ports;
      auto port_it = port_map.find(port_name);
      if (port_it != port_map.end()) {
        port_it->second.connected_net = net;
      } else {
        std::cerr << "Warning: Port " << port_name << " not found in cell "
                  << cell_name << std::endl;
      }
    } else {
      std::cerr << "Warning: Cell " << cell_name << " not found" << std::endl;
    }
  }
};

} // namespace vfpga
