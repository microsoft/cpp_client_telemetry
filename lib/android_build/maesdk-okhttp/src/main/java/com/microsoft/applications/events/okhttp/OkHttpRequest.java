package com.microsoft.applications.events.okhttp;

import android.util.Log;

import com.microsoft.applications.events.HttpClient;
import com.microsoft.applications.events.HttpClientRequest;

import java.util.Map;
import java.util.Vector;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import okhttp3.Headers;
import okhttp3.OkHttpClient;
import okhttp3.RequestBody;
import okhttp3.ResponseBody;

class OkHttpRequest implements HttpClientRequest {
    private final OkHttpClient okHttpClient;
    private final okhttp3.Request okHttpRequest;
    private final String requestId;
    private final HttpClient parent;

    OkHttpRequest(
            @NonNull OkHttpClient okHttpClient,
            @NonNull HttpClient parent,
            String url,
            String method,
            @Nullable byte[] body,
            String request_id,
            @NonNull HttpClientRequest.Headers headers)
            throws java.io.IOException {
        this.okHttpClient = okHttpClient;
        this.parent = parent;
        this.requestId = request_id;

        RequestBody okHttpBody = null;
        if (body != null && body.length > 0) {
            okHttpBody = RequestBody.create(body);
        }

        okhttp3.Headers.Builder okHeaders = new okhttp3.Headers.Builder();
        while (headers.hasNext()) {
            HttpClientRequest.HeaderEntry header = headers.next();
            okHeaders.add(header.key, header.value);
        }

        okHttpRequest = new okhttp3.Request.Builder()
            .url(parent.newUrl(url))
            .method(method, okHttpBody)
            .headers(okHeaders.build())
            .build();
    }

    @Override
    public void run() {
        String[] headerArray = {};
        byte[] body = {};
        int responseCode = 0;

        try (okhttp3.Response okHttpResponse = okHttpClient.newCall(okHttpRequest).execute()) {
            responseCode = okHttpResponse.code();
            okhttp3.Headers headers = okHttpResponse.headers();
            Vector<String> headerList = new Vector<>();
            for (int i = 0; i < headers.size(); i++) {
                headerList.add(headers.name(i));
                headerList.add(headers.value(i));
            }
            headerArray = headerList.toArray(headerArray);

            ResponseBody responseBody = okHttpResponse.body();
            if (responseBody != null) {
                body = responseBody.bytes();
            }
        } catch (Exception e) {
            Log.e("MAE", "Exception in callback", e);
        }
        parent.dispatchCallback(requestId, responseCode, headerArray, body);
    }
}