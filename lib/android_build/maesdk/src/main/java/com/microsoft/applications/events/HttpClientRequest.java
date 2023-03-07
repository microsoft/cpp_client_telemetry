//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import androidx.annotation.NonNull;

import java.io.IOException;
import java.util.Iterator;

import static java.nio.charset.StandardCharsets.UTF_8;

public interface HttpClientRequest extends Runnable {

    interface Factory {
        HttpClientRequest create(
                @NonNull HttpClient parent,
                String url,
                String method,
                byte[] body,
                String request_id,
                @NonNull Headers headers
        ) throws IOException;

        class AndroidUrlConnection implements HttpClientRequest.Factory {
            @Override
            public HttpClientRequest create(@NonNull HttpClient parent, String url, String method, byte[] body, String request_id, @NonNull Headers headers) throws IOException {
                return new Request(parent, url, method, body, request_id, headers);
            }
        }
    }

    final class HeaderEntry {
        public final String key;
        public final String value;

        public HeaderEntry(String key, String value) {
            this.key = key;
            this.value = value;
        }
    }

    final class Headers implements Iterator<HeaderEntry> {
        public final int length;
        private final int[] header_lengths;
        private final byte[] buffer;
        private int current = 0;
        private int offset = 0;

        public Headers(int[] header_lengths, byte[] buffer) {
            this.length = header_lengths.length;
            this.header_lengths = header_lengths;
            this.buffer = buffer;
        }

        @Override
        public boolean hasNext() {
            return current + 2 <= length;
        }

        @Override
        public HeaderEntry next() {
            String k = new String(buffer, offset, header_lengths[current], UTF_8);
            offset += header_lengths[current];
            String v = new String(buffer, offset, header_lengths[current + 1], UTF_8);
            offset += header_lengths[current + 1];
            current += 2;
            return new HeaderEntry(k,v);
        }
    }
}