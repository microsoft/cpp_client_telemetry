//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events.maesdktest;

import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.isA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.fail;

import android.content.Context;
import android.util.Log;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;
import com.microsoft.applications.events.DataCategory;
import com.microsoft.applications.events.EventProperty;
import com.microsoft.applications.events.HttpClient;
import com.microsoft.applications.events.ILogConfiguration;
import com.microsoft.applications.events.ILogManager;
import com.microsoft.applications.events.ILogger;
import com.microsoft.applications.events.LogConfigurationKey;
import com.microsoft.applications.events.LogManager;
import com.microsoft.applications.events.LogManager.LogConfigurationImpl;
import com.microsoft.applications.events.LogManagerProvider;
import com.microsoft.applications.events.OfflineRoom;
import com.microsoft.applications.events.PiiKind;
import org.hamcrest.Description;
import org.hamcrest.TypeSafeDiagnosingMatcher;
import org.junit.Test;
import org.junit.runner.RunWith;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class SDKUnitNativeTest extends MaeUnitLogger {
  public static class isValueSuperset extends TypeSafeDiagnosingMatcher<LogConfigurationImpl> {

    final LogConfigurationImpl subset;

    public isValueSuperset(LogConfigurationImpl subset) {
      super(LogConfigurationImpl.class);
      this.subset = subset;
    }

    /**
     * Matcher: is actual a superset of the given LogConfigurationImpl
     *
     * @param item
     * @param mismatchDescription
     */
    @Override
    protected boolean matchesSafely(LogConfigurationImpl item, Description mismatchDescription) {
      StringBuffer failure = new StringBuffer();
      if (item.valueContainsAll(subset, failure)) {
        return true;
      }
      mismatchDescription.appendText(failure.toString());
      return false;
    }

    /**
     * Generates a description of the object. The description may be part of a a description of a
     * larger object of which this is just a component, so it should be worded appropriately.
     *
     * @param description The description to be built or appended to.
     */
    @Override
    public void describeTo(Description description) {
      description.appendText("match: is this item a superset of the expected configuration");
    }
  }

  static isValueSuperset isValueSuperset(ILogConfiguration expected) {
    return new isValueSuperset((LogManager.LogConfigurationImpl) expected);
  }

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
    logger.logEvent("fred");
  }

  @Test
  public void wrapperLogManagerConfig() {
    System.loadLibrary("maesdk");

    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    HttpClient client = new HttpClient(appContext);
    OfflineRoom.connectContext(appContext);

    final String token =
        "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123";

    ILogConfiguration custom = LogManager.logConfigurationFactory();
    custom.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, token);
    assertThat(custom.getString(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN), is(token));
    ILogger logger = LogManager.initialize("", custom);
    ILogConfiguration newConfig = LogManager.getLogConfigurationCopy();
    assertNotNull(newConfig);
    assertThat(newConfig.getString(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN), is(token));

    assertNotNull(logger);
    logger.logEvent("amazingAndroidUnitTest");
    LogManager.flush();
  }

  @Test
  public void wrapperLogManagerCopyConfig() throws Exception {
    System.loadLibrary("maesdk");

    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    HttpClient client = new HttpClient(appContext);
    OfflineRoom.connectContext(appContext);

    final String token =
        "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123";

    ILogConfiguration current = LogManager.logConfigurationFactory();
    current.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, token);
    ILogConfiguration metastats = LogManager.logConfigurationFactory();
    metastats.set(LogConfigurationKey.CFG_INT_METASTATS_INTERVAL, (long) 0);
    current.set(LogConfigurationKey.CFG_MAP_METASTATS_CONFIG, metastats);
    ILogConfiguration factory = LogManager.logConfigurationFactory();
    factory.set(LogConfigurationKey.CFG_STR_FACTORY_NAME, "copyConfig");
    current.set(LogConfigurationKey.CFG_MAP_FACTORY_CONFIG, factory);
    assertThat(current.getString(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN), is(token));
    try (ILogManager logManager = LogManagerProvider.createLogManager(current)) {
      ILogConfiguration postConfig = logManager.getLogConfigurationCopy();
      assertThat((LogConfigurationImpl) postConfig, isValueSuperset(current));
    }
  }

  @Test
  public void roundTripConfigLong() {
    System.loadLibrary("maesdk");

    ILogConfiguration config = LogManager.logConfigurationFactory();
    config.set(LogConfigurationKey.CFG_INT_METASTATS_INTERVAL, (long) 23);
    ILogConfiguration mangled = config.roundTrip();
    assertThat((LogConfigurationImpl) mangled, isValueSuperset(config));
  }

  @Test
  public void roundTripConfigBool() {
    System.loadLibrary("maesdk");

    ILogConfiguration config = LogManager.logConfigurationFactory();
    config.set(LogConfigurationKey.CFG_BOOL_COMPAT_DOTS, false);
    ILogConfiguration mangled = config.roundTrip();
    assertThat((LogConfigurationImpl) mangled, isValueSuperset(config));
  }

  @Test
  public void roundTripConfigString() {
    System.loadLibrary("maesdk");

    ILogConfiguration config = LogManager.logConfigurationFactory();
    config.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, "foobar");
    ILogConfiguration mangled = config.roundTrip();
    assertThat((LogConfigurationImpl) mangled, isValueSuperset(config));
  }

  @Test
  public void roundTripConfigMap() {
    System.loadLibrary("maesdk");

    ILogConfiguration submap = LogManager.logConfigurationFactory();
    submap.set(LogConfigurationKey.CFG_INT_METASTATS_INTERVAL, (long) 23);
    ILogConfiguration config = LogManager.logConfigurationFactory();
    config.set(LogConfigurationKey.CFG_MAP_FACTORY_CONFIG, submap);
    ILogConfiguration mangled = config.roundTrip();
    assertThat((LogConfigurationImpl) mangled, isValueSuperset(config));
  }

  @Test
  public void roundTripConfigArray() {
    System.loadLibrary("maesdk");

    Long[] foobar = new Long[7];
    for (int i = 0; i < 7; ++i) {
      foobar[i] = Long.valueOf(i);
    }
    ILogConfiguration config = LogManager.logConfigurationFactory();
    config.set("foobar", foobar);
    ILogConfiguration mangled = config.roundTrip();
    assertThat((LogConfigurationImpl) mangled, isValueSuperset(config));
  }

  public native int nativeGetPiiType(EventProperty property);

  public native int nativeGetDataCategory(EventProperty property);

  @Test
  public void piiKindIdentity() {
    System.loadLibrary("maesdk");
    System.loadLibrary("native-lib");

    EventProperty prop = new EventProperty("foo", PiiKind.Identity, DataCategory.PartB);
    int roundTrip = nativeGetPiiType(prop);
    assertThat(roundTrip, is(PiiKind.Identity.getValue()));
    roundTrip = nativeGetDataCategory(prop);
    assertThat(roundTrip, is(DataCategory.PartB.getValue()));
  }
}
