//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events.maesdktest;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.FutureTask;

public class TestStub {
  class CallTests implements Callable<Integer> {
    MaeUnitLogger logger;

    CallTests(MaeUnitLogger logger) {
      this.logger = logger;
    }

    /**
     * Computes a result, or throws an exception if unable to do so.
     *
     * @return computed result
     * @throws Exception if unable to compute a result
     */
    @Override
    public Integer call() throws Exception {
      return Integer.valueOf(runNativeTests(logger));
    }
  }

  public Integer executorRun(MaeUnitLogger logger) throws ExecutionException, InterruptedException {
    ExecutorService executorService = Executors.newFixedThreadPool(2);

    FutureTask<Integer> tests = new FutureTask<Integer>(new CallTests(logger));
    executorService.execute(tests);
    return tests.get();
  }

  public native int runNativeTests(MaeUnitLogger logger);
}
