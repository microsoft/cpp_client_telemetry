package com.microsoft.applications.events;

public class SubstrateSignals {
    /**
     * Creates EventProperties object from json SignalItem.
     *
     * @param loggerPtr the ILogger pointer
     * @param signalItemJson the Signal Item in json format.
     */
    public static native boolean sendSignal(long loggerPtr, String signalItemJson);

    public static native boolean isInitialized();

    public static boolean initialize() {
        return initialize(new SubstrateSignalsOptions());
    }

    public static boolean initialize(SubstrateSignalsOptions options) {
        return nativeInitialize(options.baseUrl, options.timeoutMs, options.retryTimes, options.retryTimesToWait, options.retryStatusCodes);
    }

    private static native boolean nativeInitialize(String baseUrl, int timeoutMs, int retryTimes, int retryTimeToWait, int[] retryStatusCodes);
    public static native boolean nativeUnitialize();

    public static native boolean isEnabled();
    public static native boolean setEnabled(boolean enabled);
}
