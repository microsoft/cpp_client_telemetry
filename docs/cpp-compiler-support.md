# Modern C++ language and compiler support, and upgrade roadmap

Current modern C++ version used for development is driven from C++ style guidelines across different projects/teams which predominantly use 1DS C++ SDK:

1. [Chromium C++ guidlines](https://chromium-cpp.appspot.com/) : Chromium has default support available for C++11 ( with few exceptions ) and C++14 ( with few exceptions ), and NOT yet support C++17. As per the timelines mentioned in their website, C++17 support won't happen anytime before Mid-2021.

2. [Azure SDK C++ Compiler Support](https://azure.github.io/azure-sdk/cpp_implementation.html#linux) : Azure SDK requires all client applications to compile successfully using GCC-4.8 compiler in order to run on RHEL 7+, Oracle 7+, Oracle Linux 7+, SLES12 SP2+. GCC 4.8.1 supports all major features of C++11 standard with exception of [N2670](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2670.htm) - _Minimal support for GC_. This requirement is driven from end of life cycle for these products. RHEL 7 has maintenance support till 30 June 2024, and SLES12 SP2 till 31st March 2021. The SDK guidelines are in draft, so this may change in future.

3. [Office C++ Guidelines](https://github.com/microsoft/OfficeCppGuidelines/blob/master/CppCoreGuidelines.md) : Office C++ Guidelines cover C++17.

Based on above guidelines, below are the C++ version support guidelines and future roadmap:

1. 1DS C++ SDK code should compile successfully using below compiler versions (with C++11 support):
    - Visual Studio 2017 or later.
    - GNU Compiler Colletion (GCC) version 4.8 or later.
    - Clang 3.4 or later
    - Apple Clang 9.1 or later

        There are places in code still using features `<codecvt>` header which is deprecated in C++17, this would be fixed.

2. **C++11 features usability** : The 1DS SDK is written using C++11 features, and developers are encouraged to use these features as and when needed.

3. **C++14 features usability** : One of the omissisions from C++11 standards, and made available in C++14 standard is support for `std::make_unique`. This is already [backported](https://github.com/microsoft/cpp_client_telemetry/blob/780205d2ea0298e41e82d54a3d203366f051cdf4/lib/utils/Utils.hpp#L28) in 1DS SDK, and hence compiles successfully with C++11 compilers.
If there are any other features which needs to be used, contributions through PRs to backport them for C++11 compiler in 1DS SDK can be done. One of the benefit of supporing C++14 would be to enable use of Guidelines Support Library(GSL) library. We would re-visited this once Azure SDK lifts the requirement for supporting GCC 4.8 compiler. As of now, we can use C++14 features for Windows platform-specific code ( eg Winlnet or WinHTTP client, or other platform bindings), because we are alredy building with either Visual Studio 2017+ or modern clang (Chromium).

4. **C++17 features usability** : C++17 features are not yet supported as per Chromium C++ guideline (see above), and hence 1DS SDK doesn't support using these features. This would be revisited around Mid-2021 once Chromium removes this restrictions. There are plans to backport some of the needed features like [std::variant](https://en.cppreference.com/w/cpp/utility/variant), [std::string_view](https://en.cppreference.com/w/cpp/string/basic_string_view), and [std::visit](https://en.cppreference.com/w/cpp/utility/variant/visit) during this year(2020). For any other feature requirements, contributions through PRs to backport them for C++11 compiler in 1DS SDK can be done. Guidelines for backporting are been discussed in [Issue#557](https://github.com/microsoft/cpp_client_telemetry/issues/557) and would be finalized soon.

5. **C++20 features usability** : As of now, there are no timelines for support of C++20 features. There are plans to backport [std::span](https://en.cppreference.com/w/cpp/container/span) during this year (2020). This can be optionally lifted from Microsoft GSL span (once we start supporting C++17), as it internally uses C++14 features.

6. **End of Support** : As we start supporting builds using C++17 compiler, C++11 build support would be removed, and the backported features from C++14 and C++17 would be cleaned-up.

Summarising roadmap in tabular format:

| C++ Compilers | Currently Support | Start of Support | End of Support |
| --- | --- | -- | -- |
| C++11 | Yes | - | Mid-2021 |
| C++14 | Yes ( Soft support for Windows specific components) | - | Mid-2021 |
| C++17 | No | Mid-2021 | No Plans |
| C++20 | No | No Plans | No Plans |
