package com.microsoft.applications.events;

import java.util.List;
import java.util.Vector;

public class CommonDataContext {
    /**
    * Domain Name for the current machine
    */
    public String DomainName;

    /**
    * Friendly Machine Name
    */
    public String MachineName;

    /**
    * Unique UserName such as the log-in name
    */
    public String UserName;

    /**
    * Unique User Alias, if different than UserName
    */
    public String UserAlias;

    /**
    * IP Addresses for local network ports such as IPv4, IPv6, etc.
    */
    Vector<String> IpAddresses;

    /**
    * Collection of Language identifiers
    */
    Vector<String> LanguageIdentifiers;

    /**
    * Collection of Machine ID such as Machine Name, Motherboard ID, MAC Address, etc.
    */
    Vector<String> MachineIds;

    /**
    * Collection of OutOfScope Identifiers such as SQM_ID, Client_ID, etc.
    */
    Vector<String> OutOfScopeIdentifiers;
}
