# Contributing

Contributions to CSpydr are very welcome. For major changes, please open an issue first to discuss what you would like to change.

Following are instructions, hints and guidelines how contributions should be worked on.

## Code Structure

The code is structured into three main folders:
- **`lib`** contains third party libraries
- **`src`** contains all source code for the compiler, std library and API. It, again, is divided into 4 subdirectories **`api`**, **`compiler`**, **`linter`** and **`std`**, each corresponding to the different parts of CSpydr
- **`tests`** contains unit tests for the compiler and std library

Building CSpydr is done via CMake. You can choose which build system you want CMake to target, but in general, **Unix Makefiles** are the way to go.

When rebuilding CMake, use the **`cmake_regenerate.sh`** script for that, as a simple `cmake .` will sometimes break the generated build files.

Successfully built executables will be put into the **`bin/`** directory. To install the compiler and std library, use **`make install`**

### Compiler

The compiler is split up into many files, each containing code for one task and **one task only**. (e.g. the parser is located in `parser/parser.c`, the optimizer in `optimizer/optimizer.c`. Subdirectories are used to group bigger parts with multiple files together.

### Linter

The linter is a small program made specifically for error checking. It only has a handful of files, so no subdirectories exist.

### API

The api is the bridge between the compiler internals and a simple-to-use library for C, C++ and CSpydr. All of the code for handling library calls are located in **`api.c`**. The public header accessed by other programs is **`include/cspydr.h`**.

### Standard Library

The Standard library (std lib) consists of many CSpydr files, each containing functions and data structures for CSpydr. Third party libraries are grouped together in their corresponding subdirectory (e.g. lua in `lua/`, C std lib in `libc/`).

Each file of the std lib contains the `namespace std`, followed by `namespace <filename>` for functions and constants. Data types, when meant to be used globally, will be put into the `std` namespace directly. (e.g. `std::File`). Functions meant to operate on a data type are also grouped together into a namespace named after the data type.

## Naming Conventions

### For C: 

- functions and local variables use **`snake_case`**
- global variables, constants and enums use **`UPPER_SNAKE_CASE`**
- type definitions use **`PascalCase`** followed by **`_T`**, indicating the presence of `typedef`.

### For CSpydr:

- functions, namespaces and local variables use **`snake_case`**
- constants and enum members use **`UPPER_SNAKE_CASE`**
- global mutable variables also use **`snake_case`**
- data types use `PascalCase`.
- functions and global variables meant to be only used internally are put into a namespace called **`__internal`**
- variables meant to be static variables of functions are put into namespaces called **`__static`**

## Contribution Workflow

This is an example workflow showing how contributions can be done.
Since git is very flexible, there are many ways how this can be achieved.

> (If you don't already have a GitHub account, please create one. Your GitHub username will be referred to later as 'YOUR_GITHUB_USERNAME'. Change it accordingly in the steps below.)

1. Fork https://github.com/spydr06/cspydr using GitHub's interface to your own account. Let's say that the forked repository is at
`https://github.com/YOUR_GITHUB_USERNAME/cspydr`.

2. Clone the main CSpydr repository https://github.com/spydr06/cspydr to a local folder on your computer, say named `cspydr-dev/` (`git clone https://github.com/spydr06/cspydr cspydr-dev`)
3. `cd cspydr-dev`
4. `git remote add pullrequest https://github.com/YOUR_GITHUB_USERNAME/cspydr`
> the remote named `pullrequest` should point to YOUR own forked repo, **not the main CSpydr repository**! 
After this, your local cloned repository is prepared for making pullrequests, and you can just do normal git operations such as:
`git pull` `git status` and so on.

5. When finished with a feature/bugfix/change, you can:
`git checkout -b fix_<your thing>`
   - Don't forget to keep formatting standards before committing
6. `git push pullrequest`  # (NOTE: the `pullrequest` remote was setup on step 4)
7. On GitHub's web interface, go to: https://github.com/spydr06/cspydr/pulls

   Here the UI shows a dialog with a button to make a new pull request based on the new pushed branch.

8. After making your pullrequest (aka, PR), you can continue to work on the branch `fix_<your thing>` ... just do again `git push pullrequest` when you have more commits.

9. If there are merge conflicts, or a branch lags too much behind V's main, you can do the following:

   1. `git pull --rebase origin main` # solve conflicts and do
   `git rebase --continue`
   2. `git push pullrequest -f` # this will overwrite your current remote branch
   with the updated version of your changes.

The point of doing the above steps, is to never directly push to the main CSpydr repository, *only to your own fork*. Since your local `main` branch tracks the
main CSpydr repository's main, then `git checkout main`, as well as
`git pull --rebase origin main` will continue to work as expected


