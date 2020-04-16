package com.microsoft.applications.events.maesdktest;

import android.content.Context;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import com.microsoft.applications.events.HttpClient;

import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.assertEquals;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class ExampleInstrumentedTest {
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
        assertEquals(0, stub.runNativeTests());
    }
}
