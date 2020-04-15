package com.microsoft.applications.events;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.os.BatteryManager;
import android.os.Build;
import android.provider.Settings;

import androidx.annotation.RequiresApi;

import java.io.BufferedInputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Vector;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.FutureTask;

import static java.nio.charset.StandardCharsets.UTF_8;

/**
 * Created by maharrim on 8/21/2019.
 */

class PowerInfoReceiver extends android.content.BroadcastReceiver {
    private final HttpClient m_parent;
    boolean m_charging = true;
    boolean m_low_battery = false;

    PowerInfoReceiver(HttpClient parent)
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
    ConnectivityCallback(HttpClient parent, boolean metered) {
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

    private final HttpClient m_parent;
    private boolean m_metered;
}

class Request implements Runnable {

    Request(HttpClient parent, String url, String method, byte[] body, String request_id, int[] header_length, byte[] header_buffer)
            throws java.io.IOException {
        m_parent = parent;
        m_connection = (HttpURLConnection) parent.newUrl(url).openConnection();
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
            Vector<String> headerList = new Vector<>();
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
            Vector<byte[]> buffers = new Vector<>();
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

   private HttpURLConnection m_connection;
    private byte[] m_body;
    private String m_request_id;
    private final HttpClient m_parent;
}

public class HttpClient {
    private static final int MAX_HTTP_THREADS = 2; // Collector wants no more than 2 at a time

    public HttpClient(Context context)
    {
        m_context = context;
        String path = System.getProperty("java.io.tmpdir");
        setCacheFilePath(path);
        setDeviceInfo(calculateID(context), Build.MANUFACTURER, Build.MODEL);
        calculateAndSetSystemInfo(context);
        m_executor = createExecutor();
        createClientInstance();
        // We need API 24 to follow changes in network status
        if (hasConnectivityManager()) {
            m_connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (m_connectivityManager != null) {
                boolean is_metered = m_connectivityManager.isActiveNetworkMetered();
                m_callback = new ConnectivityCallback(this, is_metered);
                onCostChange(is_metered); // set initial value in C++ side
                m_connectivityManager.registerDefaultNetworkCallback(m_callback);
            }
        }
        m_power_receiver = new PowerInfoReceiver(this);
        IntentFilter filter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Intent status = context.registerReceiver(m_power_receiver, filter);
        if (status != null) {
            m_power_receiver.onReceive(context, status);
        }
    }

    @SuppressLint("NewApi")
    private static String getLanguageTag(Locale locale)
    {
        if (Build.VERSION.SDK_INT >= 21) {
            return locale.toLanguageTag();
        }
        return locale.toString().replace('_', '-');
    }

    private void calculateAndSetSystemInfo(android.content.Context context)
    {
        String app_id = context.getPackageName();
        PackageInfo pInfo;
        try {
            pInfo = context.getPackageManager().getPackageInfo(app_id, 0);
        } catch (PackageManager.NameNotFoundException e) {
            pInfo = null;
        }
        String app_version = "";
        if (pInfo != null && pInfo.versionName != null) {
            app_version = pInfo.versionName;
        }
        String app_language = getLanguageTag(context.getResources().getConfiguration().locale);

        String os_major_version = Build.VERSION.RELEASE;
        String os_full_version = os_major_version + " " + Build.VERSION.INCREMENTAL;
        setSystemInfo(app_id, app_version, app_language, os_major_version, os_full_version);
    }

    private String calculateID(android.content.Context context)
    {
        // The definition of ANDROID_ID changed in API 26.
        // https://developer.android.com/reference/android/provider/Settings.Secure#ANDROID_ID

        final ContentResolver resolver = context.getContentResolver();
        String id;
        try {
            id = Settings.Secure.getString(resolver, Settings.Secure.ANDROID_ID);
        } catch (Exception e) {
            id = e.toString();
        }
        if (id == null) {
            return "";
        } else {
            return "a:" + id;
        }
    }

    protected ExecutorService createExecutor()
    {
        return Executors.newFixedThreadPool(MAX_HTTP_THREADS);
    }

    protected boolean hasAndroidID() {
        return Build.VERSION.SDK_INT >= 26;
    }
    
    protected boolean hasConnectivityManager()
    {
        return Build.VERSION.SDK_INT >= 24;
    }
    
    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
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

    public native void setDeviceInfo(String id, String manufacturer, String model);

    public native void setSystemInfo(
        String app_id,
        String app_version,
        String app_language,
        String os_major_version,
        String os_full_version
    );

    public native void dispatchCallback(String id, int response, Object[] headers, byte[] body);

    public FutureTask<Boolean> createTask(String url, String method, byte[] body, String request_id, int[] header_index,
            byte[] header_buffer) {
        try {
            Request r = new Request(this, url, method, body, request_id, header_index, header_buffer);
            return new FutureTask<>(r, true);
        } catch (Exception e) {
            return null;
        }
    }

    public void executeTask(FutureTask<Boolean> t) {
        m_executor.execute(t);
    }

    private final ExecutorService m_executor;
    private ConnectivityCallback m_callback;
    private android.net.ConnectivityManager m_connectivityManager;
    private PowerInfoReceiver m_power_receiver;
    private final Context m_context;
}
