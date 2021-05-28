//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events.maesdktest;

import static org.hamcrest.Matchers.greaterThan;
import static org.hamcrest.Matchers.hasItem;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.isA;
import static org.hamcrest.Matchers.isEmptyOrNullString;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.notNullValue;
import static org.hamcrest.Matchers.nullValue;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.fail;

import android.content.Context;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;
import com.microsoft.applications.events.DebugEvent;
import com.microsoft.applications.events.DebugEventListener;
import com.microsoft.applications.events.DebugEventType;
import com.microsoft.applications.events.DiagLevel;
import com.microsoft.applications.events.HttpClient;
import com.microsoft.applications.events.ILogConfiguration;
import com.microsoft.applications.events.ILogManager;
import com.microsoft.applications.events.ILogger;
import com.microsoft.applications.events.LogConfigurationKey;
import com.microsoft.applications.events.LogManager;
import com.microsoft.applications.events.LogManager.LogConfigurationImpl;
import com.microsoft.applications.events.LogManagerProvider;
import com.microsoft.applications.events.LogSessionData;
import com.microsoft.applications.events.OfflineRoom;
import com.microsoft.applications.events.PrivacyGuard;
import com.microsoft.applications.events.PrivacyGuardInitConfig;
import com.microsoft.applications.events.Status;
import java.util.Arrays;
import java.util.Collections;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.TreeSet;
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

  public class MockHttpClient extends HttpClient {

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

  static public MockHttpClient s_client = null;

  @Test
  public void multipleLogManagerInstantiation() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    if (s_client == null) {
      s_client = new MockHttpClient(appContext);
    }
    synchronized (s_client.urlMap) {
      s_client.urlMap.clear();
    }
    OfflineRoom.connectContext(appContext);

    final String token =
        "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123";

    String[] names = {
        "alpha-contoso",
        "beta-contoso",
        "gamma-contoso",
        "delta-contoso",
    };
    for (String name : names) {
      final String url = String.format("https://%s/", name);
      ILogConfiguration custom = LogManager.logConfigurationFactory();
      custom.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, token);
      custom.set(LogConfigurationKey.CFG_STR_COLLECTOR_URL, url);
      custom.set(LogConfigurationKey.CFG_INT_MAX_TEARDOWN_TIME, (long) 5);
      custom.set(LogConfigurationKey.CFG_STR_FACTORY_NAME, name);
      custom.set(LogConfigurationKey.CFG_STR_CACHE_FILE_PATH, name);
      assertThat(custom.getString(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN), is(token));
      assertThat(custom.getString(LogConfigurationKey.CFG_STR_COLLECTOR_URL), is(url));

      final ILogManager secondaryManager = LogManagerProvider.createLogManager(custom);
      final ILogger secondaryLogger = secondaryManager.getLogger(token, "osotnoc", "");
      secondaryLogger.logEvent("osotnoc");
      secondaryManager.uploadNow();

      try {
        Thread.sleep(2000);
      } catch (InterruptedException e) {
      }
      synchronized (s_client.urlMap) {
        assertThat(s_client.urlMap.containsKey(url), is(true));
      }
    }
  }

  @Test
  public void restartManager() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    if (s_client == null) {
      s_client = new MockHttpClient(appContext);
    }
    synchronized (s_client.urlMap) {
      s_client.urlMap.clear();
    }
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
      } catch (InterruptedException e) {
        // nothing to see here
      }
      LogManager.flushAndTeardown();

      synchronized (s_client.urlMap) {
        for (int j = 0; j <= i; ++j) assertThat(s_client.urlMap.containsKey(urls[j]), is(true));
      }
    }
  }

  @Test
  public void startDDVonLogManager() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    if (s_client == null) {
      s_client = new MockHttpClient(appContext);
    }
    synchronized (s_client.urlMap) {
      s_client.urlMap.clear();
    }
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

    if (secondaryManager.initializeDiagnosticDataViewer("contoso", "http://10.0.0.2")) {
      assertThat(secondaryManager.isViewerEnabled(), is(true));
      secondaryLogger.logEvent("some.event");

      secondaryManager.flush();
      secondaryManager.pauseTransmission();
      secondaryManager.disableViewer();
    }
    LogManager.flushAndTeardown();
  }

  /*
  Disabling this test since it requires private modules.

  @Test
  public void startPrivacyGuardWithMultipleLogManagers() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    if (s_client == null) {
      s_client = new MockHttpClient(appContext);
    }
    synchronized (s_client.urlMap) {
      s_client.urlMap.clear();
    }
    OfflineRoom.connectContext(appContext);

    final String token =
            "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123";
    final String contosoToken =
            "0123456789abcdef9123456789abcdef-01234567-0123-0123-0123-0123456789ab-0124";
    final String contosoUrl = "https://bozo.contoso.com/";
    final String contosoName = "ContosoFactory";
    final String contosoDatabase = "ContosoSequel";

    final ILogger initialLogger = LogManager.initialize(token);

    PrivacyGuardInitConfig config = new PrivacyGuardInitConfig();
    config.loggerInstance = initialLogger;
    config.ScanForURLs = false;
    config.UseEventFieldPrefix = true;
    // Init Privacy Guard
    PrivacyGuard.initialize(config);

    // Register PG with default LogManager.
    assertThat(LogManager.registerPrivacyGuard(), is(true));

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

    // Register PG with secondary LogManager
    assertThat(secondaryManager.registerPrivacyGuard(), is(true));
    secondaryLogger.logEvent("some.event");
    // Unregister PG with secondary LogManager
    assertThat(secondaryManager.unregisterPrivacyGuard(), is(true));

    // Unregister PG with Default LogManager
    // This can also be done above after registration.
    assertThat(LogManager.unregisterPrivacyGuard(), is(true));

    LogManager.flushAndTeardown();
  }
   */

  @Test
  public void pauseAndResume() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    if (s_client == null) {
      s_client = new MockHttpClient(appContext);
    }
    synchronized (s_client.urlMap) {
      s_client.urlMap.clear();
    }
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

    class FilterListener extends DebugEventListener {

      long filteredCount = 0;
      @Override
      public void onDebugEvent(DebugEvent evt) {
        synchronized(this) {
          filteredCount += 1;
        }
      }
    }
    FilterListener listener = new FilterListener();
    manager.addEventListener(DebugEventType.EVT_FILTERED, listener);
    logger.logEvent("ContosoEvent");
    synchronized(listener) {

    }
    manager.resumeTransmission(); // just in case
    manager.flush();
    manager.uploadNow();
    try {
      Thread.sleep(2000);
    } catch (InterruptedException e) {
      // nothing to see here
    }
    synchronized (s_client.urlMap) {
      assertThat(s_client.urlMap.keySet(), hasItem(contosoUrl));
    }
    manager.pauseTransmission();
    try {
      Thread.sleep(125);
    } catch (InterruptedException e) {
    }
    int beforePause = -1;
    synchronized (s_client.urlMap) {
      beforePause = s_client.urlMap.get(contosoUrl);
    }
    logger.logEvent("ContosoEvent");
    manager.uploadNow();
    try {
      Thread.sleep(2000);
    } catch (InterruptedException e) {
    }
    synchronized (s_client.urlMap) {
      assertThat(s_client.urlMap.get(contosoUrl), is(beforePause));
    }
    manager.resumeTransmission();
    manager.uploadNow();
    try {
      Thread.sleep(2000);
    } catch (InterruptedException e) {
    }
    synchronized (s_client.urlMap) {
      assertThat(s_client.urlMap.get(contosoUrl), greaterThan(beforePause));
    }
  }

  @Test
  public void transmitProfiles() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    if (s_client == null) {
      s_client = new MockHttpClient(appContext);
    }
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

  @Test
  public void sessionData() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    if (s_client == null) {
      s_client = new MockHttpClient(appContext);
    }
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

    LogSessionData sessionData = manager.getLogSessionData();
    assertThat(sessionData, is(notNullValue()));
    assertThat(sessionData.getSessionFirstTime(), is(not(0l)));
    assertThat(sessionData.getSessionSDKUid(), is(not(isEmptyOrNullString())));
  }

  @Test
  public void levelFilterAndDebugEvents() {
    System.loadLibrary("maesdk");
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    if (s_client == null) {
      s_client = new MockHttpClient(appContext);
    }
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
    ILogger logger = manager.getLogger(token, "contoso", "");

    class ListenForFilter extends DebugEventListener {

      long filteredCount = 0;

      @Override
      public void onDebugEvent(DebugEvent evt) {
        synchronized (this) {
          filteredCount += 1;
        }
      }
    }

    ListenForFilter listener = new ListenForFilter();
    manager.addEventListener(DebugEventType.EVT_FILTERED, listener);
    logger.logEvent("noprops");
    manager.uploadNow();
    try {
      Thread.sleep(1000);
    } catch (InterruptedException e) {}
    synchronized(listener) {
      assertThat(listener.filteredCount, is(0L));
    }
    int[] allowed = { DiagLevel.DIAG_LEVEL_REQUIRED.value() };
    manager.setLevelFilter(DiagLevel.DIAG_LEVEL_OPTIONAL.value(),
        allowed);
    logger.logEvent("nopropsagain");
    manager.uploadNow();
    try {
      Thread.sleep(1000);
    } catch (InterruptedException e) {}
    synchronized(listener) {
      assertThat(listener.filteredCount, is(1L));
    }
    manager.removeEventListener(DebugEventType.EVT_FILTERED, listener);
    int[] everything = { DiagLevel.DIAG_LEVEL_REQUIRED.value(), DiagLevel.DIAG_LEVEL_OPTIONAL.value() };
    manager.setLevelFilter(DiagLevel.DIAG_LEVEL_OPTIONAL.value(), everything);
  }

  @Test
  public void getDefaultConfig() {
    ILogConfiguration defaultConfig = ILogConfiguration.getDefaultConfiguration();
    final String COLLECTOR_URL_PROD = "https://self.events.data.microsoft.com/OneCollector/1.0/";

    assertThat(defaultConfig.getBoolean(LogConfigurationKey.CFG_BOOL_ENABLE_ANALYTICS), is(false));
    assertThat(defaultConfig.getLong(LogConfigurationKey.CFG_INT_CACHE_FILE_SIZE), is(3145728L));
    assertThat(defaultConfig.getLong(LogConfigurationKey.CFG_INT_RAM_QUEUE_SIZE), is(524288L));
    assertThat(defaultConfig.getBoolean(LogConfigurationKey.CFG_BOOL_ENABLE_MULTITENANT), is(true));
    assertThat(defaultConfig.getBoolean(LogConfigurationKey.CFG_BOOL_ENABLE_DB_DROP_IF_FULL), is(false));
    assertThat(defaultConfig.getLong(LogConfigurationKey.CFG_INT_MAX_TEARDOWN_TIME), is(1L));
    assertThat(defaultConfig.getLong(LogConfigurationKey.CFG_INT_MAX_PENDING_REQ), is(4L));
    assertThat(defaultConfig.getLong(LogConfigurationKey.CFG_INT_RAM_QUEUE_BUFFERS), is(3L));
    assertThat(defaultConfig.getLong(LogConfigurationKey.CFG_INT_TRACE_LEVEL_MASK), is(0L));
    assertThat(defaultConfig.getBoolean(LogConfigurationKey.CFG_BOOL_ENABLE_TRACE), is(true));
    assertThat(defaultConfig.getString(LogConfigurationKey.CFG_STR_COLLECTOR_URL), is(COLLECTOR_URL_PROD));
    assertThat(defaultConfig.getLong(LogConfigurationKey.CFG_INT_STORAGE_FULL_PCT), is(75L));
    assertThat(defaultConfig.getLong(LogConfigurationKey.CFG_INT_STORAGE_FULL_CHECK_TIME), is(5000L));
    assertThat(defaultConfig.getLong(LogConfigurationKey.CFG_INT_RAMCACHE_FULL_PCT), is(75L));
    assertThat(defaultConfig.getBoolean(LogConfigurationKey.CFG_BOOL_ENABLE_NET_DETECT), is(true));
    assertThat(defaultConfig.getBoolean(LogConfigurationKey.CFG_BOOL_SESSION_RESET_ENABLED), is(false));
    ILogConfiguration http = defaultConfig.getLogConfiguration(LogConfigurationKey.CFG_MAP_HTTP);
    assertThat(http, is(notNullValue()));
    assertThat(http.getBoolean(LogConfigurationKey.CFG_BOOL_HTTP_COMPRESSION), is(true));
    assertThat(http.getString(LogConfigurationKey.CFG_STR_HTTP_CONTENT_ENCODING), is("deflate"));
    assertThat(http.getBoolean(LogConfigurationKey.CFG_BOOL_HTTP_MS_ROOT_CHECK), is(false));
    ILogConfiguration utc = defaultConfig.getLogConfiguration(LogConfigurationKey.CFG_MAP_UTC);
    assertThat(utc, is(notNullValue()));
    // we do not expect UTC to be enabled on Android.
    assertThat(utc.getBoolean(LogConfigurationKey.CFG_BOOL_UTC_ENABLED), is(false));
    ILogConfiguration meta = defaultConfig.getLogConfiguration(LogConfigurationKey.CFG_MAP_METASTATS_CONFIG);
    assertThat(meta, is(notNullValue()));
    assertThat(meta, isA(ILogConfiguration.class));
    assertThat(meta.getBoolean(LogConfigurationKey.CFG_BOOL_METASTATS_SPLIT), is(false));
    assertThat(meta.getLong(LogConfigurationKey.CFG_INT_METASTATS_INTERVAL), is(1800L));
    assertThat(meta.getString(LogConfigurationKey.CFG_STR_METASTATS_TOKEN_INT), isA(String.class));
    assertThat(meta.getString(LogConfigurationKey.CFG_STR_METASTATS_TOKEN_INT), not(isEmptyOrNullString()));
    assertThat(meta.getString(LogConfigurationKey.CFG_STR_METASTATS_TOKEN_PROD), isA(String.class));
    assertThat(meta.getString(LogConfigurationKey.CFG_STR_METASTATS_TOKEN_PROD), not(isEmptyOrNullString()));
    ILogConfiguration tpm = defaultConfig.getLogConfiguration(LogConfigurationKey.CFG_MAP_TPM);
    assertThat(tpm, is(notNullValue()));
    assertThat(tpm, isA(ILogConfiguration.class));
    assertThat(tpm.getLong(LogConfigurationKey.CFG_INT_TPM_MAX_BLOB_BYTES), is(2L * 1024 * 1024));
    assertThat(tpm.getLong(LogConfigurationKey.CFG_INT_TPM_MAX_RETRY), is(5L));
    assertThat(tpm.getBoolean(LogConfigurationKey.CFG_BOOL_TPM_CLOCK_SKEW_ENABLED), is(true));
    ILogConfiguration compat = defaultConfig.getLogConfiguration(LogConfigurationKey.CFG_MAP_COMPAT);
    assertThat(compat, is(notNullValue()));
    assertThat(compat.getBoolean(LogConfigurationKey.CFG_BOOL_COMPAT_DOTS), is(true));
    assertThat(compat.getString(LogConfigurationKey.CFG_STR_COMPAT_PREFIX), isA(String.class));

    assertThat(defaultConfig, is(instanceOf(LogManager.LogConfigurationImpl.class)));

    TreeMap<String, LogConfigurationKey> keyMap = new TreeMap<String, LogConfigurationKey>();
    for (LogConfigurationKey k : LogConfigurationKey.values()) {
      String kKey = k.getKey();
      keyMap.put(kKey, k);
    }
    TreeMap<String, Object> configMap = ((LogConfigurationImpl) defaultConfig).getConfigMap();
    for (String k : configMap.keySet()) {
      assertThat(keyMap.keySet(), hasItem(k));
    }
  }

  @Test
  public void enforceKeyNaming() {
    final String[] allowListArray = {
        "CFG_STR_UTC",
    };
    final TreeSet<String> allowSet = new TreeSet<String>(Arrays.asList(allowListArray));
    for (LogConfigurationKey k : LogConfigurationKey.values()) {
      String keyName = k.name();
      if (allowSet.contains(keyName)) {
        continue;
      }
      if (keyName.startsWith("CFG_INT_")) {
        assertThat(String.format("Key %s should be a long value", keyName), k.getValueType() == Long.class, is(true));
      }
      else if (keyName.startsWith("CFG_STR_")) {
        assertThat(String.format("Key %s should be a String value", keyName), k.getValueType() == String.class, is(true));
      }
      else if (keyName.startsWith("CFG_BOOL_")) {
        assertThat(String.format("Key %s should be a boolean value", keyName), k.getValueType() == Boolean.class, is(true));
      }
      else if (keyName.startsWith("CFG_MAP_")) {
        assertThat(String.format("Key %s should be a nested map value", keyName), k.getValueType() == ILogConfiguration.class, is(true));
      }
      else {
        fail(String.format("Key %s has an unexpected prefix", keyName));
      }
    }
  }
}
