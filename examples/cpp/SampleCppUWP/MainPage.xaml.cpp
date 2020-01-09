//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "MainPage.xaml.h"

using namespace SampleCppUWP;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// Use Telemetry SDK header and namespace
#include <LogManager.hpp>
using namespace MAT;

#include <algorithm>
#include <functional>
#include <string>

#include <Main.hpp>

MainPage^ MainPage::self;

/// <summary>
/// Convert from std::string to PlatformString
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
Platform::String ^ FromString(const std::string & input)
{
    std::wstring w_str = std::wstring(input.begin(), input.end());
    const wchar_t* w_chars = w_str.c_str();
    return (ref new Platform::String(w_chars));
}

/// <summary>
/// Simple function that prints the debug output buffer to MainPage text view control
/// </summary>
std::function<void(const char*)> PrintLine = [](const char* buffer) {
    std::string text(buffer);
    MainPage::Self()->Dispatcher->RunAsync(
        Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([text]()
    {
        MainPage::Self()->Print(FromString(text));
        MainPage::Self()->ScrollToEnd();
    }));
};

/// <summary>
/// Initialize the self singleton (needed for PrintLine above)
/// </summary>
MainPage::MainPage()
{
    InitializeComponent();
    self = this;
}

/// <summary>
/// Run performance test
/// </summary>
/// <param name="sender"></param>
/// <param name="e"></param>
void MainPage::PerfTestClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    // FlushAndTeardown the other instance since perf test would start its own
    TelemetryTeardown();

    btnPerfTest->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    btnSimpleTest->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    Print("Starting performance test...");
    perfTestResult = std::async(std::launch::async, [this]()
    {
        PerformanceTest();
        // Now refresh the UI from context of async thread
        btnPerfTest->Dispatcher->RunAsync(
            Windows::UI::Core::CoreDispatcherPriority::Normal,
            ref new Windows::UI::Core::DispatchedHandler([this]()
        {
            Print("Performance test done.");
            this->btnPerfTest->Visibility = Windows::UI::Xaml::Visibility::Visible;
            this->btnSimpleTest->Visibility = Windows::UI::Xaml::Visibility::Visible;
            // Reinitialize
            TelemetryInitialize();
        }));

    });
}

/**
 * Simple test that does Initialize, LogEvent and FlushAndTeardown
 */
void MainPage::SimpleTestClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    btnPerfTest->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    btnSimpleTest->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    // LogManager::Initialize is called from TelemetryInitialize on app start

    ILogger *logger = LogManager::GetLogger("myModuleLogger");

    EventProperties props("Microsoft.SampleCppUWP.HelloTelemetrySample",
    {
        { "_MSC_VER", _MSC_VER },

        { "piiKind.None",               EventProperty("jackfrost",  PiiKind_None) },
        { "piiKind.DistinguishedName",  EventProperty("/CN=Jack Frost,OU=PIE,DC=REDMOND,DC=COM",  PiiKind_DistinguishedName) },
        { "piiKind.GenericData",        EventProperty("jackfrost",  PiiKind_GenericData) },
        { "piiKind.IPv4Address",        EventProperty("127.0.0.1", PiiKind_IPv4Address) },
        { "piiKind.IPv6Address",        EventProperty("2001:0db8:85a3:0000:0000:8a2e:0370:7334", PiiKind_IPv6Address) },
        { "piiKind.MailSubject",        EventProperty("RE: test",  PiiKind_MailSubject) },
        { "piiKind.PhoneNumber",        EventProperty("+1-613-866-6960", PiiKind_PhoneNumber) },
        { "piiKind.QueryString",        EventProperty("a=1&b=2&c=3", PiiKind_QueryString) },
        { "piiKind.SipAddress",         EventProperty("sip:jackfrost@microsoft.com", PiiKind_SipAddress) },
        { "piiKind.SmtpAddress",        EventProperty("Jack Frost <jackfrost@microsoft.com>", PiiKind_SmtpAddress) },
        { "piiKind.Identity",           EventProperty("Jack Frost", PiiKind_Identity) },
        { "piiKind.Uri",                EventProperty("http://www.microsoft.com", PiiKind_Uri) },
        { "piiKind.Fqdn",               EventProperty("www.microsoft.com", PiiKind_Fqdn) },

        { "strKey",   "hello" },
        { "strKey2",  "hello2" },
        { "int64Key", 1L },
        { "dblKey",   3.14 },
        { "boolKey",  false },
        { "guidKey0", GUID_t("00000000-0000-0000-0000-000000000000") },
        { "guidKey1", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
        { "guidKey3", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
        { "timeKey1",  time_ticks_t((uint64_t)0) },     // ticks   precision
    });
    props.SetPriority(EventPriority_Immediate);
    logger->LogEvent(props);
    Print("HelloTelemetrySample event logged.\n");

    btnPerfTest->Visibility = Windows::UI::Xaml::Visibility::Visible;
    btnSimpleTest->Visibility = Windows::UI::Xaml::Visibility::Visible;
}

void MainPage::Print(Platform::String^ text)
{
    textBlock->Text += text;
}

void MainPage::ScrollToEnd()
{
    _scroll1->UpdateLayout();
    _scroll1->ChangeView(0.0, _scroll1->ScrollableHeight, 1.0f);
}
