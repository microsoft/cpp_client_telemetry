package com.microsoft.applications.events;

public enum DiagLevel {
  DIAG_LEVEL_REQUIRED(1),
  DIAG_LEVEL_OPTIONAL(2),
  DIAG_LEVEL_RSD(110),
  DIAG_LEVEL_RSDES(120);

  private int value;

  DiagLevel(int value) {
    this.value = value;
  }
  public int value() { return this.value; }
}
