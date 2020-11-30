//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events.maesdktest;

import static org.hamcrest.Matchers.greaterThan;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.isA;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.nullValue;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.fail;

import android.content.Context;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;
import com.microsoft.applications.events.HttpClient;
import com.microsoft.applications.events.ILogConfiguration;
import com.microsoft.applications.events.ILogManager;
import com.microsoft.applications.events.ILogger;
import com.microsoft.applications.events.LogConfigurationKey;
import com.microsoft.applications.events.LogManager;
import com.microsoft.applications.events.LogManagerProvider;
import com.microsoft.applications.events.OfflineRoom;
import com.microsoft.applications.events.Status;
import java.util.Collections;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.concurrent.FutureTask;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public class LogManagerDDVUnitTest extends MaeUnitLogger {

  public void log_failure(String filename, int line, String summary) {
    fail(String.format("%s:%d: %s", filename, line, summary));
  }

  class MockRequest implements Runnable {

    MockHttpClient m_parent;
    String m_request_id;

    public MockRequest(MockHttpClient parent, String id) {
      m_parent = parent;
      m_request_id = id;
    }

    public void run() {
      String[] headers = {};
      byte[] body = {};
      m_parent.dispatchCallback(m_request_id, 200, headers, body);
    }
  }

  class MockHttpClient extends HttpClient {

    public SortedMap<String, Integer> urlMap;

    public MockHttpClient(Context context) {
      super(context);
      urlMap = Collections.synchronizedSortedMap(new TreeMap());
    }

    public FutureTask<Boolean> createTask(
        String url,
        String method,
        byte[] body,
        String request_id,
        int[] header_index,
        byte[] header_buffer) {
      synchronized (urlMap) {
        if (urlMap.containsKey(url)) {
          urlMap.put(url, urlMap.get(url) + 1);
        } else {
          urlMap.put(url, 1);
        }
      }
      Runnable r = new MockRequest(this, request_id);
      return new FutureTask<Boolean>(r, true);
    }
  }

  @Test
  public void tripleManagerInstantiation() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    MockHttpClient client = new MockHttpClient(appContext);
    OfflineRoom.connectContext(appContext);

    final String token =
        "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123";
    final String contosoToken =
        "0123456789abcdef9123456789abcdef-01234567-0123-0123-0123-0123456789ab-0124";
    final String contosoUrl = "https://bozo.contoso.com/";
    final String contosoName = "ContosoFactory";
    final String contosoDatabase = "ContosoSequel";

    ILogConfiguration custom = LogManager.logConfigurationFactory();
    /*

    Set up configuration for a second log manager instance. Why second, when we have
    no previous log manager initialized in this test? Because we get a default, nameless
    log manager in the SDK's internal tables whether we ask for it here or not.

    Key points: we set CFG_STR_COLLECTOR_URL to define the collector URL we wish to use. We
    set CFG_INT_MAX_TEARDOWN_TIME because the SDK will crash on teardown if this parameter
    is missing. We set CFG_STR_FACTORY_NAME to give this log manager a unique identity and
    unique configuration.

     */
    custom.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, contosoToken);
    custom.set(LogConfigurationKey.CFG_STR_COLLECTOR_URL, contosoUrl);
    custom.set(LogConfigurationKey.CFG_INT_MAX_TEARDOWN_TIME, (long) 5);
    custom.set(LogConfigurationKey.CFG_STR_FACTORY_NAME, contosoName);
    custom.set(LogConfigurationKey.CFG_STR_CACHE_FILE_PATH, contosoDatabase);
    assertThat(custom.getString(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN), is(contosoToken));
    assertThat(custom.getString(LogConfigurationKey.CFG_STR_COLLECTOR_URL), is(contosoUrl));

    ILogger contosoLogger = LogManager.initialize(contosoToken, custom);

    ILogConfiguration copy = LogManager.getLogConfigurationCopy();
    assertThat(copy.getString(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN), is(contosoToken));
    assertThat(copy.getString(LogConfigurationKey.CFG_STR_COLLECTOR_URL), is(contosoUrl));
    assertThat(copy.getLong(LogConfigurationKey.CFG_INT_MAX_TEARDOWN_TIME), is((long) 5));
    assertThat(copy.getString(LogConfigurationKey.CFG_STR_FACTORY_NAME), is(contosoName));
    assertThat(copy.getLogConfiguration(LogConfigurationKey.CFG_MAP_TPM), is(not(nullValue())));

    /*
    Log an event. Ideally, we would mock and verify that the HTTP client is uploading this
    event to the desired endpoint. Coming soon to a unit test near you.
     */
    assertThat(contosoLogger, isA(ILogger.class));
    contosoLogger.logEvent("contosoevent");
    assertThat(LogManager.flush(), is(Status.SUCCESS));
    LogManager.uploadNow();
    try {
      Thread.sleep(2000);
    } catch (InterruptedException e) {
      // nothing to see here
    }
    LogManager.pauseTransmission();
    synchronized (client.urlMap) {
      assertThat(client.urlMap.containsKey(contosoUrl), is(true));
    }

    final String secondaryUrl = "https://localhost:5000/";
    copy.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, token);
    copy.set(LogConfigurationKey.CFG_STR_COLLECTOR_URL, secondaryUrl);
    copy.set(LogConfigurationKey.CFG_STR_FACTORY_NAME, "osotnoc");
    copy.set(LogConfigurationKey.CFG_STR_CACHE_FILE_PATH, "osotnoc");

    ILogManager secondaryManager = LogManagerProvider.createLogManager(copy);
    secondaryManager.resumeTransmission();
    ILogger secondaryLogger = secondaryManager.getLogger(token, "osotnoc", "");
    secondaryLogger.logEvent("osotnoc");
    secondaryManager.uploadNow();

    final String privateUrl = "https://contoso.com/wrong";
    copy.set(LogConfigurationKey.CFG_STR_COLLECTOR_URL, privateUrl);
    copy.set(LogConfigurationKey.CFG_STR_FACTORY_NAME, "PrivateThing");
    secondaryManager = LogManagerProvider.createLogManager(copy);
    secondaryLogger = secondaryManager.getLogger(token, "privateContoso", "");
    secondaryLogger.logEvent("PrivateTestEvent");
    secondaryManager.uploadNow();
    try {
      Thread.sleep(2000);
    } catch (InterruptedException e) {
    }
    synchronized (client.urlMap) {
      assertThat(client.urlMap.containsKey(privateUrl), is(true));
    }

    LogManager.pauseTransmission();
    LogManager.flushAndTeardown();
  }

  @Test
  public void restartManager() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    MockHttpClient client = new MockHttpClient(appContext);
    OfflineRoom.connectContext(appContext);

    final String[] urls = {
      "https://able.contoso.com/", "https://baker.contoso.com/", "https://charlie.contoso.com/"
    };
    final String[] databaseNames = {
      "ableData", "bravoData", "charlieData",
    };
    final String token =
        "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123";

    ILogConfiguration custom = LogManager.logConfigurationFactory();
    for (int i = 0; i < urls.length; ++i) {
      custom.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, token);
      custom.set(LogConfigurationKey.CFG_STR_COLLECTOR_URL, urls[i]);
      custom.set(LogConfigurationKey.CFG_STR_FACTORY_NAME, databaseNames[i]);
      custom.set(LogConfigurationKey.CFG_STR_CACHE_FILE_PATH, databaseNames[i]);

      ILogger contosoLogger = LogManager.initialize(token, custom);

      /*
      Log an event. Ideally, we would mock and verify that the HTTP client is uploading this
      event to the desired endpoint. Coming soon to a unit test near you.
       */
      assertThat(contosoLogger, isA(ILogger.class));
      contosoLogger.logEvent("contosoevent");
      assertThat(LogManager.flush(), is(Status.SUCCESS));
      LogManager.uploadNow();
      try {
        Thread.sleep(2000);
      } catch (java.lang.InterruptedException e) {
        // nothing to see here
      }
      LogManager.flushAndTeardown();

      synchronized (client.urlMap) {
        for (int j = 0; j <= i; ++j) assertThat(client.urlMap.containsKey(urls[j]), is(true));
      }
    }
  }

  @Test
  public void startDDVonLogManager() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    MockHttpClient client = new MockHttpClient(appContext);
    OfflineRoom.connectContext(appContext);

    final String token =
        "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123";
    final String contosoToken =
        "0123456789abcdef9123456789abcdef-01234567-0123-0123-0123-0123456789ab-0124";
    final String contosoUrl = "https://bozo.contoso.com/";
    final String contosoName = "ContosoFactory";
    final String contosoDatabase = "ContosoSequel";

    final ILogger initialLogger = LogManager.initialize(token);

    ILogConfiguration custom = LogManager.getLogConfigurationCopy();
    custom.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, contosoToken);
    custom.set(LogConfigurationKey.CFG_STR_COLLECTOR_URL, contosoUrl);
    custom.set(LogConfigurationKey.CFG_STR_FACTORY_NAME, contosoName);
    custom.set(LogConfigurationKey.CFG_STR_CACHE_FILE_PATH, contosoDatabase);
    assertThat(custom.getString(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN), is(contosoToken));
    assertThat(custom.getString(LogConfigurationKey.CFG_STR_COLLECTOR_URL), is(contosoUrl));
    assertThat(custom.getLogConfiguration(LogConfigurationKey.CFG_MAP_TPM), is(not(nullValue())));

    final ILogManager secondaryManager = LogManagerProvider.createLogManager(custom);
    final ILogConfiguration copyConfig = secondaryManager.getLogConfigurationCopy();
    assertThat(
        copyConfig.getLogConfiguration(LogConfigurationKey.CFG_MAP_TPM), is(not(nullValue())));
    final ILogger secondaryLogger = secondaryManager.getLogger(contosoToken, "contoso", "");

    assertThat(
        secondaryManager.initializeDiagnosticDataViewer("contoso", "http://10.0.0.2"), is(true));
    assertThat(secondaryManager.isViewerEnabled(), is(true));
    secondaryLogger.logEvent("some.event");

    secondaryManager.flush();
    secondaryManager.pauseTransmission();
    LogManager.flushAndTeardown();
  }

  @Test
  public void pauseAndResume() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    MockHttpClient client = new MockHttpClient(appContext);
    OfflineRoom.connectContext(appContext);

    final String token =
        "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123";
    final String contosoToken =
        "0123456789abcdef9123456789abcdef-01234567-0123-0123-0123-0123456789ab-0124";
    final String contosoUrl = "https://bozo.contoso.com/";
    final String contosoName = "ContosoFactory";
    final String contosoDatabase = "ContosoSequel";

    ILogConfiguration custom = LogManager.logConfigurationFactory();
    custom.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, contosoToken);
    custom.set(LogConfigurationKey.CFG_STR_COLLECTOR_URL, contosoUrl);
    custom.set(LogConfigurationKey.CFG_STR_FACTORY_NAME, contosoName);
    custom.set(LogConfigurationKey.CFG_STR_CACHE_FILE_PATH, contosoDatabase);
    ILogManager manager = LogManagerProvider.createLogManager(custom);
    ILogger logger = manager.getLogger(contosoToken, "contoso", "");
    logger.logEvent("ContosoEvent");
    manager.uploadNow();
    try {
      Thread.sleep(2000);
    } catch (java.lang.InterruptedException e) {
      // nothing to see here
    }
    synchronized (client.urlMap) {
      assertThat(client.urlMap.containsKey(contosoUrl), is(true));
    }
    manager.pauseTransmission();
    try {
      Thread.sleep(125);
    } catch (java.lang.InterruptedException e) {
    }
    int beforePause = -1;
    synchronized (client.urlMap) {
      beforePause = client.urlMap.get(contosoUrl);
    }
    logger.logEvent("ContosoEvent");
    manager.uploadNow();
    try {
      Thread.sleep(2000);
    } catch (java.lang.InterruptedException e) {
    }
    synchronized (client.urlMap) {
      assertThat(client.urlMap.get(contosoUrl), is(beforePause));
    }
    manager.resumeTransmission();
    manager.uploadNow();
    try {
      Thread.sleep(2000);
    } catch (java.lang.InterruptedException e) {
    }
    synchronized (client.urlMap) {
      assertThat(client.urlMap.get(contosoUrl), greaterThan(beforePause));
    }
  }

  @Test
  public void transmitProfiles() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    MockHttpClient client = new MockHttpClient(appContext);
    OfflineRoom.connectContext(appContext);

    final String token =
        "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123";
    final String contosoToken =
        "0123456789abcdef9123456789abcdef-01234567-0123-0123-0123-0123456789ab-0124";
    final String contosoUrl = "https://bozo.contoso.com/";
    final String contosoName = "ContosoFactory";
    final String contosoDatabase = "ContosoSequel";

    ILogConfiguration custom = LogManager.logConfigurationFactory();
    custom.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, contosoToken);
    custom.set(LogConfigurationKey.CFG_STR_COLLECTOR_URL, contosoUrl);
    custom.set(LogConfigurationKey.CFG_STR_FACTORY_NAME, contosoName);
    custom.set(LogConfigurationKey.CFG_STR_CACHE_FILE_PATH, contosoDatabase);
    ILogManager manager = LogManagerProvider.createLogManager(custom);
    ILogger logger = manager.getLogger(contosoToken, "contoso", "");

    assertThat(manager.getTransmitProfileName(), is("REAL_TIME"));

    assertThat(manager.setTransmitProfile("Fred"), is(Status.EFAIL));
    assertThat(manager.getTransmitProfileName(), is("REAL_TIME"));
    assertThat(manager.setTransmitProfile("BEST_EFFORT"), is(Status.SUCCESS));
    assertThat(manager.getTransmitProfileName(), is("BEST_EFFORT"));

    final String goodRuleJson =
        "[{\"name\": \"GoodRule\",\"rules\":["
            + "{\"netCost\":\"restricted\",\"timers\":[ -1,-1,-1]}"
            + "]}]";
    assertThat(manager.loadTransmitProfiles(goodRuleJson), is(Status.SUCCESS));
    assertThat(manager.getTransmitProfileName(), is("BEST_EFFORT"));
    assertThat(manager.setTransmitProfile("GoodRule"), is(Status.SUCCESS));
    assertThat(manager.getTransmitProfileName(), is("GoodRule"));
    final String badRuleJson = "badJson" + goodRuleJson;
    assertThat(manager.loadTransmitProfiles(badRuleJson), is(Status.EFAIL));
    // uploading bad json wipes out the older custome profile, and we revert
    // to the default REAL_TIME profile.
    assertThat(manager.getTransmitProfileName(), is("REAL_TIME"));
  }
}
