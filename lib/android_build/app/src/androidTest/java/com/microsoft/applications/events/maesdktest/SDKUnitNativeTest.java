package com.microsoft.applications.events.maesdktest;

import static org.hamcrest.Matchers.isA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.fail;

import android.content.Context;
import android.util.Log;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;
import com.microsoft.applications.events.HttpClient;
import com.microsoft.applications.events.ILogger;
import com.microsoft.applications.events.LogManager;
import com.microsoft.applications.events.OfflineRoom;
import org.junit.Test;
import org.junit.runner.RunWith;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class SDKUnitNativeTest extends MaeUnitLogger {
  public void log_failure(String filename, int line, String summary) {
    fail(String.format("%s:%d: %s", filename, line, summary));
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
    OfflineRoom.connectContext(appContext);

    TestStub stub = new TestStub();
    int result = stub.runNativeTests(this);
    assertEquals(0, result);
    Log.i("MAE", "Test finished");
  }

  @Test
  public void wrapperLogManager() {
    System.loadLibrary("maesdk");

    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    HttpClient client = new HttpClient(appContext);
    OfflineRoom.connectContext(appContext);

    final String token =
        "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123";

    ILogger logger = LogManager.initialize(token);
    assertThat(logger, isA(ILogger.class));
  }
}
