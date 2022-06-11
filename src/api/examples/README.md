# The CSpydr API (Examples)

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
The compiler will automatically figure out the correct library to link with, as long as it's installed. Because of this, running a CSpydr example is as easy as running:
```console
$ cspc run ./example.csp
```