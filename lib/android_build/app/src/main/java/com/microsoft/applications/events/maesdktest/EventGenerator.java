package com.microsoft.applications.events.maesdktest;

import androidx.annotation.Keep;
import com.microsoft.applications.events.EventProperties;
import com.microsoft.applications.events.ILogger;
import com.microsoft.applications.events.LogManager;
import com.microsoft.applications.events.ILogger;

@Keep
public class EventGenerator {
  static ILogger logger = null;

  public static void useWrapperLogger() {
    logger = LogManager.initialize("0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
  }

  public static void logEvents(ILogger l, long numberToGenerate) {
    if (l == null) {
      useWrapperLogger();
      l = logger;
    }
    for (long i = numberToGenerate; i > 0; --i) {
      l.logEvent("TestEvent");
    }
  }
}
