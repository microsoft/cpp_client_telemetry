#include "pch.h"
#include "Version.hpp"

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;

//
// General Information about an assembly is controlled through the following
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
//
[assembly:AssemblyTitleAttribute(L"Microsoft.Applications.Telemetry.Windows")];
[assembly:AssemblyDescriptionAttribute(L"Microsoft data file")];
[assembly:AssemblyConfigurationAttribute(L"Retail")];
[assembly:AssemblyCompanyAttribute(L"Microsoft Corporation")];
[assembly:AssemblyProductAttribute(L"Microsoft Telemetry SDK for Windows")];
[assembly:AssemblyCopyrightAttribute(L"Copyright (c) Microsoft Corporation. All rights reserved.")];
[assembly:AssemblyTrademarkAttribute(L"Microsoft Telemetry SDK for Windows")];
[assembly:AssemblyCultureAttribute(L"")];
[assembly:AssemblyKeyFile("Test.snk")];
// [assembly:AssemblyDelaySign(true)];

//
// Version information for an assembly consists of the following four values:
//
//      Major Version
//      Minor Version
//      Build Number
//      Revision
//
// You can specify all the value or you can default the Revision and Build Numbers
// by using the '*' as shown below:

[assembly:AssemblyFileVersion(BUILD_VERSION_STR)];
[assembly:AssemblyVersionAttribute(BUILD_VERSION_STR)];

[assembly:ComVisible(false)];

[assembly:CLSCompliantAttribute(true)];