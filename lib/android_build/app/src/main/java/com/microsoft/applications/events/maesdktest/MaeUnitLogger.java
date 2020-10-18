//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events.maesdktest;

import androidx.annotation.Keep;

@Keep
public abstract class MaeUnitLogger {
    abstract void log_failure(String filename, int line, String summary);
}

