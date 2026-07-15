# EmulatorJS Runtime

This directory contains the browser runtime and PCSX-ReARMed core used by the
Zoomies local web-player prototype.

- EmulatorJS version: 4.2.3
- EmulatorJS source: <https://github.com/EmulatorJS/EmulatorJS/tree/v4.2.3>
- PCSX-ReARMed core package: `@emulatorjs/core-pcsx_rearmed@4.2.3`
- RetroArch source: <https://github.com/EmulatorJS/RetroArch>
- EmulatorJS license: GNU General Public License v3.0; see `LICENSE`
- RetroArch license: GNU General Public License v3.0; see `LICENSE`
- PCSX-ReARMed license: GNU General Public License v2.0; see
  `PCSX-REARMED-LICENSE.txt`

Only the non-threaded WebGL2 and legacy PCSX-ReARMed core archives are included.
No PlayStation BIOS is included. The player explicitly uses PCSX-ReARMed's HLE
BIOS implementation.
