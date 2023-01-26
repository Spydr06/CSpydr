# cspydr-syntax-highlighting README

This is the Visual Studio Code extension for the CSpydr Programming Language.

Please check out CSpydr on its [official repository](https://github.com/Spydr06/CSpydr) for more information.

## Features

Adds syntax highlighting for the CSpydr programming language to Visual Studio Code

## Installation

1. Installation from repository
 
    1. Create a folder called `CSpydr` in `.vscode-oss/extensions` in your home directory.
    2. Copy the contents of this folder to `.vscode-oss/extensions/CSpydr`
    3. Refresh your VSCode Window or restart VSCode.

    > On UNIX, you can use the `install.sh` script in this directory. Run it in your Terminal with
    > ```
    > $ ./install.sh
    > ```

2. Installation from VSIX
 
    1. Download the `.vsix` file from the [Release Page](https://github.com/Spydr06/CSpydr/releases)
    2. Import the downloaded file into vscode

## Creating the .vsix package

To create a `.vsix` package for this extension, use the `vsce` tool.

1. Install `vsce` from `npm`

```console
$ npm install -g @vscode/vsce
```

2. Create the vsix package

```console
$ vsce package
```
