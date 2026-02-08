# Technical Specification: Virtual FPGA (vFPGA) Simulation Suite

## 1. System Overview
The vFPGA is a software-defined hardware emulation environment. It consists of a **Cycle-Accurate Simulation Core** (C++20), a **CAD Toolchain** (Python/C++) for Synthesis, Placement, and Routing (P&R), and a **Web-based Visualization Dashboard**.

---

## 2. Hardware Abstraction Layer (The Fabric)

### 2.1 Logic Element (LE) Architecture
The fundamental unit of logic, designed to emulate the behavior of a commercial FPGA Logic Cell. Each LE consists of:
* **K-Input LUT (Look-Up Table):** A $2^K$ bitmask (default $K=4$). Input signals $\{i_0, i_1, \dots, i_{K-1}\}$ form an index to retrieve the output bit.
* **D-Flip-Flop (DFF):** A synchronous state element. Updates on the global `posedge` of the simulation clock.
* **Configurable MUX:** A configuration bit selects the LE output source: either the combinatorial LUT result or the registered DFF output.

### 2.2 Interconnect Topology
The fabric uses a **Segmented Channel Architecture** (Island Style) with a Manhattan grid layout:
* **Configurable Logic Blocks (CLBs):** Clusters of $N$ LEs (typically 4–8) sharing local interconnects.
* **Connection Blocks (C-Blocks):** Programmable switches connecting CLB pins to global routing tracks.
* **Wilton Switch Blocks (S-Blocks):** Junctions where horizontal and vertical tracks intersect. Supports disjoint routing to maximize track utilization.

---

## 3. Simulation Engine (The Runtime)

### 3.1 Execution Model: Two-Phase Clocking
To guarantee cycle accuracy and determinism:
1.  **Combinational Propagation (Async Phase):**
    * Iteratively resolves logic levels through LUTs and interconnects until stability (convergence).
    * Implements a **loop detection watchdog** to identify unstable combinatorial loops (oscillations) and halt simulation with an error.
2.  **State Commitment (Sync Phase):**
    * Triggered on `Clock::RisingEdge()`.
    * Atomically updates all DFF values and Memory Blocks based on their inputs from the end of the Combinational Phase.

### 3.2 Signal Representation
* **4-State Logic:** `0` (Low), `1` (High), `X` (Unknown/Contention/Uninitialized), `Z` (High-Impedance).
* **Contention Handling:** Multiple drivers on a single net resolve to `X`.

### 3.3 I/O Handling
* **GPO (General Purpose Output):** Mapped to virtual LEDs or 7-segment displays in the UI.
* **GPI (General Purpose Input):** Mapped to virtual switches, buttons, or clock generators in the UI.

---

## 4. The CAD Toolchain (Control Plane)

The toolchain compiles high-level designs into a binary bitstream.

### 4.1 Frontend: Netlist Synthesis
* **Input Format:** Standard JSON Netlist or simplified BLIF (Berkeley Logic Interchange Format).
* **Parser:** Converts the input description into an abstract graph of specific Logic Elements and Nets.

### 4.2 Placement (Simulated Annealing)
* **Goal:** Map logical LEs to physical Grid locations ($x, y, z$).
* **Cost Function:** $Cost = \alpha \cdot \text{TotalWireLength} + \beta \cdot \text{Congestion}$.
* **Algorithm:** Iterative swapping with a cooling schedule to escape local minima.

### 4.3 Routing (Pathfinder Negotiation)
* **Algorithm:** Pathfinder Negotiated Congestion.
* **Mechanism:** 
    1. Initially routes all signals via shortest paths (ignoring overuse).
    2. Iteratively rips up and re-routes nets, gradually increasing the cost of overused resources (congestion) until a valid, non-overlapping solution is found.

---

## 5. System Integration & UI

### 5.1 Native Runtime
* **Standalone Application:** Compiled directly to a native executable (Linux/macOS/Windows) using CMake.
* **No Browser Dependency:** The simulation runs entirely on the host CPU.

### 5.2 Visualization Dashboard
* **Graphics Backend:** **SFML** or **Raylib** for high-performance 2D rendering of the FPGA grid and routing.
* **UI Overlay:** **Dear ImGui** for simulation controls, memory inspection, and signal overrides.
* **Features:**
    * Real-time grid rendering.
    * Interactive "Logic Analyzer" view.
    * Manual state injection (clicking switches).

---

## 6. Data Structures & Bitstream

* **Flat Fabric Array:** The grid is flattened into 1D arrays for cache locality.
* **Bitstream Format (`.vbit`):**
    * `Header`: Grid dimensions, LUT size ($K$), Cluster size ($N$).
    * `Configuration`: Run-length encoded stream of LUT masks and Switch settings.

---

## 7. Project Roadmap
1.  **Phase 1: Core:** Implement `LogicType` (4-state), `LogicElement`, and `Grid` in C++20.
2.  **Phase 2: Toolchain:** Build the `Place` (Annealer) and `Route` (Pathfinder) modules in C++/Python.
3.  **Phase 3: Integration:** Integrate ImGui + SFML for the native visualization dashboard.
4.  **Phase 4: Peripherals:** Add Block RAM (BRAM) and DSP slice support.
