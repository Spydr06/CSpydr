# The CSpydr API

This folder contains an API for the CSpydr compiler `cspc` to control it from C/C++ or CSpydr
You can fild examples on how to use it in the `examples/` directory.

Compiling an example requires `libcspydr.so` as well as `cspydr.h` to be installed on your computer. To install
the library, run *(needs root privileges)*:
```console
# cmake . && make install
```
Once installed, you can compile an example by linking against it:
```console
$ gcc ./example.c -lcspydr -o example
```

Using `libcspydr` from CSpydr itself is as easy as importing the `cspydr.csp` file in your code:
```cspydr
import "csp/cspydr.csp"
```