package com.microsoft.applications.events.maesdktest;

import android.os.Bundle;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.microsoft.applications.events.HttpClient;

public class MainActivity extends AppCompatActivity {

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
        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText("Fun Times " + stringFromJNI(System.getProperty("java.io.tmpdir")));
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI(String path);
    HttpClient m_client;
}
