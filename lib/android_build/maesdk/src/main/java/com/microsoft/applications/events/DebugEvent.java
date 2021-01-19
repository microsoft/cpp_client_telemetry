package com.microsoft.applications.events;

public class DebugEvent {
  /// <summary>The debug event sequence number.</summary>
  public long seq = 0;
  /// <summary>The debug event timestamp.</summary>
  public long ts = 0;
  /// <summary>The debug event type.</summary>
  public DebugEventType type = DebugEventType.EVT_UNKNOWN;
  /// <summary>[optional] Parameter 1 (depends on debug event type).</summary>
  long param1 = 0;
  /// <summary>[optional] Parameter 2 (depends on debug event type).</summary>
  long param2 = 0;
  /// <summary>[optional] The debug event data (depends on debug event type).</summary>
  Object data = null;
  /// <summary>[optional] The size of the debug event data (depends on debug event type).</summary>
  long size = 0;

  public DebugEvent(
      long seq, long ts, long type, long param1, long param2, Object data, long size) {
    this.seq = seq;
    this.ts = ts;
    if (DebugEventType.eventMap.containsKey(type)) {
      this.type = DebugEventType.eventMap.get(type);
    } else {
      this.type = DebugEventType.EVT_UNKNOWN;
    }
    this.param1 = param1;
    this.param2 = param2;
    this.data = data;
    this.size = size;
  }
}
