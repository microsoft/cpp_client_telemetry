# Microsoft Applications Telemetry C/C++ SDK

Microsoft Applications Telemetry C/C++ SDK enables cross-platform telemetry collection from various Microsoft products.
It enables data / telemetry upload to 1DS Collector++. 

# Data/Telemetry

This project collects usage data and sends it to Microsoft to help improve our products and services.
Read our privacy statement to learn more.

## What is 1DS and Collector++?

One Data Strategy (1DS) is a cross-org initiative with five teams across the company coming together to unify multiple
telemetry efforts at Microsoft. This includes the teams that built Asimov, Aria, Shared Data Platform, Geneva, OMS, App
Insights, and Visual Studio App Center. We aim to create a unified data production, collection, and set of tools for key
scenarios for the entire company. Collector++ is the externally-facing destination end-point where data/telemetry is
uploaded to that subsequently routes the data to Microsoft internal data pipeline.


```
 We need a data culture where every engineer, every day, is looking at the usage data, learning from that usage data,
 questioning what new things to test out with our products, and being on that improvement cycle which is the lifeblood
 of Microsoft...
         -- Satya Nadella
```

# Contributing

This project welcomes contributions and suggestions. Most contributions require you to agree to a Contributor License
Agreement (CLA) declaring that you have the right to, and actually do, grant us the rights to use your contribution.
For details, visit [https://cla.microsoft.com] (https://cla.microsoft.com). When you submit a pull request, a CLA-bot
will automatically determine whether you need to provide a CLA and decorate the PR appropriately (e.g., label, comment).
Simply follow the instructions provided by the bot. You will only need to do this once across all repos using our CLA.
This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ
or contact opencode@microsoft.com with any additional questions or comments.

Please read our [CONTRIBUTING.md](CONTRIBUTING.md) which outlines all of our policies, procedures, and requirements for
contributing to this project.

## Getting Started

Please follow instructions published at our [Team SharePoint site](https://microsoft.sharepoint.com/teams/1ds.sdk.cpp/SitePages/Home.aspx)
These instructions will get you a copy of the project up and running on your local machine for development and testing
purposes.

## Running the tests

There are two sets of tests available:
* 'tests/functests' - functional tests that verify customer-facing APIs and Features
* 'tests/unittests' - internal unit tests that verify operation of each individual component

These tests use Google Test / Google Mock framework and get built alongside with SDK.
Launch 'functests' and 'unittests' binary executables to capture the test results.

### End-to-end tests

'functests' include several E2E tests that verify the flow of data to Collector++.

### Unit tests

'unittests' cover various internal component tests, such as LogManager, ILogger, HTTP uploader, IStorage, etc.

## Deployment

SDK is integrated as a static or dynamic library runing in-proc within your executable on Windows, Linux and Mac.

## Built With

Please follow `Getting started` instructions to learn how to build using various compilers.
We support building on:
* Windows with Visual Studio 2017
* Windows with cmake + llvm-clang compiler
* Linux with gcc
* 

* Link to some dependency manager
* Link to some framework or build tool
* Link to some compiler, linting tool, bundler, etc

## Contributing

Please read our [CONTRIBUTING.md](CONTRIBUTING.md) which outlines all of our policies, procedures, and requirements for contributing to this project.

## Versioning and changelog

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](link-to-tags-or-other-release-location).

It is a good practice to keep `CHANGELOG.md` file in repository that can be updated as part of a pull request.

## Authors

List main authors of this project with a couple of words about their contribution.

Also insert a link to the `owners.txt` file if it exists as well as any other dashboard or other resources that lists all contributors to the project.

## License

This project is licensed under the < INSERT LICENSE NAME > - see the [LICENSE](LICENSE) file for details

## Acknowledgments

* Hat tip to anyone whose code was used
* Inspiration