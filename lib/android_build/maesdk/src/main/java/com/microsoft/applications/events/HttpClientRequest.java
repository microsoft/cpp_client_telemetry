package com.microsoft.applications.events;

import androidx.annotation.NonNull;

import java.io.IOException;
import java.util.Map;

public interface HttpClientRequest extends Runnable {

    interface Factory {
        HttpClientRequest create(
                @NonNull HttpClient parent,
                String url,
                String method,
                byte[] body,
                String request_id,
                @NonNull Map<String, String> headers
        ) throws IOException;

        class AndroidUrlConnection implements HttpClientRequest.Factory {
            @Override
            public HttpClientRequest create(@NonNull HttpClient parent, String url, String method, byte[] body, String request_id, @NonNull Map<String, String> headers) throws IOException {
                return new Request(parent, url, method, body, request_id, headers);
            }
        }
    }
}