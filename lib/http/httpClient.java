package com.microsoft.office.ariasdk;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.System;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;
import java.util.concurrent.FutureTask;

import static java.nio.charset.StandardCharsets.UTF_8;

/**
 * Created by maharrim on 8/21/2019.
 */

class Request implements Runnable {

    Request(String url,
            String method,
            byte[] body,
            String request_id,
            int[] header_length,
            byte[] header_buffer
    ) throws java.net.MalformedURLException, java.io.IOException
    {
        m_url = new URL(url);
        m_connection = (HttpURLConnection) m_url.openConnection();
        m_connection.setRequestMethod(method);
        m_body = body;
        if (body.length > 0) {
            m_connection.setFixedLengthStreamingMode(body.length);
            m_connection.setDoOutput(true);
        }
        m_request_id = request_id;
        int offset = 0;
        for (int i = 0; i + 1 < header_length.length; i += 2) {
            String k = new String(header_buffer, offset, header_length[i], UTF_8);
            offset += header_length[i];
            String v = new String(header_buffer, offset, header_length[i+1], UTF_8);
            offset += header_length[i+1];
            m_connection.setRequestProperty(k, v);
        }
    }

    public void run()
    {
        String[] headerArray = {};
        byte[] body = {};
        int response = 0;
        try {
            if (m_body.length > 0) {
                OutputStream body_stream = m_connection.getOutputStream();
                body_stream.write(m_body);
            }
            response = m_connection.getResponseCode(); // may throw
            Map<String, List<String>> headers = m_connection.getHeaderFields();
            Vector<String> headerList = new Vector<String>();
            for (Map.Entry<String, List<String>> entry : headers.entrySet()) {
                if (entry.getKey() != null) {
                    for (String v : entry.getValue()) {
                        headerList.add(entry.getKey());
                        headerList.add(v);
                    }
                }
            }
            headerArray = headerList.toArray(headerArray);
            BufferedInputStream in;
            if (response >= 300) {
                in = new BufferedInputStream(m_connection.getErrorStream());
            }
            else {
                in = new BufferedInputStream(m_connection.getInputStream());
            }
            byte[] buffer = new byte[1024];
            Vector<byte[]> buffers = new Vector<byte[]>();
            int size = 0;
            while (true) {
                int n = in.read(buffer, 0, 1024);
                if (n < 0) {
                    break;
                }
                if (n > 0) {
                    buffers.add(java.util.Arrays.copyOfRange(buffer, 0, n));
                    size += n;
                }
            }
            body = new byte[size];
            int index = 0;
            for (byte[] chunk : buffers) {
                for (byte b : chunk) {
                    body[index] = b;
                    index += 1;
                }
            }
        } catch (Exception e) {
            /* pass this on as a response of 0 */
            e.getMessage();
        } finally {
            m_connection.disconnect();
        }
        dispatchCallback(
                m_request_id,
                response,
                headerArray,
                body);
    }

    public void dispatchCallback(String id,
                                 int response,
                                 Object[] headers,
                                 byte[] body)
    {
        // this stub makes it easier to mock this method in
        // Java unit tests.
        nativeDispatchCallback(id, response, headers, body);
    }

    public native void nativeDispatchCallback(String id,
                                              int response,
                                              Object[] headers,
                                              byte[] body);

    private URL m_url;
    private HttpURLConnection m_connection;
    private byte[] m_body = {};
    public String m_request_id;
}

public class httpClient{
    public httpClient(int n_threads)
    {
        String path = System.getProperty("java.io.tmpdir");
        setCacheFilePath(path);
        m_executor = Executors.newFixedThreadPool(n_threads);
        createClientInstance();
    }

    public void finalize()
    {
        deleteClientInstance();
        m_executor.shutdown();
    }

    public native void createClientInstance();
    public native void deleteClientInstance();
    public native void setCacheFilePath(String path);
    public FutureTask<Boolean> createTask(String url,
                                String method,
                                byte[] body,
                                String request_id,
                                                int[] header_index,
                                                byte[] header_buffer)
    {
        try {
            Request r = new Request(url, method, body, request_id, header_index, header_buffer);
            FutureTask<Boolean> t = new FutureTask<Boolean>(r, true);
            m_executor.execute(t);
            return t;
        }
        catch (Exception e) {
            return null;
        }
    }

    public void executeTask(FutureTask<Boolean> t)
    {
        m_executor.execute(t);
    }

    ExecutorService m_executor;
}
