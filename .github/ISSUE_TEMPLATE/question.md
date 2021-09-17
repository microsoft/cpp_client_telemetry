---
name: Question
about: Create a question to help us improve our knowledgebase and documentation
labels: question
---

# Is your question about how to consume a C++ library?

Please refer to some of these handy resources first:
- [Consuming libraries and components](https://docs.microsoft.com/en-us/cpp/build/adding-references-in-visual-cpp-projects?view=vs-2019)
- [Consuming C++ libraries in UWP applications](https://docs.microsoft.com/en-us/cpp/porting/how-to-use-existing-cpp-code-in-a-universal-windows-platform-app?view=vs-2019)
- [Creating and using a static library](https://docs.microsoft.com/en-us/cpp/build/walkthrough-creating-and-using-a-static-library-cpp?view=vs-2019)
- [How to create a native C++ code nuget package](https://docs.microsoft.com/en-us/nuget/guides/native-packages)
- [Using cmake projects in Visual Studio](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019)

# Is your question about specific 1DS SDK APIs or Features?

Please check if you can find the answer by following these three steps below:

## Step 1. Check our SDK Doxygen documentation

You can generate the SDK HTML classes documentation [using this script](../../tools/gen-docs.cmd).

## Step 2. Check our Markdown documents collection

Documents describing various SDK features are [available here](https://github.com/microsoft/cpp_client_telemetry/tree/master/docs). We are not indexing that knowledge base yet. We will adopt the [docs.microsoft.com](http://docs.microsoft.com/) after 1DS SDK Project becomes Public OSS.

## Step 3. Learn by example

Clone the repo and search for clues in:

- [Examples directory](https://github.com/microsoft/cpp_client_telemetry/tree/main/examples)
- [API Functional tests](../../tests/functests/APITest.cpp)

Both are good resources that showcase most of the SDK features and typical use-cases.

## If you have not found the answer in Doxygen docs, Markdown nor Examples...

Please:
- Describe your scenario.
- Describe your environment: SDK version, platform, OS version, compiler type, etc.
- Describe the focus area: transmission controls, offline storage, backend routing controls, reliability, performance, etc.
- Add any other additional context.

We will review your question during our weekly community meeting or sooner.

---

Additional channels for ad-hoc help:
- **1DS Questions** DL
- **1ds.sdk.cpp** Public Group
- **MS Teams chat**

If you get your answer via those additional ad-hoc support channels, then please consider contributing it back - share your knowledge as docs contribution in a Markdown format in this git repo.
