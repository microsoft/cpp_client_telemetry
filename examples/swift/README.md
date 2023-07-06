# SampleXcodeApp

Contains a simple Xcode project to import and showcase swift wrappers package.

Details:
- Imports
    - OneDSSwift: Swift Wrapper package

- Settings
    - Include module.modulemap folder in Swift compiler search path.
    - Add OneDSSwift package in build phases dependency.

- Libraries/Frameworks to link to Target
    - OneDSSwift # Package
    - SystemConfiguration
    - Network
    # /usr/local/lib
    - libmat.a
    - libz.tbd
    - libsqlite3.a


# SamplePackage

Contains a simple swift package importing swift wrappers package and calling 1DS API via swift wrappers.

Details:
- Package Dependencies:
    - OneDSSwift: Package containing swift wrappers

- Modules Included:
    - ObjCModule: Module exposing ObjC headers via module.modulemap file.
