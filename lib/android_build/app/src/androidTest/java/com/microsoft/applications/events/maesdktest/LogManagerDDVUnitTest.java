//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events.maesdktest;

import static org.hamcrest.Matchers.hasItem;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.isA;
import static org.hamcrest.Matchers.isIn;
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
import java.util.SortedSet;
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

  class MockHttpClient extends HttpClient {

    public SortedSet<String> urlSet;

    public MockHttpClient(Context context) {
      super(context);
      urlSet = Collections.synchronizedSortedSet(new TreeSet());
    }

    public FutureTask<Boolean> createTask(
        String url,
        String method,
        byte[] body,
        String request_id,
        int[] header_index,
        byte[] header_buffer) {
      synchronized (urlSet) {
        urlSet.add(url);
      }
      Runnable r = new MockRequest(this, request_id);
      return new FutureTask<Boolean>(r, true);
    }
  }

  @Test
  public void doubleManagerInstantiation() {
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

    /*
    Log an event. Ideally, we would mock and verify that the HTTP client is uploading this
    event to the desired endpoint. Coming soon to a unit test near you.
     */
    assertThat(contosoLogger, isA(ILogger.class));
    contosoLogger.logEvent("contosoevent");
    assertThat(LogManager.flush(), is(Status.SUCCESS));

    ILogConfiguration secondaryConfig = LogManager.logConfigurationFactory();
    secondaryConfig.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, token);
    secondaryConfig.set(LogConfigurationKey.CFG_STR_COLLECTOR_URL, "https://localhost:5000/");
    secondaryConfig.set(LogConfigurationKey.CFG_INT_MAX_TEARDOWN_TIME, (long) 5);
    secondaryConfig.set(LogConfigurationKey.CFG_STR_FACTORY_NAME, "osotnoc");
    secondaryConfig.set(LogConfigurationKey.CFG_STR_CACHE_FILE_PATH, "osotnoc");

    ILogManager secondaryManager = LogManagerProvider.createLogManager(secondaryConfig);
    ILogger secondaryLogger = secondaryManager.getLogger(token, "osotnoc", "");
    secondaryLogger.logEvent("osotnoc");
    assertThat(secondaryManager.flush(), is(Status.SUCCESS));

    LogManager.flushAndTeardown();

    synchronized (client.urlSet) {
      assertThat("https://localhost:5000/", isIn(client.urlSet));
      assertThat(contosoUrl, isIn(client.urlSet));
    }

    /*
    Both ILogger and ILogManager are AutoCloseable, and it is good practice
    to call the close() method to tear each of them down. This will prevent use-after-free
    disasters from Java wrappers passing dangling (freed) C++ native pointers into native code.
     */

  }
}
