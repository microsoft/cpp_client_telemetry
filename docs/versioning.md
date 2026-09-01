# Release and version strategy

The 1DS C/C++ SDK publishes source releases from `main` as needed. Additional
releases may be published for urgent fixes.

## Version format

Release versions use four numeric components:

```text
MAJOR.MINOR.PATCH.BUILD
```

The generator policy is recorded in
[`Solutions/version.txt`](../Solutions/version.txt). At the time of writing it
is:

```text
3.year.day.1
```

Do not derive the next version from old examples in issues, release notes, or
documentation. Run the repository generator:

```powershell
.\tools\gen-version.cmd
```

```bash
./tools/gen-version.sh
```

The scripts generate
[`lib/include/public/Version.hpp`](../lib/include/public/Version.hpp) from its
template. Commit the generated header through a release preparation PR before
tagging the release.

Native SDK releases use a four-component `vX.Y.Z.W` tag. Swift Package Manager
requires three-component semantic versions, so the SPM release workflow derives
and publishes a parallel `X.Y.Z` tag for its binary package.

For the complete operational procedure, downstream package updates, validation,
and recovery guidance, see
[Cutting a release](maintainer-onboarding.md#cutting-a-release).

## API and ABI policy

### C++ API

The C++ API does not provide a general ABI-stability guarantee. Applications
should rebuild against a new SDK version and use compatible compiler, standard
library, runtime, configuration, and struct-packing settings.

Avoid breaking source compatibility within a major release. Any intentional
breaking change requires migration guidance under `docs/` and clear release
notes. Prefer additive APIs and staged deprecation.

### C API

The C API in [`mat.h`](../lib/include/public/mat.h) is the stable binary boundary
for plugins and SDK-in-SDK scenarios. Changes to exported functions, structure
layout, ownership, calling convention, or error behavior require explicit ABI
review and cross-version consumer tests.

See [Sharing a single SDK runtime](sharing-a-single-sdk-runtime.md) before
changing this boundary.

## Release and support policy

- Published releases are source releases; platform products and package feeds
  may produce downstream binaries.
- Release notes must identify compatibility requirements and migration steps.
- Do not move or rewrite a published release tag. Publish a corrected version
  instead.
- Issues are accepted for releases published within the previous 12 months and
  should be reproduced against a currently supported release whenever
  practical.
- Consumers are strongly encouraged to adopt the latest stable SDK release at
  least once every six months rather than relying indefinitely on an old
  snapshot.

Products that require long-lived customizations may maintain a product release
branch or mirror. Generally useful fixes should be contributed back to `main`
so they do not diverge permanently.
