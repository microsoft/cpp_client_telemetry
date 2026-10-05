//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import com.microsoft.applications.events.HttpClientRequest;

import org.junit.Assert;
import org.junit.Test;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Collections;
import java.io.ByteArrayInputStream;
import java.io.IOException;

import static java.nio.charset.StandardCharsets.UTF_8;
import static org.junit.Assert.*;

public class HttpClientRequestTest {
    @Test
    public void acceptsExactlyFullResponseBody() throws IOException {
        byte[] body = new byte[Request.MAX_HTTP_RESPONSE_SIZE];
        Assert.assertArrayEquals(body, Request.readResponseBody(new ByteArrayInputStream(body)));
    }

    @Test(expected = IOException.class)
    public void rejectsOversizedResponseBody() throws IOException {
        Request.readResponseBody(new ByteArrayInputStream(new byte[Request.MAX_HTTP_RESPONSE_SIZE + 1]));
    }

    @Test
    public void acceptsExactlyFullResponseHeaders() throws IOException {
        String value = new String(new char[Request.MAX_HTTP_RESPONSE_HEADERS_SIZE - 5]).replace('\0', 'x');
        Map<String, java.util.List<String>> headers = new HashMap<>();
        headers.put("X", Collections.singletonList(value));
        Assert.assertArrayEquals(new String[] {"X", value}, Request.readResponseHeaders(headers));
    }

    @Test(expected = IOException.class)
    public void rejectsOversizedAggregateResponseHeaders() throws IOException {
        String value = new String(new char[Request.MAX_HTTP_RESPONSE_HEADERS_SIZE]).replace('\0', 'x');
        Map<String, java.util.List<String>> headers = new HashMap<>();
        headers.put("X", Collections.singletonList(value));
        Request.readResponseHeaders(headers);
    }

    @Test
    public void testHeaders() {
        byte[] bytes = "key1value1key33key2value2".getBytes(UTF_8);
        int[] header_lengths = new int[] { 4, 6, 5, 0, 4, 6 };

        HttpClientRequest.Headers headers = new HttpClientRequest.Headers(header_lengths, bytes);
        ArrayList<String> actual = new ArrayList<>();
        while (headers.hasNext()) {
            HttpClientRequest.HeaderEntry entry = headers.next();
            actual.add(entry.key);
            actual.add(entry.value);
        }

        Assert.assertEquals(6, headers.length);
        Assert.assertArrayEquals(
                new String[] {"key1", "value1", "key33", "", "key2", "value2"},
                actual.toArray()
        );
    }
}
