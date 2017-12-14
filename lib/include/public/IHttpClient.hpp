// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Version.hpp"
#include "Enums.hpp"
#include <map>
#include <string>
#include <vector>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Events  {
// *INDENT-ON*


/// <summary>
/// Class encompassing a list of HTTP headers
/// </summary>
class HttpHeaders : public std::multimap<std::string, std::string>
{
  public:
    using std::multimap<std::string, std::string>::const_iterator;
    using std::multimap<std::string, std::string>::iterator;
    using std::multimap<std::string, std::string>::value_type;

  public:
    HttpHeaders()
    {
    }

    void set(std::string const& name, std::string const& value)
    {
        auto range = equal_range(name);
        auto hint = erase(range.first, range.second);
        insert(hint, std::make_pair(name, value));
    }

    void add(std::string const& name, std::string const& value)
    {
        insert(std::make_pair(name, value));
    }

    std::string const& get(std::string const& name) const
    {
        auto it = find(name);
        return (it != end()) ? it->second : m_empty;
    }

    bool has(std::string const& name) const
    {
        auto it = find(name);
        return (it != end());
    }

    using std::multimap<std::string, std::string>::begin;
    using std::multimap<std::string, std::string>::end;

  protected:
    std::string m_empty;
};

//---

/// <summary>
/// HTTP request object.
/// Individual HTTP client implementations can implement the request object in
/// the most efficient way - either fill the request first and only then issue
/// the underlying request, or create the real request immediately and forward
/// the methods and set the individual parameters directly one by one.
/// </summary>
class IHttpRequest
{
  public:
    virtual ~IHttpRequest() {}
    virtual const std::string& GetId() const = 0;
    virtual void SetMethod(std::string const& method) = 0;
    virtual void SetUrl(std::string const& url) = 0;
    virtual HttpHeaders& GetHeaders() = 0;
    virtual void SetBody(std::vector<uint8_t>& body) = 0;
    virtual void SetLatency(EventLatency priority) = 0;
    virtual size_t GetSizeEstimate() const = 0;
};

/// <summary>
/// HTTP response object.
/// Individual HTTP client implementations can implement the response object
/// in the most efficient way - either copy all the underlying data to a new
/// structure and provide just that to the callback, or keep the real
/// response data around, forward the methods and retrieve the individual
/// values directly one by one.
/// </summary>
class IHttpResponse
{
  public:
    virtual ~IHttpResponse() {}
    virtual const std::string& GetId() const = 0;
    virtual HttpResult GetResult() const = 0;
    virtual unsigned GetStatusCode() const = 0;
    virtual const HttpHeaders& GetHeaders() const = 0;
    virtual const std::vector<uint8_t>& GetBody() const = 0;
};

//---

/// <summary>
/// HTTP request object wrapping a simple structure
/// </summary>
class SimpleHttpRequest : public IHttpRequest {
  public:
    std::string          m_id;
    std::string          m_method;
    std::string          m_url;
    HttpHeaders          m_headers;
    std::vector<uint8_t> m_body;
    EventLatency         m_latency;

  public:
    SimpleHttpRequest(std::string const& id)
      : m_id(id),
        m_method("GET"),
        m_latency(EventLatency_Unspecified)
    {
    }

    virtual ~SimpleHttpRequest()
    {
    }

    virtual const std::string& GetId() const override
    {
        return m_id;
    }

    virtual void SetMethod(std::string const& method) override
    {
        m_method = method;
    }

    virtual void SetUrl(std::string const& url) override
    {
        m_url = url;
    }

    virtual HttpHeaders& GetHeaders() override
    {
        return m_headers;
    }

    virtual void SetBody(std::vector<uint8_t>& body) override
    {
        m_body = std::move(body);
    }

    virtual void SetLatency(EventLatency latency) override
    {
        m_latency = latency;
    }

    virtual size_t GetSizeEstimate() const override
    {
        // Not accounting for a few more chars here and there, assuming the
        // protocol & hostname part of the URL reasonably offsets that.
        size_t size = m_method.size() + m_url.size() + m_body.size();
        for (auto const& header : m_headers) {
            size += header.first.size() + header.second.size() + 4;
        }
        return size;
    }
};

/// <summary>
/// HTTP response object wrapping a simple structure
/// </summary>
class SimpleHttpResponse : public IHttpResponse {
  public:
    std::string          m_id;
    HttpResult           m_result;
    unsigned             m_statusCode;
    HttpHeaders          m_headers;
    std::vector<uint8_t> m_body;

  public:
    SimpleHttpResponse(std::string const& id)
      : m_id(id),
        m_result(HttpResult_LocalFailure),
        m_statusCode(0)
    {
    }

    virtual ~SimpleHttpResponse()
    {
    }

    virtual const std::string& GetId() const override
    {
        return m_id;
    }

    virtual HttpResult GetResult() const override
    {
        return m_result;
    }

    virtual unsigned GetStatusCode() const override
    {
        return m_statusCode;
    }

    virtual const HttpHeaders& GetHeaders() const override
    {
        return m_headers;
    }

    virtual const std::vector<uint8_t>& GetBody() const override
    {
        return m_body;
    }
};

//---

/// <summary>
/// Interface for receiving HTTP client responses
/// </summary>
class IHttpResponseCallback
{
  public:
    virtual ~IHttpResponseCallback() {}

    /// <summary>
    /// Called when an HTTP request is done.
    /// The passed response object contains details about the exact way the
    /// request has finished (HTTP status code, headers, content, error codes
    /// etc.). The ownership of the response object is transferred to the
    /// callback object. It can store it for later if necessary. Finally it
    /// must deleted through its virtual destructor.
    /// </summary>
    /// <param name="response">Object with response data</param>
    virtual void OnHttpResponse(IHttpResponse const* response) = 0;
};

//---

/// <summary>
/// Interface for HTTP client implementations
/// </summary>
class IHttpClient
{
  public:
    virtual ~IHttpClient() {}

    /// <summary>
    /// Creates an empty HTTP request.
    /// The created request object has only its ID pre-filled. Other fields
    /// are up to be set by the caller. The request object can then be sent
    /// using the SendRequestAsync() method. If the request object is not
    /// going to be actually used, it can be safely deleted using its virtual
    /// destructor.
    /// </summary>
    /// <returns>HTTP request object to prepare</returns>
    virtual IHttpRequest* CreateRequest() = 0;

    /// <summary>
    /// Starts an HTTP request.
    /// The method takes ownership of passed request and can destroy it before
    /// returning back to the caller. Do not access the request object in any
    /// way after this invocation and do not delete it.
    /// The callback object will be always called, even if the request will be
    /// cancelled or an error will occur immediatelly during sending. In the
    /// latter case, the OnHttpResponse() callback will be called before this
    /// method returns. The callback object must be kept alive until its
    /// OnHttpResponse() callback is called. It will be never used twice, so
    /// after that it can be safely deleted.
    /// </summary>
    /// <param name="request">Filled request object returned earlier by
    /// CreateRequest()</param>
    /// <param name="callback">Callback to receive the response</param>
    virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) = 0;

    /// <summary>
    /// Cancels an HTTP request.
    /// Caller must provide a string ID returned earlier by request->GetId().
    /// The request will be cancelled asynchronously. The caller still needs
    /// to wait for the relevant OnHttpResponse() callback, it can just come
    /// earlier with some "aborted" error status.
    /// </summary>
    /// <param name="id">ID of the request to cancel</param>
    virtual void CancelRequestAsync(std::string const& id) = 0;
};


}}} // namespace Microsoft::Applications::Events 
