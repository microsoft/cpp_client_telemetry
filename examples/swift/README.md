# SwiftWrapperApp

Contains a simple xcode project to import and showcase swift wrappers package.

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
