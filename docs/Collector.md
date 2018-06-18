Collector
=========

"Collector" is term used for the OneCollector web service which receives
telemetry packages send by the 1DS OneSDK client library.

URL
---

-   <https://mobile.pipe.aria.microsoft.com/Collector/3.0/>

Request headers
---------------

Set by the library:

    Content-Type: application/bond-compact-binary
    Client-Id: NO_AUTH
    SDK-Version: <Prefix>-<Platform>-<SKU>-<Projection>-<BuildVersion>
    X-APIKey: <tenant token>

Should be handled by the HTTP client level:

    Expect: 100-continue
    Content-Length: <length of data>
    Connection: Keep-Alive

If compression is enabled:

    Content-Encoding: deflate

Request payload
---------------

`clienttelemetry::data::v3::ClientToCollectorRequest` serialized into
Bond compact binary format v1.

If compression is enabled, the contents are deflated. The compression is
the *raw deflate* (RFC1951), e.g. what .NET
`System.IO.Compression.DeflateStream` returns, not the *zlib deflate*
(RFC1950) which RFC7230 declares for `Content-Encoding: deflate` --
Microsoft web servers implement it like that due to an old mistake and
for historical reasons. The conversion from *zlib deflate* to *raw
deflate* is to cut off the first 2 and last 4 bytes (header and
checksum).
