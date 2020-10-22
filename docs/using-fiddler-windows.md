# Using Fiddler Traffic inspector on Windows

This tutorial guides you through the process of downloading and configuring Fiddler inspector on Windows in order to see events sent with 1DS C++ Client SDK.

## **Installation instructions for 1DS Events protocol decoder / inspector for Fiddler**

### 1. Install latest Fiddler. Activate HTTPS traffic decoding. Follow the installer instructions on how to do this

### 2. Copy the Fiddler inspector binary with latest CS3.0 protocol (with some CS4.0 fields) and bond protocol support located here

[Fiddler Inspector](https://github.com/microsoft/cpp_client_telemetry/releases/download/v3.4.226/1DS_Fiddler_Inspector.zip)

### 3. Extract the contents of zip file above to _**%AppData%\Local\Programs\Fiddler\Inspectors**_

## **Running Fiddler and reading events**

### 1. Launch Fiddler on Windows

### 2. Go to Fiddler's proxy settings on: Tools > Options > HTTPS and make sure both options are checked: _**Capture HTTPS CONNECTs**_ and _**Decrypt HTTPS traffic**_

### 3. Make sure you click the [OK] button when enabling decryption

That would redirect all the HTTPS traffic via Fiddler

Now the events should be decrypted and formated to JSON when clicked on Fiddler.

![Fiddler example Windows](/docs/images/fiddlerWindows.png)
