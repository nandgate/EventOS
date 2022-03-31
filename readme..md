
# EventOS
An event based operating system for embedded systems (micro-controllers).

## Usage:
* Dependencies:
  * GCC for ARM
  * Doxygen
  * Bash
  * (Under Windows I use cygwin with all the above goodies installed.)
* Use 'make' to compile the host application (blinky)
* Use 'make clean' to remove the previous build.
* Use 'make docs' to generate to the documentation (via doxygen)
* Use 'build.sh' to:
  * Run all of the unit tests
  * Copmile the host application (blinky)
  * Generate the documentation (via doxygen)

## Notes:
* Not everything is working, though most things are working.
* Some of it is certainly broken.
* Docs are incomplete in explaining the idea/philosophy, this will be fixed "soon".
  * Cooperative (not preemptive).
  * Event driven (in case you didn't get that from the name).
  * Single application stack.
  * Not a RTOS, but good enough in some use cases.
  * Simpler and smaller than an RTOS.
  * Fits somewhere between a superloop and a RTOS.
* Docs are currently generated only for the public API.
* The current focus is on the public API, not the implementation (e.g. not optimized).
* The heap memory is used from the C-libs, this will be fixed "soon" (with an embedded appropriate memory allocator).
* Current focus is on the Cortex-M ARM
  * Portability across the entire Cortex-M family: M0, M0+, M1, M3, M4, M7 (goal).
  * No dependency on CMSIS or manufacturers headers files (sane portability) (goal).
  * Ports to other architectures (RISC-V) should be possible without changing the public API (goal).
* The OS in this project is hosted on the "BluePIll" board (STM32F103) because one was handy.
* More examples will be written "soon".
* 100% unit test coverage!
* If this works out the plan is to write/port middleware (net stacks, filesystems, etc.) to run on top of EventOS.
* MIT License.

&copy;2022 NAND Gate Technologies, LLC