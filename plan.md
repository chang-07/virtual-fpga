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

## 7. Detailed Project Roadmap

### Phase 1: Core Simulation Engine (The Kernel)
**Focus:** Correctness, Basic Primitives, Unit Testing.

*   **1.1 Data Types & Logic**
    *   [ ] Implement `LogicVal` (4-state: `0`, `1`, `X`, `Z`) with overloaded operators (`&`, `|`, `^`, `~`).
    *   [ ] Implement `Signal` class (representing a wire/net) with resolution functions for multiple drivers (handling contention).
    *   [ ] Create `Logger` and `Assertion` macros for debug capability.
*   **1.2 Hardware Primitives**
    *   [ ] **LUT (Look-Up Table):** Implement template-based $K$-input LUT with configuration masking.
    *   [ ] **DFF (Flip-Flop):** Implement positive-edge triggered state element with optional enable/reset lines.
    *   [ ] **Mux/Switch:** Implement configurable routing switches for the interconnect.
*   **1.3 The Grid & Fabric Helpers**
    *   [ ] Define `Tile` structure (holding primitive pointers and switch matrices).
    *   [ ] Implement `Fabric` class to manage the global $W \times H$ grid.
    *   [ ] Develop `BitstreamLoader` to initialize LUT masks and Switch settings from a binary (`.vbit`) file.
*   **1.4 Verification**
    *   [ ] Create unit tests for `LogicVal` truth tables.
    *   [ ] Create a "Single LUT" testbench (manually verify XOR/AND behavior correctness).

### Phase 2: CAD Toolchain (The Compiler)
**Focus:** Mapping Netlists to the Fabric.

*   **2.1 Frontend Parsing**
    *   [ ] Implement parser for JSON Netlists (compatible with Yosys JSON output).
    *   [ ] Build an in-memory `NetlistGraph` (Nodes = Cells, Edges = Nets).
*   **2.2 Tech Mapping & Packing**
    *   [ ] Map generic logic gates to physical `$lut` and `$dff` cells.
    *   [ ] Pack primitives into Clusters (CLBs) if using a clustered architecture.
*   **2.3 Physical Placement (Simulated Annealing)**
    *   [ ] Implement random placement initialization.
    *   [ ] Develop bounding-box `CostFunction`: $Cost = \alpha \cdot HPWL + \beta \cdot Congestion$.
    *   [ ] Implement **Annealing Loop**: Swap blocks, evaluate delta cost, apply temperature cooling schedule.
*   **2.4 Routing (Pathfinder)**
    *   [ ] Build `RoutingGraph`: Nodes = Pins/Wires, Edges = Switches.
    *   [ ] Implement **A* Search** (Shortest Path) for individual nets.
    *   [ ] Implement **Congestion Negotiation**: Iteratively rip-up and reroute colliding nets with increasing costs until conflict-free.

### Phase 3: Visualization & Interactive Runtime
**Focus:** UI/UX, Real-time Interaction.

*   **3.1 Graphics Engine Setup**
    *   [ ] Initialize Native Window (using SFML or Raylib).
    *   [ ] Implement a **Batch Renderer** (Vertex Arrays) for the Grid to handle high frame rates.
    *   [ ] Implement "Zoom & Pan" camera controls.
*   **3.2 UI Overlays (ImGui)**
    *   [ ] **Sidebar:** Integrate simulation controls (Speed, Step, Pause, Reset).
    *   [ ] **Inspector:** Logic to hover over tiles and display coordinate $(x,y)$ and internal register state.
    *   [ ] **Signal Analyzer:** Ability to click a wire/pin to add it to a "Waveform View".
*   **3.3 Interactive Stimulation**
    *   [ ] Implement "Virtual Switch" clickable elements (toggles GPI bits).
    *   [ ] Implement "Virtual LED" rendering (reads GPO bits).

### Phase 4: Advanced Architecture Features
**Focus:** Completeness, Performance.

*   **4.1 Hard Blocks**
    *   [ ] Implement **BRAM (Block RAM):** 4K/16K blocks with read/write ports.
    *   [ ] Implement **DSP Slices:** Hardware multipliers/accumulators.
*   **4.2 Timing Analysis**
    *   [ ] Implement connection delay modeling (Manhattan distance approx).
    *   [ ] Generate Static Timing Analysis (STA) reports.
*   **4.3 Optimization**
    *   [ ] Implement multithreaded simulation (graph partitioning).
    *   [ ] Optimize rendering culling for large grids.
