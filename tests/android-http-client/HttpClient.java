//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.FutureTask;

// A deterministic Java peer for the production C++ client, without network or AndroidX dependencies.
public final class HttpClient {
    private final ArrayList<WeakReference<FutureShim>> tasks = new ArrayList<>();
    private final ExecutorService executor = Executors.newSingleThreadExecutor();
    private int mode;

    private final class FutureShim extends FutureTask<Boolean> {
        FutureShim(String id) {
            super(() -> {
                dispatchCallback(id, mode == 1 ? -1 : 200, new String[0], new byte[0]);
                return true;
            });
        }

        @Override
        public boolean cancel(boolean interrupt) {
            if (mode == 9) {
                throw new IllegalStateException("Injected cancel failure");
            }
            return super.cancel(interrupt);
        }
    }

    public FutureTask<Boolean> createTask(
            String url, String method, byte[] body, String id, int[] lengths, byte[] headers) {
        FutureShim task = new FutureShim(id);
        tasks.add(new WeakReference<>(task));
        if (mode == 5) {
            cancelPreparing(id);
        }
        if (mode == 6) {
            throw new IllegalStateException("Injected create failure");
        }
        return mode == 12 ? null : task;
    }

    public void executeTask(FutureTask<Boolean> task) {
        if (mode == 7) {
            throw new IllegalStateException("Injected execute failure");
        }
        if (mode == 0 || mode == 1 || mode == 8) {
            task.run();
        } else if (mode == 10 || mode == 11) {
            executor.execute(task);
        }
    }

    private native void dispatchCallback(String id, int status, String[] headers, byte[] body);
    private native void cancelPreparing(String id);
    private native void startBatch(int mode, int count);
    private native boolean finishBatch();

    public static void main(String[] args) throws Exception {
        System.load(args[0]);
        HttpClient client = new HttpClient();
        String[] cases = {"success", "network-failure", "cancel", "cancel-all", "early-cancel",
                "preparing-cancel", "create-exception", "execute-exception",
                "callback-deletes-request", "cancel-exception", "async-success", "cancel-completion-race",
                "null-task"};
        String selected = args.length > 1 ? args[1] : "all";
        int stressCount = args.length > 2 ? Integer.parseInt(args[2]) : 60000;
        boolean ran = false;
        try {
            for (int mode = 0; mode < cases.length; ++mode) {
                if (!selected.equals("all") && !selected.equals(cases[mode])) {
                    continue;
                }
                ran = true;
                client.mode = mode;
                client.tasks.clear();
                client.startBatch(mode, mode == 0 ? stressCount : mode == 6 || mode == 7 || mode == 9 ? 4 : 1024);
                int live = 0;
                for (int attempt = 0; attempt < 20; ++attempt) {
                    System.gc();
                    System.runFinalization();
                    Thread.sleep(50);
                    live = 0;
                    for (WeakReference<FutureShim> task : client.tasks) {
                        if (task.get() != null) {
                            ++live;
                        }
                    }
                    if (live == 0) {
                        break;
                    }
                }
                boolean callbacksPassed = client.finishBatch();
                if (!callbacksPassed || live != 0) {
                    throw new AssertionError(cases[mode] + ": callbacks=" + callbacksPassed + ", retained=" + live);
                }
                System.out.println("PASS " + cases[mode] + ": retained FutureShim objects=" + live);
            }
        } finally {
            client.executor.shutdownNow();
        }
        if (!ran) {
            throw new IllegalArgumentException("Unknown test case: " + selected);
        }
    }
}
