# The CSpydr Programming Language

CSpydr is a low-level, static typed, compiled programming language inspired by Rust and C building ontop of LLVM. 

## Current Status
A list of all the features, that are/will be implemented.

##### Compiler features:
- [ ] LLVM compiler
- [ ] Assembly compiler
- [x] C transpiler
- [x] lexing symbols
- [x] parsing an AST
- [x] validating the code (happens during parsing)
- [ ] type evaluator
- [ ] type validator
- [x] function validation
- [x] CLI and error handling
- [ ] memory management -> under construction

##### Language features:
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
- [x] `sizeof` keyword (currently called through the C interface)
- [x] different loop types: `for`, `while` and `loop`
- [x] macros (without arguments)
- [ ] namespaces
- [ ] public/private functions, globals, types and struct members
- [ ] functions inside of structs
- [x] lambda expressions (not asynchronous)
- [x] tuples
- [ ] generics in functions and structs

##### Standard library features
- [x] basic `c17` `libc`-header implementation
- [ ] control- and safety-structs and -functions (like in Rust)
- [ ] llvm-c implementation
- [ ] higher-level wrapper-functions and -structs for the `libc` functions

## Installation

Currently, CSpydr is only available for Linux. Once a first major release is in sight I will create an [AUR](https://aur.archlinux.org/) repository for [Arch Linux](https://archlinux.org/) and port it over to [Windows](https://www.microsoft.com/windows), but at the moment Installation is done via [*make*](https://www.gnu.org/software/make/) using [*gcc*](https://gcc.gnu.org/). You also need [*LLVM*](https://llvm.org/docs/GettingStarted.html) to compile CSpydr.
Finally, enter these commands:

```bash
git clone https://github.com/spydr06/cspydr.git --recursive
cd ./cspydr
```
```bash
make
sudo make install
```
This installs the CSpydr compiler with the CSpydr STD (Standard Library)

## Usage

To compile a CSpydr program use the following command:
```bash
cspydr build <your file>
```
To directly run a program use this command:
```bash
cspydr run <your file>
```

Get help using this command:
```bash
cspydr --help
```

## The CSpydr Syntax

A simple hello-world program:

![helloworld](https://github.com/Spydr06/cspydr/blob/main/doc/img/helloworld.csp.png?raw=true)

Running this program is as easy as entering the following command:
```bash
cspydr run hello-world.csp
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
