package com.microsoft.applications.events.okhttp;

import androidx.annotation.NonNull;

import com.microsoft.applications.events.HttpClient;
import com.microsoft.applications.events.HttpClientRequest;

import java.io.IOException;

import okhttp3.Dispatcher;
import okhttp3.OkHttpClient;

public class OkHttpRequestFactory implements HttpClientRequest.Factory {
    private final OkHttpClient okHttpClient;

    public OkHttpRequestFactory() {
        // Increase the default maxRequestsPerHost from 5 to 10 when using Http/2
        this(new OkHttpClient.Builder().dispatcher(createDispatcher(10 /* maxRequestsPerHost */)).build());
    }

    public OkHttpRequestFactory(OkHttpClient okHttpClient) {
        this.okHttpClient = okHttpClient;
    }

    @Override
    public HttpClientRequest create(@NonNull HttpClient parent, String url, String method, byte[] body, String request_id, @NonNull HttpClientRequest.Headers headers) throws IOException {
        return new OkHttpRequest(okHttpClient, parent, url, method, body, request_id, headers);
    }

    private static Dispatcher createDispatcher(int maxRequestsPerHost) {
        Dispatcher dispatcher = new Dispatcher();
        dispatcher.setMaxRequestsPerHost(maxRequestsPerHost);
        return dispatcher;
    }
}