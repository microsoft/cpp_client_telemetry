# Contributing

The 1DS C++ SDK community meets every **Tuesday at 10:30AM PST**. The meeting
invitation is published to [1ds.sdk.cpp](mailto:1ds.sdk.cpp@service.microsoft.com)
team calendar:

* [Join Microsoft Teams Meeting](https://teams.microsoft.com/l/meetup-join/19%3ameeting_ZmQ3MzhlYzMtNWVmNS00MmE3LWE3MTYtMWE1MWUyNmFiZWU5%40thread.v2/0?context=%7b%22Tid%22%3a%2272f988bf-86f1-41af-91ab-2d7cd011db47%22%2c%22Oid%22%3a%2283ba88b7-f89d-4e39-86c5-39927960aca7%22%7d)
* +1(425)616-0754, 63411756#   United States
* Conference ID: 634 117 56#

Meeting agenda and notes are maintained [here](https://microsoft.sharepoint.com/teams/1ds.sdk.cpp/_layouts/15/WopiFrame.aspx?sourcedoc={15b57d3d-8461-4bdc-b5cb-249283e20e5c}&action=edit&wd=target%28Meeting%20Notes.one%7C8cfc64cc-1ae7-44d6-9780-118d8dca1197%2F%29&wdorigin=717).
If you want to propose topics, please append them to the agenda.
To request edit access, please contact [1ds.sdk.cpp@service.microsoft.com](mailto:1ds.sdk.cpp@service.microsoft.com)
or ping us on [Teams](https://teams.microsoft.com/l/channel/19%3a50d8ce341e12455fa3bbfba72b2ba2b5%40thread.skype/General?groupId=d0d4e6fc-48e6-4e38-bb62-bec0fc26b520&tenantId=72f988bf-86f1-41af-91ab-2d7cd011db47).

## Issues and Feature Requests

Issues and feature requests are tracked on [GitHub](https://github.com/microsoft/cpp_client_telemetry/issues).

## Pull Request

### How to Send Pull Requests

Everyone is welcome to contribute code to `1DS C++ SDK` via GitHub
pull requests (PRs).

To create a new PR, fork the project in GitHub and clone the upstream repo:

```sh
$ git clone https://github.com/microsoft/cpp_client_telemetry.git
```

Add your fork as an origin:

```sh
$ git remote add fork https://github.com/YOUR_GITHUB_USERNAME/cpp_client_telemetry.git
```

Check out a new branch, make modifications and push the branch to your fork:

```sh
$ git checkout -b feature_branch_name
# edit files
$ git commit
$ git push fork feature_branch_name
```

Open a pull request against the main `cpp_client_telemetry` repo.

### How to Receive Comments

* If the PR is not ready for review, please put `[WIP]` in the title, tag it
  as `work-in-progress`, or mark it as [`draft`](https://github.blog/2019-02-14-introducing-draft-pull-requests/).
* Make sure CLA is signed and CI is clear.

### How to Get PR Merged

A PR is considered to be **ready to merge** when:
* It has received one approval from Maintainers.
* Major feedbacks are resolved.
* It has been open for review for at least one working day. This gives people
  reasonable time to review.
* Trivial change (typo, cosmetic, doc, etc.) doesn't have to wait for one day.
* Urgent fix can take exception as long as it has been actively communicated.

Any Collaborator/Maintainer can merge the PR once it is **ready to merge**.

## Style Guidelines

[Coding style guidelines](docs/Coding%20style.md)

_Please note that we are rapidly evolving product with many different contributors.
Some modules have been written following platform-specific coding style.
Please try to keep your changes consistent with the coding style of a module you are modifying._

## Become a Collaborator

Collaborators have write access to the repo.

To become a Collaborator:
* Become an active Contributor by working on PRs.
* Actively participate in the community meeting, design discussion, PR review
   and issue discussion.
* Contact the Maintainers, express the willingness and commitment.
* Acknowledged and approved by two Maintainers.

## Become a Maintainer

Maintainers have admin access to the repo.

To become a Maintainer:
* Become a [member of client-telemetry-sdk organization](https://repos.opensource.microsoft.com/microsoft/teams/client-telemetry-sdk/join/).
* Become a Collaborator.
* Demonstrate the ability and commitment.
* Contact the Maintainers, express the willingness and commitment.
* Acknowledged and approved by all the current Maintainers.
