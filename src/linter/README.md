<div align="center">

# `csp-lint` - The CSpydr Code Linter

</div>

This folder contains the source code of `csp-lint`, the code linter for the CSpydr programming language.

This program is a small utility for error-checking CSpydr programs without running the actual compiler. It also features a **live mode**, which checks a program and all of its files for errors automatically.

## Installation

`csp-lint` will be installed as a part of CSpydr. View [INSTALL.md](../../INSTALL.md) for installation instructions.

## Usage

Linting a CSpydr application once can be done using:

```console
$ csp-lint <your cspydr file>
```

To enable the live mode, use the `-l` or `--live` flags. It enables you to keep `csp-lint` running in a separate terminal to get live error messages as you type the code. No need for constant recompiling.

```console
$ csp-lint <your cspydr file> --live
```

If your standard library path is **not** `/usr/share/cspydr/std`, set the corresponding path with the `-p` or `--std-path` flag.

To use `csp-lint` in a shell script or other form of program, you can use the `-v` or `--verbose` flags for program-friendlier error output. 

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update the unit tests as appropriate.

**View [CONTRIBUTING.md](../../CONTRIBUTING.md) for more information**

## License
`csp-lint` is licensed under the [MIT License](https://mit-license.org/).

> [🔙 Back to main CSpydr README](../../README.md)