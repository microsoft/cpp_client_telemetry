"""Exercise the POSIX test runner without compiling or contacting telemetry services."""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[1]


class BuildRunnerTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.calls = self.root / "calls.txt"
        shutil.copyfile(REPO_ROOT / "build-tests.sh", self.root / "build-tests.sh")
        self.write_script(
            "build.sh",
            'printf "build %s\\n" "$*" >> "$CALLS"\n'
            'printf "options %s\\n" "$CMAKE_OPTS" >> "$CALLS"\n'
            'exit "${BUILD_RESULT:-0}"\n',
        )
        self.write_script(
            "bin/ctest",
            'printf "ctest %s\\n" "$*" >> "$CALLS"\n'
            'printf "ctest-cwd %s\\n" "$PWD" >> "$CALLS"\n'
            'exit "${CTEST_RESULT:-0}"\n',
        )
        self.write_script(
            "out/tests/functests/FuncTests",
            'printf "concurrent %s\\n" "$*" >> "$CALLS"\n'
            'if mkdir "$CASE_ROOT/first-child" 2>/dev/null; then\n'
            '  sleep 0.05\n'
            '  echo first-finished >> "$CALLS"\n'
            '  exit "${FIRST_RESULT:-0}"\n'
            'else\n'
            '  sleep 0.1\n'
            '  echo second-finished >> "$CALLS"\n'
            '  exit "${SECOND_RESULT:-0}"\n'
            'fi\n',
        )

    def write_script(self, name, body):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("#!/bin/sh\n" + body, encoding="utf-8")
        path.chmod(0o755)

    def run_runner(self, *arguments, **variables):
        environment = dict(
            os.environ,
            PATH=str(self.root / "bin") + os.pathsep + os.environ["PATH"],
            CALLS=str(self.calls),
            CASE_ROOT=str(self.root),
            BUILD_RESULT="0",
            CTEST_RESULT="0",
            FIRST_RESULT="0",
            SECOND_RESULT="0",
        )
        environment.pop("CMAKE_OPTS", None)
        environment.update(variables)
        result = subprocess.run(
            ["sh", str(self.root / "build-tests.sh"), *arguments],
            cwd=self.temporary.name,
            env=environment,
            capture_output=True,
            text=True,
            timeout=10,
        )
        calls = self.calls.read_text(encoding="utf-8").splitlines()
        return result, calls

    def test_runs_complete_ctest_suite_and_waits_for_both_children(self):
        result, calls = self.run_runner()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("build release", calls)
        self.assertIn("ctest --output-on-failure", calls)
        self.assertIn("ctest-cwd " + str(self.root / "out"), calls)
        self.assertIn("first-finished", calls)
        self.assertIn("second-finished", calls)
        self.assertEqual(sum(line.startswith("concurrent ") for line in calls), 2)
        self.assertTrue(
            all(
                line == "concurrent --gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager"
                for line in calls
                if line.startswith("concurrent ")
            )
        )

    def test_preserves_configuration_and_caller_cmake_options(self):
        result, calls = self.run_runner("debug", CMAKE_OPTS="-DMATSDK_ENABLE_DEVICE_ID=OFF")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("build debug", calls)
        self.assertIn(
            "options -DMATSDK_ENABLE_DEVICE_ID=OFF "
            "-DMATSDK_BUILD_UNIT_TESTS=ON -DMATSDK_BUILD_FUNC_TESTS=ON",
            calls,
        )

    def test_build_failure_stops_before_running_stale_tests(self):
        result, calls = self.run_runner(BUILD_RESULT="23")
        self.assertEqual(result.returncode, 23)
        self.assertFalse(any(line.startswith(("ctest ", "concurrent ")) for line in calls))

    def test_ctest_failure_stops_before_concurrent_checks(self):
        result, calls = self.run_runner(CTEST_RESULT="24")
        self.assertEqual(result.returncode, 24)
        self.assertFalse(any(line.startswith("concurrent ") for line in calls))

    def test_first_child_failure_is_not_lost(self):
        result, calls = self.run_runner(FIRST_RESULT="25")
        self.assertEqual(result.returncode, 25)
        self.assertIn("first-finished", calls)
        self.assertIn("second-finished", calls)

    def test_second_child_failure_is_not_lost(self):
        result, calls = self.run_runner(SECOND_RESULT="26")
        self.assertEqual(result.returncode, 26)
        self.assertIn("first-finished", calls)
        self.assertIn("second-finished", calls)


if __name__ == "__main__":
    unittest.main()
