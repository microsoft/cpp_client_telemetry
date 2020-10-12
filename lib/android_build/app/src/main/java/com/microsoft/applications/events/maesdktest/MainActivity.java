//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events.maesdktest;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import com.microsoft.applications.events.HttpClient;
import com.microsoft.applications.events.OfflineRoom;
import java.util.concurrent.ExecutionException;

public class MainActivity extends AppCompatActivity {
  class DummyLogger extends MaeUnitLogger {

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
    tv.setText(String.format("Tests returned %d", result));
      }
    catch(ExecutionException e) {
      tv.setText("Woopsy");
    }
    catch(InterruptedException e) {
      tv.setText("Interrupted!");
    }
  }

  /**
   * A native method that is implemented by the 'native-lib' native library, which is packaged with
   * this application.
   */
  public native String stringFromJNI(String path);

  HttpClient m_client;
}

