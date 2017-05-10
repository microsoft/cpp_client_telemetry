Contributing
============

The Aria C++ Client Library is used by many teams in Microsoft and
follows the internal open source model, so that all of them can
contribute with features, improvements and bug fixes.

The following guidelines allow easy collaboration during its development
across all involved teams and ensure high quality of the resulting code.

1.  Prepare your work on a separate branch.
2.  Before integrating:
    -   Make sure all changes follow local [Coding
        style](Coding%20style.md).
    -   Make sure the code builds cleanly on all platforms and with all
        settings and there are no failed tests.
        -   Can be easily accomplished by running the
            [ng (dynamic)](https://quickbuild.skype.net/overview/316786)
            configuration on QuickBuild, it will ask for a branch name
            to build.
        -   The config automatically builds the code on all platforms,
            runs unit and functional tests (also under AppVerifier and
            OACR checker for SDL compliance) and validates coding style.

3.  Create a VSTS pull request for code review.
    -   Add link to the successful "ng (dynamic)" build as a comment.
    -   If possible, post about the pending code review also to the
        ["Aria C++ SDK rewrite" Skype group
        chat](https://join.skype.com/fsg9c6oo7aPV), so that wider
        audience is notified about the opportunity.

4.  Iterate on the changes until all code review comments are addressed.
    -   Do not add extra commits like "code review fixes".
    -   Add any fixes to the previous relevant commits through Git amend
        or interactive rebase commands and force push the branch.
    -   Perform the build check again (see 2.).

5.  Integrate the changes to `master` using rebase/fast-forward
    merge strategy.
    -   There should be basically no merge commits on the main branch.


