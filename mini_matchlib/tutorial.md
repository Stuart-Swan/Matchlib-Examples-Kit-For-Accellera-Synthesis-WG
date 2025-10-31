
# Codex Prompt

In this directory there is the file

`include/new_connections.h`

There is an example that uses this code here:

`examples/06_thruput_accurate`

The example is built with the build.sh script in that directory,
and run with the command:

`./sim_sc`

Run the example and inspect the output to stdout.

Create a tutorial document that describes how the new_connections.h file works.
The audience is a C++ user who has basic SystemC and hardware design knowledge.
The tutorial should cover almost everything that is present in new_connections.h.
Ideally it should be structured in the same sequence that a SystemC simulation executes:
the phases during design instantiation and elaboration, start of simulation, and actual simulation.
Explain in detail how new_connections.h enables easy "throughput accurate" modeling for designs
such as shown in the 06* example.

If you have questions or need clarifications, ask me.

Use the command:

`gdb sim_sc`

to run the example to inspect the execution so you can deeply understand how it all works.

Output the tutorial into a new file tutorial.md . Use appropriate markdown formatting to make it easy to follow.
Include easy to follow references to the code in new_connections.h.

# Throughput-Accurate Connections Tutorial

This tutorial explains how `include/new_connections.h` structures throughput-accurate transaction channels for SystemC designs. We walk through the code in the same order that a SystemC simulation executes—design instantiation and elaboration, start of simulation, and clocked execution—using the `examples/06_thruput_accurate` design as a running example.

## Why These Connections Matter

The goal of the Connections API is to let synthesizable modules (written with blocking `Push`/`Pop` methods) run in a SystemC simulation that preserves clock-cycle throughput semantics. The mini implementation provides a self-contained handshake infrastructure that mirrors the ready/valid hardware protocol while remaining easy to instrument and reason about in software.

Key takeaways:

- A single global clock drives the entire handshake network.
- Each channel buffers exactly one message in each direction, matching the timing of a throughput-accurate RTL block.
- Runtime management is centralized in a lightweight connection manager thread that updates handshake signals around every clock edge.

---

## Phase 1 – Design Instantiation & Static Setup

### 1.1 Mandatory include and synthesis guards

`new_connections.h` pulls in SystemC and enforces `SC_INCLUDE_DYNAMIC_PROCESSES` so that it can spawn helper processes (`include/new_connections.h:24-33`). Most of the interesting logic is under `#ifndef __SYNTHESIS__`; the `__SYNTHESIS__` branches collapse to simple interface stubs for downstream synthesis tools, while the simulation branches provide the detailed behavior.

### 1.2 Global clock registration (`SimConnectionsClk`)

- `SimConnectionsClk` stores a pointer to the user-provided `sc_clock` and the handshake epsilon used to separate `Post`/`Pre` phases (`include/new_connections.h:65-81`).
- The top-level design must call `Connections::set_sim_clk(&clk);` during module construction, before simulation starts. In the example, this happens inside `Top`’s constructor right after the clock is declared (`examples/06_thruput_accurate/testbench.cpp:22-41`).

### 1.3 Connection manager singleton

- `ConManager` holds every active handshake endpoint (`Blocking_abs` derivatives) in a `tracked` vector (`include/new_connections.h:97-156`).
- Calls to `get_conManager().add(...)` register the endpoints as soon as channels are constructed.
- `init_sim_clk()` checks that `set_sim_clk()` was called and spawns the manager’s runtime thread.
- Static singletons (`ConManager_statics`) make the manager and clock globally accessible without extra plumbing (`include/new_connections.h:159-183`).

### 1.4 Channel base class

All concrete channels derive from `chan_base`. Under simulation this extends `sc_channel` so the kernel invokes their `start_of_simulation()` hook later (`include/new_connections.h:187-195`).

---

## Phase 2 – Elaboration: Binding Ports and Channels

### 2.1 Channel internals created at construction time

Constructing a `Connections::Combinational<Message>` channel allocates three SystemC signals (`vld`, `dat`, `rdy`) and two simulation helper objects (`In_sim_port` and `Out_sim_port`) (`include/new_connections.h:329-416`).

- `In_sim_port` exposes a ready signal to the producer and captures data from the channel when a valid input arrives (`include/new_connections.h:220-268`).
- `Out_sim_port` drives the valid/data pair seen by downstream consumers and clears its buffer once the consumer indicates readiness (`include/new_connections.h:271-317`).
- Both helpers immediately register themselves with the `ConManager` via `get_conManager().add(this)` inside their constructors, ensuring they participate in the global handshake updates.

Internally each channel keeps a single-element buffer for input (`in_buf_dat`, `in_buf_vld`) and output (`out_buf_dat`, `out_buf_vld`) storage (`include/new_connections.h:410-413`). This is what preserves cycle accuracy: values do not immediately transfer between producer and consumer; instead they advance one stage per clock under the manager’s control.

### 2.2 Port types (`Connections::In` and `Connections::Out`)

Modules declare interface ports of type `Connections::In<T>` and `Connections::Out<T>` (`include/new_connections.h:549-753`). These derive from `sc_port` and forward all API calls (`Pop`, `Push`, resets, non-blocking variants) to whatever channel instance is bound to them at elaboration.

Key elaboration-time behavior:

- `operator()` overloads wire the handshake signals between ports and channels (`include/new_connections.h:631-655` and `728-752`).
- `end_of_elaboration()` stores human-readable names for the backing helper objects (`In_sim_port::set_in_port_names`, `Out_sim_port::set_out_port_names`) so diagnostics can identify the endpoints later (`include/new_connections.h:620-626` and `717-723`).
- Optional `disable_spawn()` calls (if used) are validated to ensure they happen before the kernel locks down the design (`include/new_connections.h:612-617` and `709-714`). When a port is disabled the corresponding helper simply stops toggling ready/valid, which is used when processes want to directly access the rdy/vld/dat signals rather than calling Push/Pop/PushNB/PopNB.

### 2.3 Reset preparation

Module reset procedures (e.g., `dut::main()` at `examples/06_thruput_accurate/dut.h:24-33`) call `Reset()` on every port. That funnels into `Combinational_base::ResetRead()` and `ResetWrite()` (`include/new_connections.h:418-427`). Besides clearing the local buffers, these methods invoke `get_conManager().add_clock_event(...)`, which is just a thin wrapper over `init_sim_clk()`. This matters because the handshake manager thread is created lazily the first time any channel participates in a reset.

---

## Phase 3 – Start of Simulation Hooks

When SystemC processes `start_of_simulation()` callbacks, two important things happen:

1. `SimConnectionsClk::start_of_simulation()` assigns a default epsilon of 10 ps (`include/new_connections.h:74-80`). The epsilon acts as a tiny temporal separation between the two halves of the handshake manager loop.
2. Each `Combinational_base` channel finalizes endpoint naming. If no ports were bound (i.e., the user accesses the channel object directly), default names such as `my_chan.In` and `my_chan.Out` are applied (`include/new_connections.h:513-525`).

Because `Reset()` must be called before the clock manager spins, these hooks are guaranteed to run before any user threads start executing. If `set_sim_clk()` was forgotten, `init_sim_clk()` reports an error and terminates before time advances (`include/new_connections.h:117-120`).

---

## Phase 4 – Runtime: Throughput-Accurate Handshake

### 4.1 The manager thread

`ConManager::run()` is the heart of throughput accuracy (`include/new_connections.h:130-155`). Once spawned, it repeatedly:

1. Waits for `epsilon` time units after the active clock edge.
2. Calls `Post()` on every tracked endpoint. For an `Out_sim_port`, this drives `vld` and `dat` when the output buffer holds a value; for an `In_sim_port`, this asserts `rdy` whenever the input buffer is empty (`include/new_connections.h:243-301`).
3. Waits for `period - 2*epsilon`, which lands just before the next active edge.
4. Calls `Pre()` on each endpoint. `In_sim_port::Pre()` latches new data into its buffer when both `vld` and `rdy` are high, while `Out_sim_port::Pre()` clears its buffer once a transfer completes (`include/new_connections.h:252-307`).
5. Waits for another `2*epsilon`, returning to the beginning of the cycle.

This 2-phase update aligns exactly with the ready/valid semantics of hardware pipelines: producers present new data immediately after a clock edge, consumers sample it just before the next edge. Because all channels share the same thread, multi-channel transfers remain synchronized with a single clock source.

### 4.2 Blocking and non-blocking APIs

The user-facing API on `Combinational_base` exposes the expected blocking and non-blocking methods:

- `Push()` and `Pop()` loop on `wait()` until the handshake succeeds. For example, `dut::main()` issues `in1.Pop()`/`out1.Push()` and naturally stalls if the downstream end is not ready (`examples/06_thruput_accurate/dut.h:34-41`).
- `PushNB()`/`PopNB()` attempt the transfer once and report success (`include/new_connections.h:441-478`).
- `PeekNB()` lets consumers observe upcoming data without removing it (`include/new_connections.h:451-457`).

Behind the scenes these APIs simply manipulate the local one-item buffers; the manager thread does the actual signal driving so the blocking semantics align with cycle boundaries.

### 4.3 Interaction with the example design

Running the example under `gdb` (as required) produces a trace that shows exactly one result per clock tick:

```
Stimulus started
...
8 ns TB resp sees: 000000000 000000000
9 ns TB resp sees: 000000101 000001001
```

Each timestamp advances by 1 ns—the period of `Top::clk`—demonstrating that pushes from `stim()` are accepted at full throughput and the DUT produces a new sum every cycle once the pipeline fills. This is a direct consequence of the manager’s `Post`/`Pre` scheduling and the channel-local buffering described above.

### 4.4 Debugging aids

Because every helper inherits from `Blocking_abs`, the manager could be extended to print endpoint names or dump traffic. The mini version only stores the names (`include/new_connections.h:263-316`), but the infrastructure mirrors the production library’s logging features. The `disable_spawn()` hooks allow advanced users to bypass automatic ready/valid toggling if they want direct access to the rdy/vld/dat signals.

---

## Putting It Together: Design Workflow

1. **Declare the global clock** and call `Connections::set_sim_clk()` during top-level construction (`examples/06_thruput_accurate/testbench.cpp:24-30`).
2. **Instantiate `Connections::Combinational` channels** between producers and consumers (testbench or DUT). The constructors automatically register their handshake helpers.
3. **Bind DUT ports** using the usual SystemC port binding syntax; the operators connect `vld/rdy/dat` signals and store endpoint names.
4. **Issue `Reset()` on every port/channel** from your reset thread before exercising the design. This lazily spins up the manager thread and clears buffers.
5. **Use `Push`/`Pop` APIs** inside your synchronous processes. The manager thread handles synchronization so your code can be written in a natural blocking style while remaining throughput-accurate.
6. **(Optional) Use non-blocking or peek variants** when desired.

Follow this recipe and the simulation will mirror cycle-by-cycle hardware timing without forcing your code to manipulate `sc_signal` objects directly.

---

## Additional Notes and Limitations

- Only a single global clock is supported (`include/new_connections.h:17-19`, `110-128`). Designs with multiple asynchronous domains would need the full library.
- Features such as logging, random stalls, and tracing are omitted here for clarity but can be layered on top of the same `Blocking_abs` infrastructure.
- The `__SYNTHESIS__` branches give synthesis tools clean handshake semantics while the simulation branches provide richer behavior. You can compile the same source for both simulation and downstream RTL generation.

With these pieces in mind, you can extend `new_connections.h` for larger designs or adapt the pattern to your own throughput-accurate modeling needs.
