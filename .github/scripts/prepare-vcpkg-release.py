"""Copy the release's registry recipe into a vcpkg checkout."""

import argparse
import json
from pathlib import Path
import re
import shutil


def prepare_port(source_port, destination_port, version, sha512):
    if not re.fullmatch(r"\d+\.\d+\.\d+\.\d+", version):
        raise ValueError("Expected a four-component SDK release version")
    if not re.fullmatch(r"[0-9a-fA-F]{128}", sha512):
        raise ValueError("Expected a 128-character hexadecimal SHA512")
    source_port = source_port.resolve()
    destination_port = destination_port.resolve()
    if destination_port.name != "cpp-client-telemetry" or destination_port.parent.name != "ports":
        raise ValueError("Destination must be a ports/cpp-client-telemetry directory")
    if (
        source_port == destination_port
        or source_port in destination_port.parents
        or destination_port in source_port.parents
    ):
        raise ValueError("Source and destination ports must not overlap")
    vcpkg_root = destination_port.parent.parent
    if (
        not (vcpkg_root / ".vcpkg-root").is_file()
        or not (vcpkg_root / "scripts" / "buildsystems" / "vcpkg.cmake").is_file()
    ):
        raise ValueError(
            "Destination must belong to a vcpkg checkout containing "
            ".vcpkg-root and scripts/buildsystems/vcpkg.cmake"
        )

    manifest = json.loads((source_port / "vcpkg.json").read_text(encoding="utf-8"))
    if manifest["name"] != "cpp-client-telemetry":
        raise ValueError("Release recipe is not the cpp-client-telemetry port")
    required_features = {"device-id", "android-capi-http-client"}
    actual_features = set(manifest.get("features", {}))
    unsupported_features = actual_features - required_features
    if unsupported_features:
        raise ValueError(
            "Release recipe has unsupported features: "
            + ", ".join(sorted(unsupported_features))
        )
    missing_features = required_features - actual_features
    if missing_features:
        raise ValueError(
            "Release recipe is missing required features: "
            + ", ".join(sorted(missing_features))
        )
    manifest["version"] = version
    manifest.pop("port-version", None)

    portfile = (source_port / "portfile.cmake").read_text(encoding="utf-8")
    if "MATSDK_VCPKG_SOURCE_DIR" in portfile:
        raise ValueError("Registry recipes must not allow development source overrides")
    for field, value in (("REF", f"v{version}"), ("SHA512", sha512)):
        portfile, count = re.subn(
            rf"(?m)^([ \t]*{field}[ \t]+)[^\r\n]+",
            lambda match: match[1] + value,
            portfile,
        )
        if count != 1:
            raise ValueError(f"Release portfile must contain exactly one {field}")

    # Replace the complete port so obsolete patches cannot survive a release.
    if destination_port.exists():
        shutil.rmtree(destination_port)
    shutil.copytree(source_port, destination_port)
    (destination_port / "portfile.cmake").write_text(portfile, encoding="utf-8", newline="\n")
    (destination_port / "vcpkg.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8", newline="\n"
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-port", type=Path, required=True)
    parser.add_argument("--destination-port", type=Path, required=True)
    parser.add_argument("--version", required=True)
    parser.add_argument("--sha512", required=True)
    arguments = parser.parse_args()
    prepare_port(
        arguments.source_port,
        arguments.destination_port,
        arguments.version,
        arguments.sha512,
    )
