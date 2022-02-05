# The CSpydr Programming Language

CSpydr is a low-level, static typed, compiled programming language inspired by Rust and C building ontop of LLVM. 

## Current Status

<div align="center">

![Made with](https://img.shields.io/badge/made%20with-C-123456?style=flat)
[![Stars](https://img.shields.io/github/stars/spydr06/cspydr?style=flat)](https://github.com/Spydr06/CSpydr/stargazers)
[![Forks](https://img.shields.io/github/forks/spydr06/cspydr?style=flat)](https://github.com/Spydr06/CSpydr/network/members)
[![License](https://img.shields.io/github/license/spydr06/cspydr?style=flat)](https://github.com/Spydr06/CSpydr/blob/main/LICENSE)
[![Issues](https://img.shields.io/github/issues/spydr06/cspydr?style=flat)](https://github.com/Spydr06/CSpydr/issues)
[![Build](https://img.shields.io/badge/build-success-success?style=flat)](https://github.com/Spydr06/CSpydr/releases)

</div>

A list of all the features, that are/will be implemented.

##### cspc Compiler features:
- [x] Assembly compiler (only missing: va_lists, returnign of large structs)
- [ ] move to an intermediate bytecode compiler
- [x] C transpiler
- [x] lexing symbols
- [x] parsing an AST
- [x] validating the code (happens during parsing)
- [x] type evaluator
- [x] type validator
- [x] function validation
- [x] CLI and error handling
- [x] memory management

##### CSpydr Language features:
- [x] functions
- [x] function arguments 
- [x] global/local variables
- [x] structs/enums
- [x] typedefs
- [x] control statements
- [x] expressions
- [x] arrays
- [x] file imports
- [x] `extern` functions and globals
- [x] `sizeof` keyword
- [x] `typeof` data type
- [x] different loop types: `for`, `while` and `loop`
- [x] macros
- [x] namespaces
- [x] public functions, globals, types and struct members
- [ ] functions inside of structs
- [x] lambda expressions (not asynchronous)
- [x] tuples
- [ ] generics in functions and structs
- [ ] templates in fuctions and structs

##### CSpydr Standard library features
- [x] basic `c17` `libc`-header implementation
- [ ] from-the-ground custom written `stdlib` basing on linux syscalls
- [ ] control- and safety-structs and -functions (like in Rust)
- [ ] higher-level wrapper-functions and -structs for the `libc` functions

## Installation

Currently, CSpydr is only available for Linux. Once a first major release is in sight I will create an [AUR](https://aur.archlinux.org/) repository for [Arch Linux](https://archlinux.org/) and port it over to [Windows](https://www.microsoft.com/windows), but at the moment Installation is done via CMake.
Finally, enter these commands:

```bash
git clone https://github.com/spydr06/cspydr.git --recursive
cd ./cspydr
```
```bash
cmake .
make
```

To install CSpydr with all of it's components (cspc - The CSpydr Compiler and the CSpydr Standard Library), enter this command (as root):
```bash
sudo make install
```

## Usage

To compile a CSpydr program use the following command:
```bash
cspc build <your file>
```
To directly run a program use this command:
```bash
cspc run <your file>
```

Get help using this command:
```bash
cspc --help
```

## The CSpydr Syntax

A simple [hello-world](https://github.com/Spydr06/CSpydr/blob/main/doc/src/helloworld.csp) program:

```cspydr
# helloworld.csp
import "libc/stdio.csp";

fn main(argv: i32, argc: &&char): i32
{
    let message = "Hello, World!";
    puts(message);

    <- 0;
}
```

Running this program is as easy as entering the following command:
```bash
cspc run hello-world.csp
```

*(I will write a proper documentation in the future!)*

## Editor support

CSpydr currently only supports Visual Studio Code, since thats the code editor I personally use for developing. I will add support for other editors later on.

Check out the Visual Studio Code extension [here](https://github.com/spydr06/cspydr-vscode-extension).

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update the unit tests as appropriate.

## License
CSpydr is licensed under the [MIT License](https://mit-license.org/)

## Resources | Reference | Inspiration

- chibicc C compiler: https://github.com/rui314/chibicc.git
- tac programming language: https://github.com/sebbekarlsson/tac.git
- summus programming language: https://github.com/igor84/summus.git
- porth programming language: https://gitlab.com/tsoding/porth.git
