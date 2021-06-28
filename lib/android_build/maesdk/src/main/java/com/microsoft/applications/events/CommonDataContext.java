//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import java.util.Vector;

/**
 * `CommonDataContexts` provide a convenient struct to convey common information to
 * detect in the data being uploaded. This is expected to be internal information that
 * the host application may have access to, but do not wish to be uploaded via the 1DS
 * SDK path.
 */
public class CommonDataContext {
    /**
    * Domain Name for the current machine
    */
    public String domainName = "";

    /**
    * Friendly Machine Name
    */
    public String machineName = "";

    /**
     * List of multiple usernames for multi-user scenarios.
     */
    public Vector<String> userNames = new Vector<>();

    /**
     * List of multiple User Aliases for multi-user scenarios.
     */
    public Vector<String> userAliases = new Vector<>();
    /**
    * IP Addresses for local network ports such as IPv4, IPv6, etc.
    */
    public Vector<String> ipAddresses = new Vector<>();

    /**
    * Collection of Language identifiers
    */
    public Vector<String> languageIdentifiers = new Vector<>();

    /**
    * Collection of Machine ID such as Machine Name, Motherboard ID, MAC Address, etc.
    */
    public Vector<String> machineIds = new Vector<>();

    /**
    * Collection of OutOfScope Identifiers such as SQM_ID, Client_ID, etc.
    */
    public Vector<String> outOfScopeIdentifiers = new Vector<>();
}

