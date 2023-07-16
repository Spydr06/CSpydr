# cspydr.lua README

This is the neovim plugin for the CSpydr Programming Language.

Please check out CSpydr on its [official repository](https://github.com/Spydr06/CSpydr) for more information.

## Features

Adds syntax highlighting for the CSpydr programming language to NeoVim

## Installing

Please use the bundeled `install.sh` script.

```console
$ ./install.sh
```

Alternatively, you can copy the `cspydr.lua` and `syntax/cspydr.vim` files individually.

Be sure to include the following snippet in your `init.lua` file.
```lua
require('cspydr').setup()
```