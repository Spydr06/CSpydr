# The CSpydr Programming Language

CSpydr is a low-level, static typed, compiled programming language inspired by Rust and C. It is not meant to replace C but rather work with it, since it is based on C. 

## Installation

Currently, CSpydr is only available for Linux. Once a first major release is in sight I will create an AUR repository for Arch Linux.
But at the moment Installation is done via *make* and *gcc*. You also need [llvm](https://llvm.org/docs/GettingStarted.html) to compile CSpydr.
Finally, enter these commands:

```bash
git clone https://github.com/spydr06/cspydr.git
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
cspydr -t foo.csp -o foo.out
```

Get help using this command:
```bash
cspydr --help
```

## The CSpydr Syntax

A simple hello-world program:
```
# hello-world.csp

fn main(argc: i32, argv: *str): i32
{
    printf("Hello World!");         # at the moment, this calls stdio.h directly through C. I will write my own implementation once the code features are done.
    <- 0;
}
```
Compiling this program is as easy as entering the following command:
```bash
cspydr -t hello-world.csp
```
Then execute it:
```bash
./a.out
```

*I will write a proper documentation in the future.*

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update the unit tests as appropriate.

## License
CSpydr is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html)