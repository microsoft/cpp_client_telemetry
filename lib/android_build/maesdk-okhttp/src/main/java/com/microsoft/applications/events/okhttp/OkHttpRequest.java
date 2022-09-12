package com.microsoft.applications.events.okhttp;

import android.util.Log;

import com.microsoft.applications.events.HttpClient;
import com.microsoft.applications.events.HttpClientRequest;

import java.util.Map;
import java.util.Vector;

import androidx.annotation.NonNull;
import okhttp3.Headers;
import okhttp3.OkHttpClient;
import okhttp3.RequestBody;
import okhttp3.ResponseBody;

class OkHttpRequest implements HttpClientRequest {
    private final okhttp3.Request m_okHttpRequest;
    private final String m_request_id;
    private final HttpClient m_parent;
    private final OkHttpClient m_okHttpClient;

    OkHttpRequest(
            @NonNull OkHttpClient okHttpClient,
            @NonNull HttpClient parent,
            String url,
            String method,
            byte[] body,
            String request_id,
            @NonNull Map<String, String> headers)
            throws java.io.IOException {
        m_okHttpClient = okHttpClient;
        m_parent = parent;
        m_request_id = request_id;

        RequestBody okHttpBody = null;
        if (body.length > 0) {
            okHttpBody = RequestBody.create(body);
        }

        m_okHttpRequest = new okhttp3.Request.Builder()
                .url(parent.newUrl(url))
                .method(method, okHttpBody)
                .headers(Headers.of(headers))
                .build();
    }

    @Override
    public void run() {
        String[] headerArray = {};
        byte[] body = {};
        int responseCode = 0;

        try (okhttp3.Response okHttpResponse = m_okHttpClient.newCall(m_okHttpRequest).execute()) {
            responseCode = okHttpResponse.code();
            Headers headers = okHttpResponse.headers();
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
        m_parent.dispatchCallback(m_request_id, responseCode, headerArray, body);
    }
}