"""Exercise SwiftPM binary selection without building the C++ SDK."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]


class SPMPackageTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        shutil.copy2(REPO_ROOT / "Package.swift", self.root)
        metadata = self.root / "tools" / "apple"
        metadata.mkdir(parents=True)
        shutil.copy2(
            REPO_ROOT / "tools" / "apple" / "MATTelemetryAvailability.json", metadata
        )
        self.local = self.root / "build" / "apple"
        self.local.mkdir(parents=True)
        (self.local / "MATTelemetryAvailability.json").write_text(
            json.dumps({
                "diagnosticDataViewer": True,
                "privacyGuard": True,
                "sanitizer": True,
            }),
            encoding="utf-8",
        )

    def manifest(self, use_local=False):
        environment = dict(os.environ)
        environment.pop("MATSDK_SPM_USE_LOCAL_XCFRAMEWORK", None)
        if use_local:
            environment["MATSDK_SPM_USE_LOCAL_XCFRAMEWORK"] = "1"
        result = subprocess.run(
            ["swift", "package", "--package-path", str(self.root), "dump-package"],
            env=environment,
            check=True,
            capture_output=True,
            text=True,
        )
        return {target["name"]: target for target in json.loads(result.stdout)["targets"]}

    def test_default_uses_release_without_local_framework_or_local_modules(self):
        targets = self.manifest()
        binary = targets["MATTelemetry"]
        self.assertRegex(
            binary["url"],
            r"^https://github\.com/microsoft/cpp_client_telemetry/releases/download/"
            r"v\d+\.\d+\.\d+\.\d+/MATTelemetry\.xcframework\.zip$",
        )
        self.assertRegex(binary["checksum"], r"^[0-9a-f]{64}$")
        self.assertIsNone(binary.get("path"))
        self.assertIn("DiagnosticDataViewer.swift", targets["OneDSSwift"]["exclude"])
        self.assertIn("PrivacyGuard.swift", targets["OneDSSwift"]["exclude"])
        self.assertIn("Sanitizer.swift", targets["OneDSSwift"]["exclude"])

    def test_local_opt_in_uses_matching_framework_and_availability(self):
        targets = self.manifest(use_local=True)
        binary = targets["MATTelemetry"]
        self.assertEqual(binary["path"], "build/apple/MATTelemetry.xcframework")
        self.assertIsNone(binary.get("url"))
        self.assertIsNone(binary.get("checksum"))
        self.assertEqual(targets["OneDSSwift"]["exclude"], [])


if __name__ == "__main__":
    unittest.main()
