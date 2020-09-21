package com.microsoft.applications.events.maesdktest;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModelProvider;
import com.microsoft.applications.events.HttpClient;
import com.microsoft.applications.events.OfflineRoom;
import com.microsoft.applications.events.maesdktest.databinding.ActivityMainBinding;
import java.util.concurrent.FutureTask;

public class MainActivity extends AppCompatActivity {
  protected enum RequestCode {
    CREATE_FILE
  }

  // Used to load the 'native-lib' library on application startup.
  static {
    System.loadLibrary("native-lib");
    System.loadLibrary("maesdk");
  }

  public void mwwClick(View view) {
    createFile();
  }

  public void sendEventClick(View view) {
    unitTestViewModel.sendAnEvent();
  }

  public void runUnitTests(View view) {
    unitTestViewModel.doTest();
  }

  private UnitTestViewModel unitTestViewModel;

  FutureTask<Integer> unitTests;

  private ActivityMainBinding binding;

  private void createFile() {
    Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
    intent.addCategory(Intent.CATEGORY_OPENABLE);
    intent.setType("text/csv");
    intent.putExtra(Intent.EXTRA_TITLE, "LogEvent.csv");

    startActivityForResult(intent, RequestCode.CREATE_FILE.ordinal());
  }

  protected void onCreateFile(int resultCode, Intent data) {
    if (resultCode != Activity.RESULT_OK) {
      return;
    }
    Uri uri = null;
    assert (data != null);
    if (data == null) {
      return;
    }
    uri = data.getData();
    if (uri != null) {
      unitTestViewModel.gatherStatistics(getContentResolver(), uri,
          binding.pairSwitch.isChecked());
    }
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    super.onActivityResult(requestCode, resultCode, data);
    RequestCode request = RequestCode.values()[requestCode];
    switch (request) {
      case CREATE_FILE:
        onCreateFile(resultCode, data);
        break;
      default:
        throw new Error(String.format("Unimplemented request %s", request.toString()));
    }
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    binding = ActivityMainBinding.inflate(getLayoutInflater());
    setContentView(binding.getRoot());

    m_client = new HttpClient(getApplicationContext());
    OfflineRoom.connectContext(getApplicationContext());
    TestStub testStub = new TestStub();
    final ViewModelProvider.Factory factory = new ViewModelProvider.NewInstanceFactory();
    final ViewModelProvider provider = new ViewModelProvider(this, factory);
    unitTestViewModel = provider.get(UnitTestViewModel.class);

    // Example of a call to a native method

    unitTestViewModel
        .getUnitTestStatus()
        .observe(
            this,
            new Observer<String>() {

              /**
               * Called when the data is changed.
               *
               * @param s The new data
               */
              @Override
              public void onChanged(String s) {
                binding.sampleText.setText(s);
              }
            });
    unitTestViewModel
        .getRunning()
        .observe(
            this,
            new Observer<Boolean>() {

              /**
               * Called when the data is changed.
               *
               * @param running The new data
               */
              @Override
              public void onChanged(Boolean running) {
                binding.textProgress.setVisibility(running ? View.VISIBLE : View.INVISIBLE);
              }
            });
    unitTestViewModel
        .getSendEventStatus()
        .observe(
            this,
            new Observer<String>() {
              @Override
              public void onChanged(String status) {
                binding.eventText.setText(status);
              }
            }
        );
  }

  /**
   * A native method that is implemented by the 'native-lib' native library, which is packaged with
   * this application.
   */
  public native String stringFromJNI(String path);

  HttpClient m_client;
}
