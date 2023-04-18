//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events.maesdktest;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import com.microsoft.applications.events.HttpClient;
import com.microsoft.applications.events.OfflineRoom;

import java.util.Locale;
import java.util.concurrent.ExecutionException;

public class MainActivity extends Activity {
    static class DummyLogger extends MaeUnitLogger {
        @Override
        void log_failure(String filename, int line, String summary) {
            Log.e("MAE", String.format("Uh oh %s: %s", filename, summary));
        }
    }

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("maesdk");
    }

    HttpClient m_client;

    @SuppressLint("SetTextI18n")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        m_client = new HttpClient(getApplicationContext());
        OfflineRoom.connectContext(getApplicationContext());
        TestStub testStub = new TestStub();
        DummyLogger dummyLogger = new DummyLogger();
        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        try {
            Integer result = testStub.executorRun(dummyLogger);
            tv.setText(String.format(Locale.ROOT, "Tests returned %d", result));
        } catch (ExecutionException e) {
            tv.setText("Woopsy");
        } catch (InterruptedException e) {
            tv.setText("Interrupted!");
        }
    }
}

