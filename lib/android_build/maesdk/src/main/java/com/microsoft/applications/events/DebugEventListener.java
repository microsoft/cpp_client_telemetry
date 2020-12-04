package com.microsoft.applications.events;

abstract public class DebugEventListener {
  abstract public void onDebugEvent(DebugEvent evt);

  public long nativeIdentity = -1;
}
