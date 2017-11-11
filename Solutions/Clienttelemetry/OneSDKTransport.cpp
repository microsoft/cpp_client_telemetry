#if 0
#pragma warning( disable : 4189 4100)

#include <algorithm>
#include <map>
#include <ctime>
#include <vcclr.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdint.h>
#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Text;

#include <string>

#define ARIA_SDK_VERSION "1.0"

String^ ToPlatformString(std::string s)
{
    return gcnew String(s.c_str());
}

bool PostData(String^ url, String^ data)
{
    System::Net::HttpWebRequest ^request = safe_cast<System::Net::HttpWebRequest^>(System::Net::HttpWebRequest::Create(url));
    request->Method = "POST";
    request->Date = DateTime::UtcNow;
    request->ServicePoint->Expect100Continue = false;

    request->Accept = "application/json, text/javascript, */*; q=0.01";
    request->KeepAlive = true;
    request->ContentLength = data->Length;
    request->ContentType = "application/json";
    request->UserAgent = ToPlatformString(ARIA_SDK_VERSION);

    request->Headers->Add("Accept-Encoding", "gzip,deflate,sdch");
    request->Headers->Add("Accept-Language", "en-US;q=0.6,en;q=0.4");
    request->Headers->Add("X-Requested-With", "XMLHttpRequest");

    System::IO::Stream ^stream = request->GetRequestStream();

    UTF8Encoding^ utf8 = gcnew UTF8Encoding;
    auto text = utf8->UTF8->GetBytes(data);
    stream->Write(text, 0, text->Length);
    stream->Flush();

    auto response = request->GetResponse();
    System::Console::WriteLine(response->ToString());

    response->Close();
    stream->Close();

    return true;
}

extern "C" void SendAsJSON(void *ptr)
{
    /*
                    for (auto &kv : values)
                    {
                        String^ key = ToPlatformString(kv.first.c_str());
                        String^ val = ToPlatformString(kv.second.c_str());
                        System::Console::WriteLine(key + "=" + val + "\n");
                    }
    */
    // PostData(ToPlatformString(ARIA_COLLECTOR_URL "?" ARIA_COLLECTOR_PARAMS), payload);
};
#endif