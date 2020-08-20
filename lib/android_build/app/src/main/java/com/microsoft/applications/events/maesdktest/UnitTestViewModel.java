package com.microsoft.applications.events.maesdktest;

import android.content.ContentResolver;
import android.net.Uri;
import androidx.annotation.Nullable;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
import java.io.OutputStream;
import java.util.concurrent.Future;

public class UnitTestViewModel extends ViewModel {
  class DummyLogger extends MaeUnitLogger {

    @Override
    void log_failure(String filename, int line, String summary) {
      testData.postValue(summary);
    }
  }

  private final MutableLiveData<String> testData = new MutableLiveData<String>();
  private final MutableLiveData<Boolean> runningData = new MutableLiveData<Boolean>();
  private final TestStub stub = new TestStub();
  private Future<Integer> testFuture;
  private Future<Boolean> statsFuture;

  public UnitTestViewModel() {
    setRunning(false);
  }

  public LiveData<String> getUnitTestStatus() {
    return testData;
  }

  public LiveData<Boolean> getRunning() {
    return runningData;
  }

  public void setStatus(@Nullable String newStatus) {
    testData.postValue(newStatus);
  }

  public void setRunning(boolean running) {
    runningData.postValue(running);
  }

  public void doTest() {
    if (testFuture != null) {
      if (testFuture.isDone()) {
        testFuture = null;
      } else {
        testData.postValue("Still running");
        return;
      }
    }
    testData.postValue("Running");
    runningData.postValue(true);
    DummyLogger dummyLogger = new DummyLogger();
    testFuture = stub.executorRun(dummyLogger, this);
  }

  public void gatherStatistics(ContentResolver contentResolver, Uri uri) {
    if (statsFuture != null && !statsFuture.isDone()) {
      return;
    }
    statsFuture = null;
    testData.postValue("Gathering Statistics");
    runningData.postValue(true);
    statsFuture = stub.collectStatistics(contentResolver, uri, this);
  }
}
