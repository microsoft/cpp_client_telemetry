//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events.maesdktest;

import android.content.Context;
import com.microsoft.applications.events.HttpClient;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.FutureTask;

public class TestStub {
  class CallTests implements Callable<Integer> {
    MaeUnitLogger logger;
    HttpClient httpClient;
    Context appContext;

    CallTests(MaeUnitLogger logger, HttpClient httpClient, Context appContext) {
      this.logger = logger;
      this.httpClient = httpClient;
      this.appContext = appContext;
    }

    /**
     * Computes a result, or throws an exception if unable to do so.
     *
     * @return computed result
     * @throws Exception if unable to compute a result
     */
    @Override
    public Integer call() throws Exception {
      return Integer.valueOf(
          runNativeTests(logger, httpClient, appContext, System.getProperty("java.io.tmpdir")));
    }
  }

  public Integer executorRun(MaeUnitLogger logger, HttpClient httpClient, Context appContext)
      throws ExecutionException, InterruptedException {
    ExecutorService executorService = Executors.newFixedThreadPool(2);

    FutureTask<Integer> tests =
        new FutureTask<Integer>(new CallTests(logger, httpClient, appContext));
    executorService.execute(tests);
    return tests.get();
  }

  public native int runNativeTests(
      MaeUnitLogger logger, HttpClient httpClient, Context appContext, String cacheFilePath);
}
