# Technical Specification: Virtual FPGA (vFPGA) Simulation Suite

## 1. System Overview
The vFPGA is a software-defined hardware emulation environment. It consists of a **Cycle-Accurate Simulation Core** (C++20) and a **CAD Toolchain** (Python/C++) for Synthesis, Placement, and Routing (P&R).

---

## 2. Hardware Abstraction Layer (The Fabric)

### 2.1 Logic Element (LE) Architecture
The fundamental unit of logic. Each LE consists of:
* **K-Input LUT (Look-Up Table):** A $2^k$ bitmask. For $K=4$, a 16-bit integer stores the truth table. Input signals $\{i_0, i_1, i_2, i_3\}$ are treated as a 4-bit index to retrieve the output bit.
* **D-Flip-Flop (DFF):** A synchronous state element. Updates on the global `posedge` of the simulation clock.
* **Configurable MUX:** A 1-bit configuration flag determines if the LE output is the raw LUT result (combinational) or the DFF output (registered).

### 2.2 Interconnect Topology
The fabric uses a **Segmented Channel Architecture** (Island Style):
* **Configurable Logic Blocks (CLBs):** A cluster of $N$ LEs (typically 4–8) sharing a local interconnect to reduce global routing pressure.
* **Connection Blocks (C-Blocks):** Buffers and MUXes that interface CLB input/output pins to the horizontal and vertical routing tracks.
* **Wilton Switch Blocks (S-Blocks):** Intersections where routing tracks meet. Configurable switches allow signals to turn corners ($L$-shape) or pass through ($+$ shape) using a disjoint topology to maintain track regularity.



---

## 3. Simulation Engine (The Runtime)

### 3.1 Execution Model: Two-Phase Clocking
To ensure cycle accuracy and prevent race conditions, the engine executes in two distinct phases per simulation tick:

1.  **Combinational Propagation (Async):** * Signals propagate through LUTs and Switch Blocks.
    * Uses a **Directed Acyclic Graph (DAG)** traversal or a convergence loop (`while(signals_changing)`) to resolve logic levels.
    * Maximum iteration depth is enforced to detect unstable combinational loops (e.g., an inverter feeding back into itself without a DFF).
2.  **State Commitment (Sync):** * Triggered on `Clock::RisingEdge()`.
    * All DFFs latch their current input values to their outputs simultaneously.
    * The state is updated globally before the next Combinational Phase begins.

### 3.2 Signal Representation
* **4-State Logic:** `0` (Low), `1` (High), `X` (Unknown/Contention), `Z` (High-Impedance).
* **Contention Handling:** If multiple drivers write to a single routing track, the state resolves to `X`.

---

## 4. The CAD Toolchain (Control Plane)

The toolchain transforms a hardware description into a **Bitstream**.

### 4.1 Placement (Simulated Annealing)
* **Input:** A netlist of logical LUTs.
* **Process:** Randomly maps logical LUTs to physical CLB locations. 
* **Cost Function:** Minimizes the **Bounding Box Wire Length (BBWL)** and local congestion. 
* **Optimization:** Swaps LE positions iteratively, accepting bad moves with decreasing probability (Temperature) to avoid local minima.

### 4.2 Routing (Pathfinder Algorithm)
The engine uses the **Pathfinder Negotiated Congestion** algorithm:
1.  **Iteration 1:** Every net is routed using the shortest path (A* or Dijkstra), ignoring resource overlap.
2.  **Subsequent Iterations:** The "cost" of using a routing track increases based on how many nets are currently trying to use it (congestion).
3.  **Convergence:** Nets are ripped up and re-routed until every signal has a unique, non-overlapping path through the S-Blocks.



---

## 5. Data Structures & Performance

* **Interconnect Map:** A flat 1D array representing the 2D grid to maximize cache locality.
* **Bitstream Format:**
    * `Header`: Fabric dimensions ($N \times M$), $K$-input size.
    * `Payload`: Array of `uint16_t` for LUT masks + `uint8_t` for S-Block configurations.
* **Parallelism:** The Combinational Phase can be multi-threaded by partitioning the grid into independent sub-graphs where no timing dependencies exist.

---

## 6. Project Roadmap
1.  **Core:** Implement `LogicElement` and `SwitchBox` classes in C++20.
2.  **Sim:** Build the `Simulator` loop with a global clock and signal propagation.
3.  **Router:** Implement a basic A* router in Python for the bitstream generator.
4.  **UI:** Create a web-based dashboard (React/Canvas) to visualize real-time signal flow.
