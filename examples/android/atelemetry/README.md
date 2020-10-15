# How to start client-server

Scripts below are for Windows. But the same process works on Linux and Mac.

1. Start emulator:

```console
set ANDROID_VERBOSE=socket,gles
set "PATH=C:\Android\android-sdk-30\emulator;%PATH%"
emulator.exe @generic_x86_64_API_29 -selinux permissive -shell

```

2. Build the code and install it:

```console
build.cmd
install.cmd
```

3. Start Telemetry Agent - server first

```console
server.cmd
```

4. Start Telemetry Client next

```console
client.cmd
```

5. Use `adb logcat` to observe client telemetry emitted over Binder IPC channel to land in server (Telemetry Agent) process.

