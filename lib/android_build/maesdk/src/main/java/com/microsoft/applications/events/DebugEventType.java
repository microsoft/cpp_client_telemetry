package com.microsoft.applications.events;

import java.util.TreeMap;

public enum DebugEventType {
  /// <summary>API call: logEvent.</summary>
  EVT_LOG_EVENT(0x01000000L),
  /// <summary>API call: logAppLifecycle.</summary>
  EVT_LOG_LIFECYCLE(0x01000001L),
  /// <summary>API call: logFailure.</summary>
  EVT_LOG_FAILURE(0x01000002L),
  /// <summary>API call: logPageView.</summary>
  EVT_LOG_PAGEVIEW(0x01000004L),
  /// <summary>API call: logPageAction.</summary>
  EVT_LOG_PAGEACTION(0x01000005L),
  /// <summary>API call: logSampledMetric.</summary>
  EVT_LOG_SAMPLEMETR(0x01000006L),
  /// <summary>API call: logAggregatedMetric.</summary>
  EVT_LOG_AGGRMETR(0x01000007L),
  /// <summary>API call: logTrace.</summary>
  EVT_LOG_TRACE(0x01000008L),
  /// <summary>API call: logUserState.</summary>
  EVT_LOG_USERSTATE(0x01000009L),
  /// <summary>API call: logSession.</summary>
  EVT_LOG_SESSION(0x0100000AL),

  /// <summary>Event(s) added to queue.</summary>
  EVT_ADDED(0x01001000L),
  /// <summary>Event(s) cached in offline storage.</summary>
  EVT_CACHED(0x02000000L),
  /// <summary>Event(s) dropped.</summary>
  EVT_DROPPED(0x03000000L),
  /// <summary>Event(s) filtered.</summary>
  EVT_FILTERED(0x03000001L),

  /// <summary>Event(s) sent.</summary>
  EVT_SENT(0x04000000L),
  /// <summary>Event(s) being uploaded.</summary>
  EVT_SENDING(0x04000000L),
  /// <summary>Event(s) send failed.</summary>
  EVT_SEND_FAILED(0x04000001L),
  /// <summary>Event(s) send retry.</summary>
  EVT_SEND_RETRY(0x04000002L),
  /// <summary>Event(s) retry drop.</summary>
  EVT_SEND_RETRY_DROPPED(0x04000003L),
  /// <summary>Event(s) skip UTC registration.</summary>
  EVT_SEND_SKIP_UTC_REGISTRATION(0x04000004L),
  /// <summary>Event(s) rejectedL), e.g.
  /// Failed regexp check or missing event name.
  /// </summary>
  EVT_REJECTED(0x05000000L),

  /// <summary>HTTP client state events.</summary>
  EVT_HTTP_STATE(0x09000000L),

  /// BEGIN: deprecated events group
  ///
  /// Events below might have been reported by previous
  /// version of SDK (Aria-v1)L), and/or by earlier versions
  /// of 1DS C++ SDK. Please use generic EVT_HTTP_STATE
  /// event instead.
  ///
  /// <summary>HTTP stack failure.</summary>
  EVT_CONN_FAILURE(0x0A000000L),
  /// <summary>HTTP stack failure.</summary>
  EVT_HTTP_FAILURE(0x0A000001L),
  /// <summary>Compression failed.</summary>
  EVT_COMPRESS_FAILED(0x0A000002L),
  /// <summary>HTTP stack unknown host.</summary>
  EVT_UNKNOWN_HOST(0x0A000003L),
  /// END: deprecated events group

  /// <summary>HTTP response error.</summary>
  EVT_HTTP_ERROR(0x0B000000L),
  /// <summary>HTTP response 200 OK.</summary>
  EVT_HTTP_OK(0x0C000000L),

  /// <summary>Network state change.</summary>
  EVT_NET_CHANGED(0x0D000000L),

  /// <summary>Storage full.</summary>
  EVT_STORAGE_FULL(0x0E000000L),
  /// <summary>Storage failed.</summary>
  EVT_STORAGE_FAILED(0x0E000001L),

  /// <summary>Ticket Expired</summary>
  EVT_TICKET_EXPIRED(0x0F000000L),
  /// <summary>Unknown error.</summary>
  EVT_UNKNOWN(0xDEADBEEFL);

  private long value;

  DebugEventType(long value) {
    this.value = value;
  }

  public long value() {
    return this.value;
  }

  static final TreeMap<Long, DebugEventType> eventMap = new TreeMap<Long, DebugEventType>();

  static {
    for (DebugEventType t : DebugEventType.values()) {
      eventMap.put(t.value(), t);
    }
  }
}
