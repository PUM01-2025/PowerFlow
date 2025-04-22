# PowerFlow

PowerFlow is a [power flow/load flow](https://en.wikipedia.org/wiki/Power-flow_study) simulation tool that can be used to calculate voltages in electrical grids given known power consumptions. PowerFlow is written in C++ for fast performance and comes with Matlab and Python bindings for use within Matlab and Python scripts.

Version 1.0 of PowerFlow was written in 2025 by a group of eight students as part of the course TDDD96 "Software Engineering - Bachelor Project" at Linköping University.

## Building

To build PowerFlow for your platform, you need [CMake](https://cmake.org/) and a suitable C++17 compiler:

- Windows: MSVC, shipped alongside [Visual Studio 2022](https://visualstudio.microsoft.com/).
- Linux: g++ version >= ?.
- Mac: clang version >= ?.

In addition to CMake and a compiler, you may need additional software depending on the build target:

### Matlab

PowerFlow can be compiled into a Matlab executable (MEX) that can then be used like any other Matlab function. PowerFlow has been tested to work with Matlab version R2024b.

To be able to compile and run the MEX file, Matlab must be installed on your computer.

FYLL I CMAKE-KOMMANDON HÄR!

### Python

FYLL I INSTRUKTIONER HÄR!

### Standalone?

SKA DETTA TAS MED?

## Usage

It is strongly recommended to read the "General concepts" section below before continuing with the usage instructions. For examples of use cases, see the `examples/` directory.

### General concepts

In PowerFlow, an electrical network/power network is referred to as a *network* consisting of one or more interconnected *grids* that each have an associated voltage level. A grid consists of a set of *nodes* connected by *edges*, representing the cables in the grid. A network can thus be depicted as a graph, for example:

INFOGA BILD HÄR!

Each edge in the graph has an associated impedance (Z) and each node has an associated quantity depending on its type (see below). *Connections* between grids (the dashed lines in the graph) represent ideal transformers.

#### Node types

A grid node is of one of the following types: LOAD, MIDDLE, SLACK_EXTERNAL or SLACK.

- LOAD nodes are nodes where the power consumption is known and the voltage is unknown.
- MIDDLE nodes are nodes that are only used for branching and connections.
- SLACK_EXTERNAL nodes are nodes where the voltage is known and the power is unknown.
- SLACK nodes are similar to SLACK_EXTERNAL nodes, but the voltage is instead specified by a connection to another grid.

Note that:

- Each grid in a network must have a voltage reference. Therefore, at least one SLACK_EXTERNAL or SLACK node must exist in each grid.
- When connecting two grids, one of the nodes has to be a SLACK node and the other node has to be a MIDDLE node.

#### Per-unit

All calculations are performed using *per-unit* values instead of actual units (Volt, Watt etc.). Per-unit for voltages and powers are defined as:

GÅR DET ATT INFOGA FORMLER I MARKDOWN?

where *S_base* and *V_base* are real-valued scale factors. Each grid in a network has an associated S_base and V_base.

From the above definitions, it follows that per-unit values for impedances can be calculated as follows:

FYLL I HÄR!

Note that sqrt(3) ... TREFAS??

#### Solvers

PowerFlow implements two different algorithms (solvers): For grids that have a single SLACK/SLACK_EXTERNAL node, contain no cycles and where the LOAD nodes are located at the "leaves", a *Backward-Forward-Sweep* (BFS) algorithm is used. For other grids, the *Gauss-Seidel* algorithm is used. PowerFlow automatically detects which solver is most suitable for each grid.

#### Limitations

Some limitations on the structure of a network are imposed by PowerFlow. It is not possible to:

- INGA DUBBLA KANTER??
- MER HÄR?

### Network files

A *network file* is a text file that describes a network in a simple format that PowerFlow understands. The syntax of network files is best described by an example. The following example defines the network illustrated in the "General concepts" section:

```
# Lines beginning with # are ignored.

# The "grid" command indicates the start of a grid description.
# Since this is the first grid in the file, it will get ID 0.
grid

# Here, S_base followed by V_base are specified.
1000000000 10000

# Här följer kabeldata på formatet <startnod> <slutnod> <impedans>.
# The % sign marks end of list.
0 1 (0.05, 0.05)
1 2 (0.05, 0.05)
1 3 (0.05, 0.05)
%

# Här anges de LOAD-noder som studeras. Ett nodindex på varje rad. Listan
# avslutas med %-tecken.
# (I det här mellanspänningsnätet är den här listan tom.)
0 s
%

# Första lågspänningsnätet.
grid
10000000 400
0 1 (0.02, 0.02)
1 2 (0.05, 0.03)
%
0 s
2 l
%

# Andra lågspänningsnätet.
grid
10000000 400
0 1 (0.02, 0.03)
0 2 (0.03, 0.03)
%
0 s
1 l
2 l
%

# Här anges anslutningarna mellan näten på formatet:
# <slacknät-nummer> <slacknod-index> <pqnät-nummer> <pqnod-index>
# Listan måste avslutas med ett %-tecken.
connections
0 2 1 0
0 3 2 0
%
```

### Using PowerFlow in a Matlab script

***NOTE:** On Ubuntu and possibly other Linux distributions, Matlab may need to be started using a command similar to `LD_PRELOAD=/lib/x86_64-linux-gnu/libstdc++.so.6 matlab`.*

FYLL I HÄR!

### Using PowerFlow in a Python script

FYLL I HÄR!

### Using the standalone application?

SKA DETTA TAS MED?

## For developers

### Unit tests

FYLL I HÄR!

### Project structure

- `examples/` - Various examples.
- `include/powerflow/` - Public headers.
- `matlab/` - Matlab implementation.
- `python/` - Python implementation.
- `src/` - powerflow source code.
- `standalone/` - Standalone executable.
- `tests/` - Unit tests.
