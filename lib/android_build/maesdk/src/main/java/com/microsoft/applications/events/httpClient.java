package com.microsoft.applications.events;

import android.annotation.TargetApi;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.os.BatteryManager;
import android.os.Build;
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

class PowerReceiver extends android.content.BroadcastReceiver {
    final httpClient m_parent;
    boolean m_charging = true;
    boolean m_low_battery = false;

    PowerReceiver(httpClient parent)
    {
        m_parent = parent;
    }

    final public void onReceive(android.content.Context context, android.content.Intent intent) {
        final int status = intent.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
        final boolean isCharging = status == BatteryManager.BATTERY_STATUS_CHARGING
                || status == BatteryManager.BATTERY_STATUS_FULL;
        final boolean isLow = intent.getBooleanExtra(BatteryManager.EXTRA_BATTERY_LOW, false);
        m_parent.onPowerChange(isCharging, isLow);
    }
}

// See below: we test build version before instantiating this.
@TargetApi(24)
class ConnectivityCallback extends android.net.ConnectivityManager.NetworkCallback {
    ConnectivityCallback(httpClient parent, boolean metered) {
        m_parent = parent;
        m_metered = metered;
    }

    final public void onCapabilitiesChanged(Network network, NetworkCapabilities networkCapabilities) {
        final boolean new_metered = !networkCapabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_NOT_METERED);
        if (new_metered != m_metered) {
            m_metered = new_metered;
            m_parent.onCostChange(m_metered);
        }
    }

    final httpClient m_parent;
    boolean m_metered;
}

class Request implements Runnable {

    Request(httpClient parent, String url, String method, byte[] body, String request_id, int[] header_length, byte[] header_buffer)
            throws java.net.MalformedURLException, java.io.IOException {
        m_parent = parent;
        m_url = parent.newUrl(url);
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
            String v = new String(header_buffer, offset, header_length[i + 1], UTF_8);
            offset += header_length[i + 1];
            m_connection.setRequestProperty(k, v);
        }
    }

    public void run() {
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
            } else {
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
        m_parent.dispatchCallback(m_request_id, response, headerArray, body);
    }

    private URL m_url;
    private HttpURLConnection m_connection;
    private byte[] m_body = {};
    public String m_request_id;
    protected httpClient m_parent;
}

public class httpClient {
    private static final int MAX_HTTP_THREADS = 2; // Collector wants no more than 2 at a time

    public httpClient(android.content.Context context) {
        m_context = context;
        String path = System.getProperty("java.io.tmpdir");
        setCacheFilePath(path);
        m_executor = createExecutor();
        createClientInstance();
        // We need API 24 to follow changes in network status
        if (hasConnectivityManager()) {
            m_connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            boolean is_metered = m_connectivityManager.isActiveNetworkMetered();
            m_callback = new ConnectivityCallback(this, is_metered);
            onCostChange(is_metered); // set initial value in C++ side
            m_connectivityManager.registerDefaultNetworkCallback(m_callback);
        }
        m_power_receiver = new PowerReceiver(this);
        IntentFilter filter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Intent status = context.registerReceiver(m_power_receiver, filter);
        m_power_receiver.onReceive(context, status);
    }

    protected ExecutorService createExecutor()
    {
        return Executors.newFixedThreadPool(MAX_HTTP_THREADS);
    }

    protected boolean hasConnectivityManager()
    {
        return Build.VERSION.SDK_INT >= 24;
    }
    
    public void finalize() {
        if (m_callback != null) {
            m_connectivityManager.unregisterNetworkCallback(m_callback);
            m_callback = null;
        }
        m_context.unregisterReceiver(m_power_receiver);
        m_power_receiver = null;
        deleteClientInstance();
        m_executor.shutdown();
    }

    public URL newUrl(String url) throws java.net.MalformedURLException
    {
        return new URL(url);
    }

    public native void createClientInstance();

    public native void deleteClientInstance();

    public native void setCacheFilePath(String path);

    public native void onCostChange(boolean isMetered);

    public native void onPowerChange(boolean isCharging, boolean isLow);

    public native void dispatchCallback(String id, int response, Object[] headers, byte[] body);

    public FutureTask<Boolean> createTask(String url, String method, byte[] body, String request_id, int[] header_index,
            byte[] header_buffer) {
        try {
            Request r = new Request(this, url, method, body, request_id, header_index, header_buffer);
            FutureTask<Boolean> t = new FutureTask<Boolean>(r, true);
            return t;
        } catch (Exception e) {
            return null;
        }
    }

    public void executeTask(FutureTask<Boolean> t) {
        m_executor.execute(t);
    }

    ExecutorService m_executor;
    ConnectivityCallback m_callback;
    android.net.ConnectivityManager m_connectivityManager;
    PowerReceiver m_power_receiver;
    Context m_context;
}
