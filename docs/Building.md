Building
--------

The SDK can be built in two fundamentally different ways – using Skype
toolchain or using plain CMake and platform tools.

### Stand-alone build

-   Make sure that CMake, Visual Studio 2015 and Windows SDK are
    installed
-   Obtain 3rd party libraries (SQLite, Google Test/Mock)
    -   Although this way of building does not depend on any
        Skype-specific libraries, it still needs some other 3rd party
        external ones. They can be built locally beforehand or obtained
        in a binary form from some other source.
    -   The following locations contain the necessary binaries prebuilt
        for Skype:
        -   <https://nexus.skype.net/content/repositories/skype/skype/porting/gmock/gmock17/1.7.0.53/win-i386-vs2015xp-crtdynamic/gmock-lib-1.7.0.53.tgz>
        -   <https://nexus.skype.net/content/repositories/skype/skype/porting/gmock/gmock17/1.7.0.53/gmock-headers-1.7.0.53.tgz>
        -   <https://nexus.skype.net/content/repositories/skype-ivy/skype/client-shared_utility/zlib/master/win-x86_vs2015-crtdynamic-debug/1.2.8.79/zlib-1.2.8.79.tgz>
        -   <https://nexus.skype.net/content/repositories/thirdparty/sqlite/sqlite-win-x86_vs2015-crtdynamic-debug/3.8.7.4.76/sqlite.tgz>
        -   Extract them into folder `sysroot` in the project directory
            (there will be just one `sysroot\include` and one
            `sysroot\lib`, containing mix of files from
            different libraries)
-   Generate Visual Studio project (in project's root directory):
    -   `mkdir Solutions`
    -   `cd Solutions`
    -   `cmake .. -DBUILD_UNIT_TESTS=YES -DBUILD_FUNC_TESTS=YES`
-   Open the project in Visual Studio:
    -   `start AriaSDK.sln`
-   This kind of build is limited to the Win32 PAL.

### Skype builds

The library is a standard "dev\_buildtools" project, so the usual steps
are needed to build it.

-   Make sure Skype Ant and Cygwin are installed
    -   Skype Ant can be downloaded from
        <https://nexus.skype.net/content/repositories/skype/skype/engineering-services/skype-ant/>,
        pick the latest version and extract the `skype-ant` directory to
        this project's root directory
    -   Cygwin (64-bit version recommended) can be obtained from
        <https://cygwin.com/setup-x86_64.exe>, just a basic installation
        is needed (with tools like `tar` or `unzip`).
-   Make sure Nexus credentials are set in `~/build.properties`
    correctly:

        nexus.user=msad_alias
        nexus.passwd=msad_password

-   Have the latest dev\_buildtools
    -   `git clone https://skype.visualstudio.com/SCC/_git/dev_buildtools`
    -   `dev_buildtools` should be in this project's root directory
    -   Pull new changes in case of problems with dependencies etc.
-   In order to fetch dependencies (with Cygwin in path):
    -   `skype-ant\bin\ant -Dqb.buildConfig=win-x86_vs2015-crtdynamic -Dqb.flavor=debug -Dbuild.version=0.0.0.0 cfw.dev-fetch`
-   In order to generate (Visual Studio) project (with Cygwin in path):
    -   `skype-ant\bin\ant -Dqb.buildConfig=win-x86_vs2015-crtdynamic -Dqb.flavor=debug -Dbuild.version=0.0.0.0 "-Dcmake.generator=Visual Studio 14" -Dcustom-cmake-args="-DBUILD_UNIT_TESTS=yes -DBUILD_FUNC_TESTS=yes" cfw.generate`
    -   `start Solutions\AriaSDK.sln` to open in VS
-   Both the previous actions can be also done with the GUI helper
    `dev_buildtools\local\generate-vs-gui.cmd`.
-   The EDIT\_CACHE project can be used to edit build parameters (e.g.
    build version or PAL type) from withing the VS IDE.
-   In order to use this with the Win32 PAL (as opposed to Skype PAL),
    use EDIT\_CACHE to change `PAL_IMPLEMENTATION` from the CMake GUI or
    set it during the `cfw.generate` step right away.


