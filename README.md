# Aria C++ V2 SDK 
## SDK Information

#### Contact:
* Max Golovanov (**maxgolov@microsoft.com**)
* saahme   (**Sajeed Ahmed**)
* ariaesdks (**ARIA SDK Team**)

#### Branches:
* *Master* 
<br> - Should be used only after the dev build passed and you want your change into master
<br> - Pull Request is needed to push to Master, at least 1 person needs to be added for approval. Please add aria sdk team (**ariaesdks@microsoft.com**)
<br> - Any push will trigger a new [build](https://msasg.visualstudio.com/Shared%20Data/Mobile%20Analytics%20-%20Mobile%20SDKs/_build/index?context=allDefinitions&path=%5CAriaSDK%5CC&definitionId=1755&_a=completed)
* *Dev*
<br> - This should be clean green but if it's red, no problem just go ahead and fix it
<br> - Fell free to push to dev once your feature is done. We encourage you to use *build\\* branch first
<br> - Any push will trigger a new [build](https://msasg.visualstudio.com/Shared%20Data/Mobile%20Analytics%20-%20Mobile%20SDKs/_build/index?context=allDefinitions&path=%5CAriaSDK%5CC&definitionId=1399&_a=completed)

* *<yourname\>\\\<feature>*
<br> - Create a new branch for each feature you are working on or you just want to check some changes you made
<br> - Any new branch or change to this branch will not trigger any build

* *build\\\<branch_name>*
<br> - Create a new branch under *build\\* folder if you want an automatic build to be triggered online.
<br> - This allows you to test local changes with no need of an environment setup
<br> - Any push will trigger a new [build]https://msasg.visualstudio.com/Shared%20Data/Mobile%20Analytics%20-%20Mobile%20SDKs/_build/index?context=allDefinitions&path=%5CAriaSDK%5CC&definitionId=1754&_a=completed)

#### Best Practice: 
* User your own branch per feature. Once it can build locally, push it to dev so MSASG vso will build to all supported platforms
* Code practices:
<br> - Follow the code that is written in the file you are modifying.
<br> - Do not refactor code that is already written unless, refactoring is the name of the game
<br> - [Some Guildines](https://blogs.msdn.microsoft.com/brada/2005/01/26/internal-coding-guidelines/)

#### Supported platforms:
* Windows

#### Deps:
* [Visual Studio](https://www.visualstudio.com/vso/) 2015 or higher
* [Google Test Adapter](https://github.com/csoltenborn/GoogleTestAdapter)

## Building
#### On MSASG
* Pushing to **dev** will trigger all [C++ dev](https://msasg.visualstudio.com/Shared%20Data/Mobile%20Analytics%20-%20Mobile%20SDKs/_build/index?context=allDefinitions&path=%5CAriaSDK%5CC&definitionId=1399&_a=completed) build
* Pushing to **master** will trigger only the [C++ master](https://msasg.visualstudio.com/Shared%20Data/Mobile%20Analytics%20-%20Mobile%20SDKs/_build/index?context=allDefinitions&path=%5CAriaSDK%5CC&definitionId=1755&_a=completed) build
* Pushing to **build\\<branch_name>** will trigger only the [C++ features](https://msasg.visualstudio.com/Shared%20Data/Mobile%20Analytics%20-%20Mobile%20SDKs/_build/index?context=allDefinitions&path=%5CAriaSDK%5CC&definitionId=1754&_a=completed) build

#### Locally
* Install the **Deps**
* run: **AriaSDK.sln** solution and build & use Google Test Adapter to run the tests
* run: **build-all.bat** from command line and it will build
* Note: On at least one instance, the location for the windows SDK headers had to be manually specified (e.g. C:\Program Files (x86)\Windows Kits\10\Include\10.x\shared) in the VC++ Include Directories

## Running tests
#### Dependency:
* Google Test Adapter (Just to run/debug the tests inside Visual Studio)

## Run Tests:
* Run Tests from Visual Studio or from command line. See **build-win32Debug.bat** for more information

#### Test Result:
* You will see them in Visual Studio

## Useful Links: 
* [Builds](https://msasg.visualstudio.com/Shared%20Data/Mobile%20Analytics%20-%20Mobile%20SDKs/_build?path=%5CAriaSDK%5CC&_a=allDefinitionss)
* [Code](https://msasg.visualstudio.com/Shared%20Data/Mobile%20Analytics%20-%20Mobile%20SDKs/_git/Aria.SDK.NewC)
* [Dashboard](https://msasg.visualstudio.com/Shared%20Data/Mobile%20Analytics%20-%20Mobile%20SDKs?activeDashboardId=f92e3841-f65a-4cf2-9743-84b55f5a869a)