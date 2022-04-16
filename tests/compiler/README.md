# The CSpydr test suite

> This directory contains a testing system for **cspc** - the CSpydr Programming Language Compiler. The testing suite is based on the **acutest** testing framework. See `lib/acutest` for further information.

* Tests for individual components of the compiler are located in their respective header files.
<br/>
*e.g.: `test_lexer.h` for `src/compiler/lexer`, `test_preprocessor.h` for `src/compiler/preprocessor`, `test_parser.h` for `src/compiler/parser`*

* Tests for the compiler as a whole *(the executable `bin/cspc`)* are located as `.csp` files in `tests/compiler/files` containing ordinary CSpydr code. These files get automatically compiled and run by the testing system as defined in `test_compiler.h`.
<br/>
If a file contains `# success` in the first line *(mind: whitespaces matter!)*, the test is succeeded when the compiler and the program return 0.
<br/>
If a file contains `# failure` in the first line, the test is succeeded when the compiler or the program fails.