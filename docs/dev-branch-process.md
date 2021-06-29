# Development Process

## Branches

Naming convention is simple and straightforward.

- **Master (master)** is the default branch available in the Git repository for the `current` release. Current SDK release is `v3.x`. This branch should be stable all the time and won’t allow any direct check-in. You can only merge it after code review. All team members are responsible for keeping the master stable and up-to-date.

- **Development (dev)** is the main development branch for the next major release `v4.x`. The dev branch’s idea is to make changes in it and restrict the developers from making any changes in the master branch directly. Changes in the dev branch undergo reviews. When ready, the branch will be fully promoted to `master`.

## `master` branch renaming to `main` would commence in 2021

It is planned for `v4.x` branch to coincide with renaming of the `master` branch to `main`. Many communities, both on GitHub and in the wider Git community, are considering renaming the default branch name of their repository from `master`. We are also committed to gradually rename the default branch of our repository from `master` to `main`.

## Continuous Integration on `dev` branch

All of the existing tests are enabled on both `master` and `dev` branches. While implementing any refactoring changes, please adjust all tests accordingly.

## Documenting Feature and API changes

Please use Markdown format to document any API changes from v3.x to v4.x in [docs directory](https://github.com/microsoft/cpp_client_telemetry/tree/master/docs).

## Release labels for multiple versions

While the `master` (`main`) branch contains linear history of all releases and associated git tags, it may be practical to branch-off specific releases for use in concrete products.

Sometimes a product may require a feature to be added on top of old long-term supported release.

Example branches to be created as-needed:

- `release/v3.6.123`
- `release/$product/v3.6.123`. For example, `release/edge/m102`.

All feature work made by committers (product owners) on old releases should be carried forward into the new release.
Committers use their best judgement how to plan the forward-porting of their change into latest release.

All feature work made by maintainers on latest release (or `dev` branch) will not be backported to old releases.
Commmitters use their best judgment if they'd like to backport a feature.

In case of a backport of a new feature to old release, `release/$product/v3.x.y` branch would be created.
Old release long-term support branches should never be full-promoted back into `dev` or `master` (`main`).

Continuous Integration is enabled on `release/*` branches.

## Compilers, Toolchains and Language Standard requirements for different releases

It may be necessary for the new release to require a new compiler or language standard. For example, starting with `dev` branch we would only support Visual Studio 2019 and above. `C++14` and above will also be required. It is possible that support for some build configurations will be dropped in the new release. It is also possible that some new features added to `dev` branch may not be backportable to old releases.
