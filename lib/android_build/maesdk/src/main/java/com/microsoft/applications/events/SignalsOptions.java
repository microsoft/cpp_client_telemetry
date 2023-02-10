package com.microsoft.applications.events;

public class SignalsOptions {
    public String baseUrl = "";
    public int timeoutMs = 90000;
    public int retryTimes = 3;
    public int retryTimesToWait = 3000;
    public int[] retryStatusCodes = {429, 500, 503, 504, 507, 0};
}
