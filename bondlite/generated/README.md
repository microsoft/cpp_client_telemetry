This directory contains generated files.

-   **bond\_const.json, DataPackage.json**  
    Generated from .bond files by Bond compiler:

        gbc schema ..\external\bond\core\bond_const.bond
        gbc schema ..\schema\DataPackage.json

    Checked-in to avoid dependency on `gbc` for testing/developing
    `bondjson2cpp.py`.

-   **BondConstTypes.hpp, DataPackage\_\*.hpp**  
    Generated from .json by `bondjson2cpp.py`:

        python ..\bin\bondjson2cpp.py DataPackage.json

    Checked-in to avoid dependency on Python for compiling C++ code.

-   **bond, bond-aria**  
    Directories used only when building with full Bond.

See `README.md` in the parent directory for details.
