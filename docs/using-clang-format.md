# Using clang-format on 1DS C++ SDK Code

## Command-line use

To format a file according to [1DS C++ SDK coding style](Coding%20style.md), from the command line.

Setup the build tools environment first.

For Windows - cmd.exe command:

```call tools\setup-devenv.cmd```

For POSIX (Linux and Mac) - shell command:

```source tools/setup-devenv.sh```

Command will add the tools from repo *tools* directory to PATH environment variable.

Then run:

```git cl format <filename>```

At the moment the tool requires to specify the file path. Uber goal is to integrate the toolset
from Chromium to automagically apply clang-format on all files in a current pending commit.

## Editor integrations

For further guidance on editor integration, see these specific pages:

* [Download link for LLVM tools for Windows](https://releases.llvm.org/9.0.0/LLVM-9.0.0-win64.exe)
* [LLVM tools extension for Visual Studio](https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.llvm-toolchain)
* [Visual Studio code extension](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format)
* [CppStyle Eclipse CDT extension](https://marketplace.eclipse.org/content/cppstyle)

## Are robots taking over my freedom to choose where newlines go

No. For the project as a whole, using clang-format is just one optional way to format your code.
While it will produce style-guide conformant code, other formattings would also satisfy the style
guide. For certain modules it may be appropriate to use alternate coding style. In those scenarios
a local directory *.clang-format* settings file takes precedence over the one at top-level.
