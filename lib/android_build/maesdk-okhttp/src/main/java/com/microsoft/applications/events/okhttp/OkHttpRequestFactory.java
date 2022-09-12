package com.microsoft.applications.events.okhttp;

import androidx.annotation.NonNull;

import com.microsoft.applications.events.HttpClient;
import com.microsoft.applications.events.HttpClientRequest;

import java.io.IOException;
import java.util.Map;

import okhttp3.OkHttpClient;

public class OkHttpRequestFactory implements HttpClientRequest.Factory {
    private final OkHttpClient m_okHttpClient;

    public OkHttpRequestFactory() {
        this(new OkHttpClient.Builder().build());
    }

    public OkHttpRequestFactory(OkHttpClient okHttpClient) {
        m_okHttpClient = okHttpClient;
    }

    @Override
    public HttpClientRequest create(@NonNull HttpClient parent, String url, String method, byte[] body, String request_id, @NonNull Map<String, String> headers) throws IOException {
        return new OkHttpRequest(m_okHttpClient, parent, url, method, body, request_id, headers);
    }
}