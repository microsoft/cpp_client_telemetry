package com.microsoft.applications.events;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import static org.mockito.Mockito.*;
import org.mockito.junit.MockitoJUnitRunner;

import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.os.BatteryManager;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.FutureTask;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;

import com.microsoft.applications.events.httpClient;

import static org.junit.Assert.*;

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(MockitoJUnitRunner.class)
public class EventsUnitTest {
    @Mock
    Context mockContext;

    @Mock
    Intent mockIntent;

    @Mock
    ConnectivityManager mockManager;

    @Mock
    URL mockUrl;

    @Mock
    HttpURLConnection mockConnection;

    @Mock
    OutputStream mockBodyStream;

    @Mock
    ExecutorService mockExecutor;

    class Stubby extends httpClient {

        Stubby(Context context) throws java.io.IOException {
            super(context);
        }

        @Override
        protected boolean hasConnectivityManager() {
            return false;
        }

        @Override
        public void createClientInstance() {

        }

        @Override
        public void deleteClientInstance() {

        }

        @Override
        public void setCacheFilePath(String path) {

        }

        @Override
        public void onCostChange(boolean isMetered) {

        }

        @Override
        public void onPowerChange(boolean isCharging, boolean isLow) {

        }

        @Override
        public void dispatchCallback(String id, int response, Object[] headers, byte[] body) {
            assertNotNull(id);
        }

        @Override
        public URL newUrl(String url) throws java.net.MalformedURLException {
            return mockUrl;
        }

        @Override
        protected ExecutorService createExecutor()
        {
            return mockExecutor;
        }
    }

    class StubbyAllow extends Stubby {
        StubbyAllow(Context context) throws java.io.IOException {
            super(context);
        }

        @Override
        protected boolean hasConnectivityManager() {
            return true;
        }
    }

    @Test
    public void canInstantiate() throws java.io.IOException {
        when(mockContext.registerReceiver(isA(BroadcastReceiver.class), isA(IntentFilter.class))).thenReturn(mockIntent);
        Stubby stubs = new Stubby(mockContext);
        /* Stubby should not attempt to access the CONNECTIVITY_SERVICE */
        verify(mockContext, times(0)).getSystemService(Context.CONNECTIVITY_SERVICE);
        verify(mockContext, times(1)).registerReceiver(isA(BroadcastReceiver.class), isA(IntentFilter.class));
        verify(mockIntent, times(1)).getIntExtra(BatteryManager.EXTRA_STATUS, -1);
    }

    @Test
    public void canInstantiateWithConnectivity() throws java.io.IOException {
                when(mockContext.registerReceiver(isA(BroadcastReceiver.class), isA(IntentFilter.class))).thenReturn(mockIntent);
        when(mockContext.getSystemService(Context.CONNECTIVITY_SERVICE)).thenReturn(mockManager);
        StubbyAllow stubs = new StubbyAllow(mockContext);
        verify(mockContext, times(1)).getSystemService(Context.CONNECTIVITY_SERVICE);
        verify(mockManager, times(1)).isActiveNetworkMetered();
        verify(mockManager, times(1)).registerDefaultNetworkCallback(isA(android.net.ConnectivityManager.NetworkCallback.class));
    }

    @Test
    public void canCreateTask() throws java.io.IOException {
        final List<String> someValue = new Vector<String>();
        someValue.add("bar");
        final Map<String, List<String>> headerMap = new TreeMap<String, List<String>>();
        headerMap.put("foo", someValue);
        String body_string = "fred";
        byte[] body_bytes = body_string.getBytes();
        final InputStream bodyStream = new ByteArrayInputStream(body_bytes);

        when(mockUrl.openConnection()).thenReturn(mockConnection);
        when(mockConnection.getOutputStream()).thenReturn(mockBodyStream);
        when(mockConnection.getHeaderFields()).thenReturn(headerMap);
        when(mockConnection.getResponseCode()).thenReturn(200);
        when(mockConnection.getInputStream()).thenReturn(bodyStream);
        when(mockContext.registerReceiver(isA(BroadcastReceiver.class), isA(IntentFilter.class))).thenReturn(mockIntent);
        
        Stubby stubs = new Stubby(mockContext);
        final String url = "https://www.contoso.com";
        final String method = "POST";
        final byte[] body = {0, 1, 2};
        final String request_id = "Fred The Request";
        final int[] header_index = {4, 3};
        final byte[] header_buffer = {0, 1, 2, 3, 7, 8, 9};
        FutureTask<Boolean> task = stubs.createTask(
            url, method, body, request_id, header_index, header_buffer
        );
        assertNotNull(task);
        task.run();
    }
}