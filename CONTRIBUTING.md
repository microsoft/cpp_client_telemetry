---
ms.reviewedAt: 02/20/2019
ms.reviewedBy: maxgolov
---

# Contributing

1DS C++ Client Library is used by many teams in Microsoft and follows the internal open source model. All interested parties can contribute with features, improvements and bug fixes. These guidelines allow easy collaboration during its development across all involved teams and ensure high quality of the resulting code. Before contributing to this project, please review this document for policies and procedures which will ease the contribution and review process for everyone.

If you have questions, please contact [1ds.sdk.cpp](mailto:1ds.sdk.cpp@service.microsoft.com) DL.

This project adopted Inner Source [model](https://oe-documentation.azurewebsites.net/inner-source/index.html)

## Pull Request Process

1. Prepare your work on a separate branch, preferred format of the branch name:

    remotes/origin/YOUR_ALIAS/FEATURE_BRANCH_NAME

2. Before integrating into 'onesdk' or 'master' branch:

    - Make sure all changes follow local [Coding style](docs/Coding%20style.md).

    - Make sure the code builds cleanly on all platforms and there are no failed tests.

3. Create a VSTS pull request for code review at [MS ASG VSO](http://msasg.visualstudio.com).
   Completion of the pull request requires at least one reviewer.

4. Iterate on the changes until all code review comments are addressed.
   You may push commits to your feature branch and VSO would auto-refresh the pull request.

5. Merging your changes:
* Integrate the changes to 'onesdk' by completing your pull request.
* Resolve merge conflicts if necessary.
* Discuss conflicts with your code reviewers if necessary.

## Issues and Feature Requests

Please report issues, feature proposals and feature requests, roadmaps, etc. to:
* [1ds.sdk.cpp DL](mailto:1ds.sdk.cpp@service.microsoft.com)
* [1DS Client SDK Support](mailto:1dsclientsdksupport@microsoft.com)

## Style Guidelines

[Coding style guidelines](docs/Coding%20style.md)

Please note that we are rapidly evolving product with many different contributors.
Some modules have been written following platform-specific coding style.
Please try to keep your changes consistent with the coding style of a module you are modifying.

## License Information

This project has adopted the internal open source model. (c) Microsoft - All Rights Reserved.
We are currently working on contributing this project to open source under MIT license by April 2019.
Please DO NOT SHARE THIS PROJECT SOURCE CODE EXTERNALLY until we satisfy all the legal requirements.
