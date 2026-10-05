"""Regression tests for promoting the release overlay into the registry."""

import importlib.util
import json
from pathlib import Path
import re
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "prepare_vcpkg_release", REPO_ROOT / ".github" / "scripts" / "prepare-vcpkg-release.py"
)
PREPARE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PREPARE)


class ReleasePortTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        root = Path(self.temporary.name)
        self.source = root / "release" / "tools" / "ports" / "cpp-client-telemetry"
        self.source.mkdir(parents=True)
        self.destination = root / "vcpkg" / "ports" / "cpp-client-telemetry"
        self.destination.mkdir(parents=True)
        vcpkg_root = self.destination.parent.parent
        self.checkout_markers = (
            vcpkg_root / ".vcpkg-root",
            vcpkg_root / "scripts" / "buildsystems" / "vcpkg.cmake",
        )
        for marker in self.checkout_markers:
            marker.parent.mkdir(parents=True, exist_ok=True)
            marker.touch()
        (self.destination / "obsolete.patch").write_text("old patch", encoding="utf-8")
        self.manifest = json.loads(
            (REPO_ROOT / "tools" / "ports" / "cpp-client-telemetry" / "vcpkg.json").read_text(
                encoding="utf-8"
            )
        )
        self.manifest["port-version"] = 7
        self.write_manifest()
        self.portfile = (
            REPO_ROOT / "tools" / "ports" / "cpp-client-telemetry" / "portfile.cmake"
        ).read_text(encoding="utf-8")
        (self.source / "portfile.cmake").write_text(self.portfile, encoding="utf-8")

    def write_manifest(self):
        (self.source / "vcpkg.json").write_text(json.dumps(self.manifest), encoding="utf-8")

    def prepare(self):
        PREPARE.prepare_port(self.source, self.destination, "3.10.999.1", "a" * 128)

    def test_promotes_complete_release_port(self):
        (self.source / "release.patch").write_text("new patch", encoding="utf-8")
        self.prepare()
        actual = json.loads((self.destination / "vcpkg.json").read_text(encoding="utf-8"))
        expected = dict(self.manifest, version="3.10.999.1")
        del expected["port-version"]
        self.assertEqual(actual, expected)
        portfile = (self.destination / "portfile.cmake").read_text(encoding="utf-8")
        self.assertIn("REF v3.10.999.1\n", portfile)
        self.assertIn(f"SHA512 {'a' * 128}\n", portfile)
        self.assertIn("-DMATSDK_SQLITE_PROVIDER=${MATSDK_VCPKG_SQLITE_PROVIDER}", portfile)
        self.assertIn("-DMATSDK_ANDROID_HTTP_CLIENT=${MATSDK_ANDROID_HTTP_CLIENT}", portfile)
        self.assertEqual((self.destination / "release.patch").read_text(), "new patch")
        self.assertFalse((self.destination / "obsolete.patch").exists())
        self.assertEqual(
            json.loads((self.source / "vcpkg.json").read_text(encoding="utf-8")), self.manifest
        )
        self.prepare()
        self.assertEqual((self.destination / "portfile.cmake").read_text(), portfile)

    def test_rejects_release_missing_required_features_before_replacing_port(self):
        for feature in ("minimal-sqlite", "android-curl-openssl"):
            with self.subTest(feature=feature):
                definition = self.manifest["features"].pop(feature)
                self.write_manifest()
                with self.assertRaisesRegex(ValueError, feature):
                    self.prepare()
                self.assertTrue((self.destination / "obsolete.patch").exists())
                self.manifest["features"][feature] = definition

    def test_rejects_missing_or_duplicate_archive_fields(self):
        for field in ("REF", "SHA512"):
            for replacement in ("", f"    {field} duplicate\n    {field} duplicate\n"):
                with self.subTest(field=field, replacement=replacement):
                    malformed = re.sub(
                        rf"(?m)^[ \t]*{field}[ \t]+[^\n]*\n", replacement, self.portfile
                    )
                    (self.source / "portfile.cmake").write_text(malformed, encoding="utf-8")
                    with self.assertRaisesRegex(ValueError, field):
                        self.prepare()
                    self.assertTrue((self.destination / "obsolete.patch").exists())

    def test_rejects_invalid_release_metadata(self):
        for version, sha512 in (("v3.10.999.1", "a" * 128), ("3.10.999.1", "invalid")):
            with self.subTest(version=version, sha512=sha512):
                with self.assertRaises(ValueError):
                    PREPARE.prepare_port(self.source, self.destination, version, sha512)
                self.assertTrue((self.destination / "obsolete.patch").exists())

    def test_rejects_destination_outside_the_port(self):
        with self.assertRaisesRegex(ValueError, "Destination"):
            PREPARE.prepare_port(self.source, self.destination.parent, "3.10.999.1", "a" * 128)

    def test_rejects_overlapping_ports(self):
        with self.assertRaisesRegex(ValueError, "overlap"):
            PREPARE.prepare_port(self.source, self.source, "3.10.999.1", "a" * 128)
        self.assertTrue((self.source / "portfile.cmake").exists())

    def test_rejects_lookalike_non_vcpkg_destination_without_deleting_files(self):
        lookalike = Path(self.temporary.name) / "other-project" / "ports" / "cpp-client-telemetry"
        lookalike.mkdir(parents=True)
        sentinel = lookalike / "keep.txt"
        sentinel.write_text("unrelated project", encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "vcpkg checkout"):
            PREPARE.prepare_port(self.source, lookalike, "3.10.999.1", "a" * 128)
        self.assertEqual(sentinel.read_text(encoding="utf-8"), "unrelated project")
        self.assertEqual(list(lookalike.iterdir()), [sentinel])

    def test_requires_both_checkout_markers_before_replacing_port(self):
        for marker in self.checkout_markers:
            with self.subTest(marker=marker.name):
                marker.unlink()
                with self.assertRaisesRegex(ValueError, "vcpkg checkout"):
                    self.prepare()
                self.assertEqual(
                    (self.destination / "obsolete.patch").read_text(encoding="utf-8"), "old patch"
                )
                marker.touch()

    def test_validates_and_replaces_the_resolved_destination(self):
        alias = self.destination / ".." / "cpp-client-telemetry"
        PREPARE.prepare_port(self.source, alias, "3.10.999.1", "a" * 128)
        self.assertFalse((self.destination / "obsolete.patch").exists())
        self.assertEqual(
            json.loads((self.destination / "vcpkg.json").read_text(encoding="utf-8"))["version"],
            "3.10.999.1",
        )


if __name__ == "__main__":
    unittest.main()
