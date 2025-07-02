//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events.maesdktest;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.FutureTask;

public class TestStub {
  static class CallTests implements Callable<Integer> {
    private final MaeUnitLogger logger;
    private final TestStub testStub;

    CallTests(TestStub testStub, MaeUnitLogger logger) {
      this.testStub = testStub;
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
      return testStub.runNativeTests(logger);
    }
  }

  public Integer executorRun(MaeUnitLogger logger) throws ExecutionException, InterruptedException {
    ExecutorService executorService = Executors.newFixedThreadPool(2);

    FutureTask<Integer> tests = new FutureTask<Integer>(new CallTests(this, logger));
    executorService.execute(tests);
    return tests.get();
  }

  public native int runNativeTests(MaeUnitLogger logger);
}
