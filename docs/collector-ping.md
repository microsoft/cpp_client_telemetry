Aria SDK Internet connectivity check.<br>Collector Reachability check.
======================================================================

**Solution 1. Win32 Desktop: DNS probing**

**DnsQuery** API**:**
<https://msdn.microsoft.com/en-us/library/windows/desktop/ms682016%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396>

**Usage Example:**
<https://support.microsoft.com/en-us/help/831226/how-to-use-the-dnsquery-function-to-resolve-host-names-and-host-addres>

Above API allows to exercise DNS probe directly.

**Solution 2. Win32 Desktop: WinInet enum - status code for DNS failure**

Alternate solution is to listen to WinInet status codes / state callbacks. One
of the callback states reports if HTTPS connection DNS resolution passed. This
would require a custom callback status code in HTTP stack implementation. If
connectivity fails, then HTTP Client Manager code should not try to perform
exponential back-off.

**Solution 3. Network Awareness API (cross-platform!) – DNS probe and NCSI HTTP
GET check**

NCSI is designed to be responsive to network conditions, so it examines the
connectivity of a network in a variety of ways. For example, NCSI tests
connectivity by trying to connect to: <http://www.msftncsi.com> - a simple Web
site that exists only to support the functionality of NCSI.

**How does it work?**

Windows OS does indeed check a Microsoft site for connectivity, using the
Network Connectivity Status Indicator site. There are a few variations of the
connection checking process:  
- NCSI performs a DNS lookup on [www.msftncsi.com](http://www.msftncsi.com)  
- Then requests **GET** <http://www.msftncsi.com/ncsi.txt>

This file is a plain-text file and contains only the text Microsoft NCSI. NCSI
sends a DNS lookup request for dns.msftncsi.com. This DNS address should resolve
to **131.107.255.255**. If the address does not match, then it is assumed that
the internet connection is not functioning correctly. Any platform, including
Windows, Android, Mac OS X / iOS (Objective-C) may utilize this end-point to
actively probe for internet connectivity. Windows Desktop may utilize an
existing implementation of NCSI from **Connect.dll** (Desktop apps only):
**IsInternetConnected** . Currently Aria SDK does not use this function and it
uses more lightweight WinInet.dll internet connectivity check instead.

**IsInternetConnected** API**:**
<https://msdn.microsoft.com/en-us/library/windows/desktop/aa366143%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396>

However, the above function **IsInternetConnected** - is not available for
Universal apps (Win10, Xbox, Win10 Phone). Universal apps must implement their
own DNS probe and HTTP GET request.

**Windows 10 UWP status codes for Solutions \#2 and \#3**

**HttpStatusCode** enum:
<https://docs.microsoft.com/en-us/uwp/api/windows.web.http.httpstatuscode>

Several **HttpStatusCode** enum values could be used to detect the connectivity
state:

**BadGateway** - An intermediate proxy server received a bad response from
another proxy or the origin server.  
**GatewayTimeout** - An intermediate proxy server timed out while waiting for a
response from another proxy or the origin server.  
**None** - The client request wasn't successful.  
**ProxyAuthenticationRequired** - The requested proxy requires authentication.
The Proxy-Authenticate header contains the details of how to perform the
authentication.  
**RequestTimeout** - The client did not send a request within the time the
server was expecting the request.

Windows 10 HTTP client for Windows Store apps can be used to actively probe the
same NCSI host as what’s performed by **IsInternetConnected** API in Desktop
apps.

**Linux C/C++ implementation options for Solution \#1**

res_query: <https://linux.die.net/man/3/res_query>

**len = res_query(host, C_IN, T_MX, &answer, sizeof(answer));**

Linux **res_query** can be used to check if **NCSI host** DNS completes
successfully and the result matches the hardcoded IP address of NCSI. Another
secondary step would be to utilize Aria SDK HTTP stack to actively probe and
compare the result of fetching ncsi.txt . **res_query** can also be used to
verify if Collector URL hostname resolves to an IP address.

**Java API implementation options for Solution \#1**

**getAllByName:**
<https://developer.android.com/reference/java/net/InetAddress.html#getAllByName(java.lang.String)>

**checkConnect:**
[https://developer.android.com/reference/java/lang/SecurityManager.html\#checkConnect(java.lang.String,
int)](https://developer.android.com/reference/java/lang/SecurityManager.html%23checkConnect(java.lang.String,%20int))

Same approach as for Windows and Linux: perform DNS resolution and GET request
to <http://www.msftncsi.com/ncsi.txt> , to verify that the internet is
available.

**Objective-C API implementation options for NCSI reachability check (Solution
\#1)**

**SCNetworkReachability**:
<https://developer.apple.com/documentation/systemconfiguration/scnetworkreachability-g7d>

**SCNetworkReachability** programming interface allows an application to
determine the status of a system's current network configuration and the
reachability of a target host. A remote host is considered reachable when a data
packet, sent by an application into the network stack, can leave the local
device. Reachability does not guarantee that the data packet will actually be
received by the host.

**SCNetworkReachability** can be used to ping Microsoft NCSI host. OR
alternatively - native iOS HTTP stack can be used in attempt to **GET**
<http://www.msftncsi.com/ncsi.txt> , essentially implementing the same internet
connectivity check as what Windows **IsInternetConnected** function is doing
(Solution \#3).

**Retry policy adjustments**

There are two non-mutually-exclusive approaches for DNS probing + connectivity
check:

-   **Active internet connectivity check – HTTP-'ping' Microsoft NCSI
    infrastructure end-point**

-   **No active internet probing, but rely on Aria collector HTTP stack status
    codes**

A separate SDK network state should be added to common properties NetworkState
enum:

\- **offline** - internet is not connected / NCSI probe failed or not performed
yet.

\- **online** - internet connected. NCSI probe passed. Does not necessarily mean
Aria collector is reachable!

\- **connected –** device is online. And so far - HTTP posts to collector have
been successful.

Diagram below explains the combined set of reachability checks:  


![](media/079f4688a4125a212ae99beb47e89e27.png)

Device is **offline**. Perform DNS probe of Microsoft NCSI host. If DNS query
passed with expected outcome, then perform HTTP GET query to NCSI host to
confirm the network connectivity. Verify that the content returned by NCSI
server – matches the expected text: “**Microsoft NCSI**”. If matches, then
device is **online**.

Device is **online**. Assuming the internet is reachable, perform HTTP POST to
Aria collector. If POST succeeds, then upgrade device to **connected**.
Exponential back-off cadence is going to be applied as-needed whenever device
remains in **connected** state. If HTTP POST fails to connect because of
timeout, DNS failure or reset, then downgrade device state back to **offline**,
requiring it to perform another internet reachability check at regular
**act_stats** interval cadence.

Benefits of the proposed approach:

-   Aria SDK performs internet connectivity and reachability check without
    causing undue stress to collector.

-   Aria SDK takes care of HTTP stack SSL socket connectivity state - avoids
    performing exponential backoff,  
    thus avoiding performance (CPU, RAM, Radio) issues client-side.

-   Network state Detector PAL becomes optional with active probing /
    reachability check: device intelligently reschedules TPM wake-up depending
    on network reachability result. This allows to reduce memory utilization: no
    need to keep pending unposted events in in-RAM TPM back-off queue. Once
    device downgrades to **offline** state – events may get saved to disk,
    freeing-up some RAM previously allocated for HTTP post.
