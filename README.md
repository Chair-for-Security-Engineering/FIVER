# FIVER - Fault Injection VERification

This repository contains the source code for the paper [*FIVER - Robust Verification of Countermeasures against Fault Injections*](https://eprint.iacr.org/2021/936.pdf).

Features
---

FIVER is a framework written in C++ relying on Reduced Ordered Binary Decision Diagrams (ROBDDs). The software is used to analyze and verify implementations of countermeasures against fault-injection attacks. The implementation can be given as Verilog netlist or as instruction list.

Contact and Support
---

Please contact Jan Richter-Brockmann (jan.richter-brockmann@rub.de) if you have any questions, comments, if you found a bug that should be corrected, or if you want to reuse the framework or parts of it for your own research projects. 

Build Instructions
---

To build FIVER, please follow the instructions below:

1. Clone the FIVER repository. 
2. Download and include the [Boost Graph Library (BGL)](https://www.boost.org/doc/libs/1_73_0/libs/graph/doc/index.html).
3. Update the `INCLUDES` variable in the FIVER makefile with the path to your copy of BGL.
4. Download the [CUDD](https://davidkebo.com/source/cudd_versions/cudd-3.0.0.tar.gz) package and follow the instructions in the `README` for installation.
5. Change directory to the FIVER folder.
6. `make release`

Quick Start
---

Build the FIVER framework following the instructions above. FIVER is configured by a configuration file placed in the `config/` folder. You can set the number of cores used by FIVER and the memory used per core (passed to the CUDD library). Additionally, you can set the path to the library describing the behavior of the logic gates. To analyze a target design, the `circuit` configurations are used. The design description can either be passed as Verilog (`*.v` file) or as list of instructions (`*.nl` file) to FIVER (examples can be found in `test/` directory). In case you select a Verilog netlist, the module name must be given. The `correction_based` setting is set to `true` in case a correction based countermeasure should be analyzed. The `fault` configurations are used to determine the fault model. Set `interrupt` to `true` in case the analyses should be stopped when an effective fault is found. You will obtain a detailed fault report. 

1. `make release` 
2. `bin/release/verify -c config/verify`

Examplary output for `test/craft-1round/craft-1round-1bit.nl` 

```
[     0.000] Netlist: test/craft-1round/craft-1round-1bit.nl
[     0.002] Parse: 1118 gate(s) / 1779 signal(s)
[     0.169] Elaborate: 1 register stage(s) / 0 logic layer(s) and 2 logic stages. 

Valid fault locations: 766
Reduced fault locations: 0
Analyzing circuit with 128 inputs and 65 outputs.

Found 2 valid stages to inject faults.

Fault injections with n=1 faults per logic level and variate set to 1
-------------------------------------
Permuted fault Locations: 316
[====================================================================================================] 100 %
Permuted fault Locations: 450
[====================================================================================================] 100 %

Tested 766 combinations.

>>> found 0 effective faults
>>> found 78768 ineffective faults
>>> found 19280 detected faults
>>> found 98048 fault scenarios

--------------------------------------------------------------
Tested 766 combinations in total.

>>> found 0 effective faults
>>> found 78768 ineffective faults
>>> found 19280 detected faults
>>> found 98048 fault scenarios

>>> fault rate: 0.000000
Time: 18.922252 ms
```

To obtain the above results, the following configuration file and blacklist is used:

```
# HOW-TO:
#  1) For auto-detection of number of cores used during analysis provide "general.cores = 0", otherwise specify number of cores.
#  2) Specify the amount of RAM to be used for the internal computation on BDDs (given in GB by "general.memory").
#  3) Specify the level of verbosity:
#     - 0: off (report results only)
#     - 1: on (report results and details)
#  4) List all Designs Under Test (DUTs) under "circuit.dut" (with spaces as separating character).
#  5) Variate: defines univariate, bivariate, ... (1,2,...)
#  6) Specify the location of the fault mapping that should be used (mapping)
#  7) Specify the location where faults should be injection (location: c, s, cs)
#  8) blacklist: points to a txt file that contains a list of all entities that should be excluded
#  9) reduced_complexity: 0: disable - 1: naive - 2: aggrassive - 3: conservative
# 10) correction_based: false: detection-based countermeasure, true: correction-based countermeasure
# 11) interrupt: true: verification is interrupted if an effective fault is detected - detailed report of fault location, mapping, and input vectors

[general]
cores               = 8
memory              = 8
verbose             = 1
[library]
file                = cell/Library.txt
name                = NANG45
[circuit]
dut                 = test/craft-1round/craft-1round-1bit.nl
module              = sbox_wrapper
correction_based    = false
[fault]
number              = 1
variate             = 1
mapping             = model/bitflip.txt
location            = cs
blacklist           = model/blacklist.txt
reduced_complexity  = 0
interrupt           = false
```



```
# This file is used to determine entities that should not be faulted
# The parser searches for all strings that are given in this file in the annotations of the nl file.
# In case it finds a match, the corresponding gate will be blacklisted.
Check1_CheckInst Red_ToCheckInst
```

Configuration Details
---

The configuration file contains a setting to control the complexity reduction (`reduced_complexity`). Valid values are:

- 0: complexity reduction is disabled (all gates will be considered for fault injections)
- 1: a naive approach is applied (all registers, input gates to registers, and spreading gates will be considered)
- 2: applies an aggressive approach (should be used with caution, **no guarantee for correctness**, better use approach 3)
- 3: conservative approach which was presented and discussed in the related paper (see below)

Verilog Annotations
---

In case you would like to pass a Verilog netlist to FIVER, the clock signal and control signal need to be annotated. This is required to allow a correct parsing of the netlist to the internally used instruction list (`*.nl` file). The given example shows the syntax of annotations:

```
  input [63:0] Input;
  input [63:0] Key;
  output [63:0] Output;
  (* fiver="clock" *)input clk;
  (* fiver="control" *)input rst;
  output ErrorFlag;
```



Troubleshooting
---

Here are some common issues you may encounter during execution along with possible fixes.

### Shared libraries (libcudd-3.0.0.so)

In case you get an error message similar to: 

```
./bin/verify: error while loading shared libraries: libsylvan.so: cannot open shared object file: No such file or directory
```

please export the `/lib` directory to your linker library path, e.g., using `export LD_LIBRARY_PATH=``pwd``/lib` before executing the binary.

Licensing
---

Copyright (c) 2021, Jan Richter-Brockmann, Pascal Sasdrich, Amir Moradi (verilogParser.h).
All rights reserved.

Please see `LICENSE` for further license instructions.

Publications
---



J. Richter-Brockmann, A. Rezaei Shahmirzadi, P. Sasdrich, A. Moradi, T. Güneysu (2021): [FIVER - Robust Verification of Countermeasures against Fault Injections](https://eprint.iacr.org/2021/936.pdf). TCHES 2021 (preprint)
