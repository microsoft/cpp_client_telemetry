//
// Copyright (c) Microsoft Corporation. All rights reserved.
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

