#!/usr/bin/env python3
#
# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
"""Run the production Android HttpClient JNI lifetime tests on a booted emulator."""

import argparse
import os
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", required=True, type=Path)
    parser.add_argument("--library", required=True, type=Path)
    parser.add_argument("--java-home", type=Path, default=os.environ.get("JAVA_HOME"))
    parser.add_argument("--case", default="all")
    parser.add_argument("--stress-count", type=int, default=60000)
    args = parser.parse_args()
    suffix = ".exe" if os.name == "nt" else ""
    adb = args.sdk / "platform-tools" / ("adb" + suffix)
    javac = args.java_home / "bin" / ("javac" + suffix) if args.java_home else "javac"
    build_tools = sorted(
        (args.sdk / "build-tools").iterdir(),
        key=lambda path: tuple(int(part) for part in path.name.split(".")),
    )[-1]
    d8 = build_tools / ("d8.bat" if os.name == "nt" else "d8")

    def run(*command, **kwargs):
        environment = os.environ.copy()
        if args.java_home:
            environment["JAVA_HOME"] = str(args.java_home)
        return subprocess.run(
            [str(arg) for arg in command], check=True, timeout=180, env=environment, **kwargs)

    with tempfile.TemporaryDirectory(prefix="matsdk-jni-") as directory:
        output = Path(directory)
        run(javac, "--release", "8", "-d", output,
            Path(__file__).with_name("HttpClient.java"))
        libraries = ["--lib", args.java_home] if args.java_home else []
        run(d8, "--min-api", "24", *libraries, "--output", output, *output.rglob("*.class"))
        remote = "/data/local/tmp/" + output.name
        run(adb, "shell", "mkdir", "-p", remote)
        try:
            run(adb, "push", output / "classes.dex", remote + "/classes.dex")
            run(adb, "push", args.library.resolve(), remote + "/libandroid-http-client-tests.so")
            run(adb, "shell", "CLASSPATH=" + remote + "/classes.dex",
                "app_process", "-Xcheck:jni", "/system/bin",
                "com.microsoft.applications.events.HttpClient",
                remote + "/libandroid-http-client-tests.so", args.case, args.stress_count)
        finally:
            run(adb, "shell", "rm", "-f", remote + "/classes.dex",
                remote + "/libandroid-http-client-tests.so")
            run(adb, "shell", "rmdir", remote)


if __name__ == "__main__":
    main()
