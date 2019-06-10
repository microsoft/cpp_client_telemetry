---
layout: page
title: Using Fiddler inspector on Mac OS X
sub_title:

---
This tutorial guides you through the process of downloading and configuring Fiddler inspector on Mac OS X in order to see events sent with 1DS C++ Client SDK.

### **Installation instructions for 1DS Events protocol decoder / inspector for Fiddler**

1. Install latest Fiddler. Activate HTTPS traffic decoding. Follow the installer instructions on how to do this.

2. Copy the Fiddler inspector binary with latest CS3.0 protocol (with some CS4.0 fields) and bond protocol support located here: 

	**\\\\ARIA-BUILD-01.REDMOND.CORP.MICROSOFT.COM\\Releases\\Windows\\tools\\Fiddler\\OneDSInspector.zip**
    
    to some directory of your choice.

3. Go to _**%LOCALAPPDATA%\Programs\fiddler-mac\Inspectors**_ and delete de v1 inspector if you have it. The v1 inspector uses an older version of the Bond lib and it will crash if you keep both versions in one directory. Delete the old inspector contents and deploy the contents of the copied .zip file (step 2) to _**%LOCALAPPDATA%\Programs\fiddler-mac\Inspectors**_.

4. Download Mono latest release for Mac OS X from the [official site](https://www.mono-project.com/download/stable/) and follow the installation instructions.

5. Follow Fiddler's guidelines ("Fiddler for OS X Install Instructions") available in the .zip archive from step 2. 

_**Note:** You should follow those instructions in order to deploy moz certificates and ensure the SSL decryption works fine._

### **Running Fiddler on 32-bit mode and reading events**

1. To launch Fiddler on 32-bit mode you need to run: `mono --arch=32 Fiddler.exe`

2. After launching Fiddler on Mac go to Fiddler's proxy settings and make sure both options are checked: _**intercept SSL traffic**_ and _**Decrypt SSL traffic**_.

3. Make sure you click the [OK] button when enabling decryption.

_**Note:** Sometimes Mac OS X has some trouble rendering Fiddler's UI. If there's no [OK] Button showing up you might need to resize the window in order for it to appear._ 

For realtime monitoring of events flow, for one particular process (assuming the defaultp HTTP stack(libcurl) is not customized), the environment variable for libcurl must be set respect to the proxy you set when launching the process:

```
export ALL_PROXY=127.0.0.1:8888
./yourAppName.bin
``

That would redirect all the HTTPS traffic via Fiddler.


Now the events should be decrypted and formated to JSON when clicked on Fiddler:

![Fiddler example Mac](/images/22867-fiddler_example.png)