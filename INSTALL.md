# Installation

Building and installation is done from source.

## Overview
- [Installation](#installation)
  - [Overview](#overview)
  - [Compatibility](#compatibility)
  - [Obtaining CSpydr](#obtaining-cspydr)
  - [Dependencies](#dependencies)
  - [Building](#building)
  - [Installation](#installation-1)
  - [Usage](#usage)

## Compatibility

**Supported Operating Systems:**
- [x] Linux
- [ ] Windows *(in progress)*
- [ ] BSD *(planned)*
- [ ] MacOS

> Note:
> Using CSpydr on Windows via the WSL is possible, but will have some issues with system calls such as `mmap`, causing many programs to segfault.
> The most reliable way to use CSpydr on **any** platform is to use it inside a linux virtual machine.

**Supported C Compilers:**
- [x] `gcc`
- [x] `clang`
- [ ] `msvc` *(in progress)*

**Supported Assemblers and Linkers:**
- [x] GNU `as` and `ld`

## Obtaining CSpydr

CSpydr can be obtained by simply cloning this repository or dowloading the `.zip` file on [GitHub](https://github.com/spydr06/cspydr.git)
```console
$ git clone https://github.com/spydr06/cspydr.git --recursive
```

## Dependencies

CSpydr depends on the following libraries. They need to be installed on your computer for CSpydr to successfully build. Be sure to also install the corresponding `-dev` or `-devel` of library packages for header files if needed.

**Libraries:**
- `glibc` (GNU C standard library, should be bundled with every major distribution)
- `llvm` with `llvm-c` bindings **(optional)**
- `json-c`
- `libpkgconf`

**Programs:**
- `make`
- `gcc` or `clang`
- `as` (GNU assembler (part of the `binutils` package))
- `ld` (GNU linker (part of the `binutils` package))
- `llvm-config` **(optional)**

## Building
Building CSpydr is done via a configure script and Makefiles using the following commands (after cloning this repository):

```console
$ cd ./cspydr
$ ./configure
$ make
```

## Installation

Global installation is necessary, because of the compiler needing the standard library to be present at `/usr/local/share/cspydr/std`.
The install paths can be changed using the `--prefix` and `--exec-prefix` flags on the `./configure` script.
To install CSpydr with all of its components (cspc - The CSpydr Compiler and the CSpydr Standard Library), enter this command (needs root privileges):
```console
# make install
```
Alternatively, you can specify the path of the std library with the `-p` or `--std-path` flags:
```console
$ cspc <your build command> -p ./src/std
```

## Usage

To compile a CSpydr program use the following command:
```console
$ cspc build <your file>
```
To directly run a program use this command:
```console
$ cspc run <your file>
```
To launch a special debug shell, start your program using the `debug` action:
<br/>
*(not finished yet!)*
```console
$ cspc debug <your file>
```

Get help using this command:
```console
$ cspc --help
```
