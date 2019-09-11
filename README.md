# 1DS C/C++ SDK

1DS C/C++ SDK enables cross-platform telemetry collection from various Microsoft products. It enables data / telemetry upload to 1DS Collector++.

# Overview

One Data Strategy (1DS) is a cross-org initiative with five teams across the
company coming together to unify multiple telemetry efforts at Microsoft. This
includes the teams that built Asimov, Aria, Shared Data Platform, Geneva, OMS,
Application Insights, and Visual Studio App Center. We aim to create a unified
data production, collection, and set of tools for key scenarios for the entire
company. Collector++ is the externally-facing destination end-point where
data/telemetry is uploaded to that subsequently routes the data to Microsoft
internal data pipeline.

```
We need a data culture where every engineer, every day, is looking at the
usage data, learning from that usage data, questioning what new things to
test out with our products, and being on that improvement cycle which is the
lifeblood of Microsoft...
                                                              -- Satya Nadella
```

# Getting Started

The SDK is released as a source package. Here are the supported platforms:

| Build Environment                                     | Target Platform |
| ----------------------------------------------------- | --------------- |
| Linux with [GCC](https://gcc.gnu.org/)                |                 |
| Mac OS X with cmake and XCode standard clang compiler |                 |
| Windows with Visual Studio 2017                       |                 |
| Windows with cmake + llvm-clang compiler              |                 |

## Running the tests

There are two sets of tests available:
* 'tests/functests' - functional tests that verify customer-facing APIs and Features
* 'tests/unittests' - internal unit tests that verify operation of each individual component

These tests use Google Test / Google Mock framework and get built alongside with SDK.
Launch 'functests' and 'unittests' binary executables to capture the test results.

### Unit Tests

'unittests' cover various internal component tests, such as LogManager, ILogger, HTTP uploader, IStorage, etc.

### Integration Tests

'functests' include integration tests that verify the flow of data to Collector++.

# Getting Support

We recommend [GitHub issues](https://github.com/microsoft/cpp_client_telemetry/issues)
as the communication channel for both feature requests and issues.

We are also available on email and community meeting, please refer to [CONTRIBUTING.md](CONTRIBUTING.md)
for more details.


# Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for more details.

# Versioning

This library follows [Semantic Versioning](http://semver.org/).

# License

By contributing to OpenTelemetry Specification repository, you agree that
your contributions will be licensed under [MIT License](LICENSE).
