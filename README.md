## BShell

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

A lightweight shell written in C++20 for POSIX-compliant operating systems.

## Features

- GNU Readline integration
  - Full command history support
  - Tab completion
- Custom-made command parsing
  - Supports escape characters
  - Supports quotes, single (`'`) and double (`"`)
  - Supports environment variables

### Planned features

- Piping (`|`)
- Redirection (`>`)
- Background execution (`&`)

### Compatibility & Issues

This program is known to compile on Linux and macOS; however, it has only been tested on Linux. 
Windows (& other non-POSIX OS) support is not planned, but PRs are welcomed!

If you run in to any issues when testing, please open an issue with as much detail as possible. 
PRs for issues or new features are also welcomed!

## Installation

### Prerequisites

* Linux (Debian/Ubuntu): `sudo apt install build-essential cmake libreadline-dev`
* MacOS: `brew install cmake readline`

### Building from source

```shell
git clone https://github.com/AvenIsHere/BShell.git
cd BShell
mkdir build && cd build
cmake ..
make
```

## Usage

To use the shell, run the following command from the directory you build it in:
```shell
./BShell
```

## Licence
The source code for this project is licensed under the GNU General Public License v3.0.
