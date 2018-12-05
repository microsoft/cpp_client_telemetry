# Aria C++ V3 / 1DS SDK 
## SDK Information

#### Contact:
* Max Golovanov (**maxgolov@microsoft.com**)
* saahme   (**Sajeed Ahmed**)
* ariaesdks (**ARIA SDK Team**)

#### Branches:

* *master* 
<br> - Should be used only after the' onesdk' build passed and you want your change into master
<br> - Pull Request is needed to push to Master, at least 1 person needs to be added for approval. Please add aria sdk team (**ariaesdks@microsoft.com**)
<br> - Any push will trigger a new build

* *onesdk*
<br> - This should be clean green but if it's red, no problem just go ahead and fix it
<br> - Fell free to push to 'onesdk' once your feature is done
<br> - Any push will trigger a new build

#### Best Practice: 
* User your own branch per feature. Once it can build locally, push it to dev so MSASG vso will build to all supported platforms
* Code practices:
<br> - Follow the code that is written in the file you are modifying.
<br> - Do not refactor code that is already written unless, refactoring is the name of the game
<br> - [Some Guildines](https://blogs.msdn.microsoft.com/brada/2005/01/26/internal-coding-guidelines/)

#### Supported platforms:
* Windows (vs2017)
* Windows (llvm-clang)
* Mac OS X (llvm-clang)
* Linux (gcc, various distros)

#### Deps:
* [Visual Studio](https://www.visualstudio.com/vso/) 2017 or higher
* [Google Test Adapter](https://github.com/csoltenborn/GoogleTestAdapter)

#### Locally
* Install the **Deps**
* run: **build-all.bat** from command line and it will build

## Running tests

#### Dependency:
* Google Test Adapter (Just to run/debug the tests inside Visual Studio)

## Run Tests:

* Run Tests from Visual Studio or from command line:

build-Win32Debug.bat
build-Win32Release.bat
build-x64Debug.bat
build-x64Release.bat

#### Test Result:
* You will see them in Visual Studio
