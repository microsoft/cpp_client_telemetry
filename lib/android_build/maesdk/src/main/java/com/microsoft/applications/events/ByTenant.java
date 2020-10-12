//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import androidx.annotation.Keep;

@Keep
public class ByTenant {
    public String tenantToken;
    public long count;

    public ByTenant(String tenantToken, Long count)
    {
        this.tenantToken = tenantToken;
        this.count = count;
    }
}

