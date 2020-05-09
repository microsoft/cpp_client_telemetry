package com.microsoft.applications.events.maesdktest;

import android.content.Context;
import android.util.Log;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import com.microsoft.applications.events.HttpClient;

import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class ExampleInstrumentedTest extends MaeUnitLogger {
    public void log_failure(String filename, int line, String summary)
    {
        StringBuilder failure = new StringBuilder(filename == null ? "NO file" : filename);
        failure.append(":");
        failure.append(line);
        failure.append(": ");
        failure.append(summary == "null" ? "null" : summary);
        fail(failure.toString());
    }

    @Test
    public void useAppContext() {
        // Context of the app under test.
        Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();

        assertEquals("com.microsoft.applications.events.maesdktest", appContext.getPackageName());
    }

    @Test
    public void runNativeTests() {
        System.loadLibrary("native-lib");
        System.loadLibrary("maesdk");

        Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        HttpClient client = new HttpClient(appContext);

        TestStub stub = new TestStub();
        int result = stub.runNativeTests(this);
        assertEquals(0, result);
        Log.i("MAE", "Test finished");
    }
}
