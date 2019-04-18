This README describes how to build GoLang wrapper around native C++ Telemetry SDK.

This not officially supported Go SDK, rather than a sample showing how to route data from Go to 1DS Collector++

# Installing 3rd party dependencies

## Linux

```
apt-get install golang swig
```

## Mac OS X

```
brew install golang
brew install swig
```

# Building

Build script would auto-detect the OS (Linux or Mac) and compile/link Go .exe for that OS

```
./build.sh
```

# Running

Requires root:

```
./run-tests.sh
```
