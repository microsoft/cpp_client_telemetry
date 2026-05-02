#!/bin/sh
cd ${0%/*}
SKU=${1:-release}
SIMULATOR=${2:-iPhone 8}

set -e

./build-ios.sh ${SKU}

cd tests/unittests

xcrun simctl list devices available
echo 'End of xcrun simctl list devices available'

# Resolve simulator UUID from simctl JSON using an exact name match.
# If the same device name exists across multiple iOS runtimes, pick the
# newest runtime and fail if that runtime still contains duplicate matches.
SIM_MATCH=$(
  xcrun simctl list devices available --json | python3 -c '
import json
import re
import sys

simulator_name = sys.argv[1]
devices_by_runtime = json.load(sys.stdin).get("devices", {})
matches = []

for runtime, devices in devices_by_runtime.items():
    if not runtime.startswith("com.apple.CoreSimulator.SimRuntime.iOS-"):
        continue

    match = re.search(r"iOS-(\d+)(?:-(\d+))?$", runtime)
    if match is None:
        continue

    version = tuple(int(part) for part in match.groups(default="0"))
    runtime_label = "iOS " + ".".join(str(part) for part in version)

    for device in devices:
        if device.get("name") == simulator_name and device.get("isAvailable", True):
            matches.append((version, runtime_label, device["udid"]))

if not matches:
    print(f"ERROR: No available simulator found for {simulator_name!r}", file=sys.stderr)
    raise SystemExit(1)

newest_version = max(version for version, _, _ in matches)
newest_matches = [(runtime_label, udid) for version, runtime_label, udid in matches if version == newest_version]

if len(newest_matches) != 1:
    print(f"ERROR: Multiple available simulators found for {simulator_name!r} in newest runtime:", file=sys.stderr)
    for runtime_label, udid in newest_matches:
        print(f"  - {runtime_label}: {udid}", file=sys.stderr)
    raise SystemExit(1)

runtime_label, udid = newest_matches[0]
print(f"{udid}|{runtime_label}")
' "$SIMULATOR"
)
SIM_ID=${SIM_MATCH%%|*}
SIM_RUNTIME=${SIM_MATCH#*|}

if [ -z "$SIM_ID" ]; then
  echo "ERROR: No available simulator found for '$SIMULATOR'"
  exit 1
fi

echo "Using simulator: $SIMULATOR ($SIM_RUNTIME, id=$SIM_ID)"

xcodebuild test -scheme iOSUnitTests -destination "id=$SIM_ID"

cd ../functests
xcodebuild test -scheme iOSFuncTests -destination "id=$SIM_ID"
