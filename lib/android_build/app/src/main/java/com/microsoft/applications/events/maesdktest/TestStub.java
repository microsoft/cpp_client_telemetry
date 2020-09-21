package com.microsoft.applications.events.maesdktest;

import android.content.ContentResolver;
import android.net.Uri;
import com.microsoft.applications.events.ILogger;
import com.microsoft.applications.events.LogManager;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.Arrays;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.FutureTask;

public class TestStub {
  static Integer sendEventCount = 0;

  enum StatisticsFlavor {
    FROM_JAVA,
    FROM_NATIVE,
    PAIRED
  }

  class CallTests implements Callable<Integer> {
    MaeUnitLogger logger;
    UnitTestViewModel unitTestViewModel;

    CallTests(MaeUnitLogger logger, UnitTestViewModel unitTestViewModel) {
      this.logger = logger;
      this.unitTestViewModel = unitTestViewModel;
    }

    /**
     * Computes a result, or throws an exception if unable to do so.
     *
     * @return computed result
     * @throws Exception if unable to compute a result
     */
    @Override
    public Integer call() {
      int rc = runNativeTests(logger);
      unitTestViewModel.setStatus(String.format("Tests complete, rc=%d", rc));
      unitTestViewModel.setRunning(false);
      return Integer.valueOf(rc);
    }
  }

  class SendEvent implements Callable<Integer> {
    final String token =
        "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123";

    UnitTestViewModel unitTestViewModel;
    ILogger logger;

    SendEvent(UnitTestViewModel unitTestViewModel) {
      this.unitTestViewModel = unitTestViewModel;
    }

    public Integer call() {
      unitTestViewModel.setEventStatus("Woo Hoo");
      logger = LogManager.initialize(token);
      logger.logEvent("testEvent");
      Integer result;

      synchronized (sendEventCount) {
        result = (sendEventCount += 1);
      }
      unitTestViewModel.setEventStatus(String.format("Sent %d instances of testEvent", result));
      unitTestViewModel.setRunning(false);
      return result;
    }
  }

  class CollectStats implements Callable<Boolean> {

    OutputStream outputStream;
    ContentResolver contentResolver;
    Uri uri;
    UnitTestViewModel unitTestViewModel;
    boolean pairObservations;

    CollectStats(
        ContentResolver contentResolver,
        Uri uri,
        UnitTestViewModel unitTestViewModel,
        boolean pairObservations) {
      this.contentResolver = contentResolver;
      this.uri = uri;
      this.unitTestViewModel = unitTestViewModel;
      this.pairObservations = pairObservations;
    }

    private double mean(long[] sample) {
      double sum = 0.0;
      for (long i : sample) {
        sum += i;
      }
      return sum / sample.length;
    }

    private double variance(long[] sample) {
      double mean = mean(sample);
      double sum = 0.0;
      for (long i : sample) {
        double diff = i - mean;
        sum += diff * diff;
      }
      return sum / (sample.length - 1);
    }

    class Ranky implements Comparable<Ranky> {
      long ns;
      boolean isNative;

      Ranky(long ns, boolean isNative) {
        this.ns = ns;
        this.isNative = isNative;
      }

      /**
       * Compares this object with the specified object for order. Returns a negative integer, zero,
       * or a positive integer as this object is less than, equal to, or greater than the specified
       * object.
       *
       * <p>The implementor must ensure <tt>sgn(x.compareTo(y)) == -sgn(y.compareTo(x))</tt> for all
       * <tt>x</tt> and <tt>y</tt>. (This implies that <tt>x.compareTo(y)</tt> must throw an
       * exception iff <tt>y.compareTo(x)</tt> throws an exception.)
       *
       * <p>The implementor must also ensure that the relation is transitive:
       * <tt>(x.compareTo(y)&gt;0 &amp;&amp; y.compareTo(z)&gt;0)</tt> implies
       * <tt>x.compareTo(z)&gt;0</tt>.
       *
       * <p>Finally, the implementor must ensure that <tt>x.compareTo(y)==0</tt> implies that
       * <tt>sgn(x.compareTo(z)) == sgn(y.compareTo(z))</tt>, for all <tt>z</tt>.
       *
       * <p>It is strongly recommended, but <i>not</i> strictly required that
       * <tt>(x.compareTo(y)==0) == (x.equals(y))</tt>. Generally speaking, any class that
       * implements the <tt>Comparable</tt> interface and violates this condition should clearly
       * indicate this fact. The recommended language is "Note: this class has a natural ordering
       * that is inconsistent with equals."
       *
       * <p>In the foregoing description, the notation <tt>sgn(</tt><i>expression</i><tt>)</tt>
       * designates the mathematical <i>signum</i> function, which is defined to return one of
       * <tt>-1</tt>, <tt>0</tt>, or <tt>1</tt> according to whether the value of <i>expression</i>
       * is negative, zero or positive.
       *
       * @param o the object to be compared.
       * @return a negative integer, zero, or a positive integer as this object is less than, equal
       *     to, or greater than the specified object.
       * @throws NullPointerException if the specified object is null
       * @throws ClassCastException if the specified object's type prevents it from being compared
       *     to this object.
       */
      @Override
      public int compareTo(Ranky o) {
        return (int) Math.signum(this.ns - o.ns);
      }
    }

    private double uMannWhitneyWilcoxon(long[] nSample, long[] jSample) {
      Ranky[] rankies = new Ranky[nSample.length + jSample.length];
      for (int i = 0; i < nSample.length; ++i) {
        rankies[i] = new Ranky(nSample[i], true);
      }
      for (int i = nSample.length; i < jSample.length + nSample.length; ++i) {
        rankies[i] = new Ranky(jSample[i - nSample.length], false);
      }
      Arrays.sort(rankies);
      long sum = 0;
      for (int i = 0; i < rankies.length; ++i) {
        if (rankies[i].isNative) {
          sum += i + 1;
        }
      }
      return sum - 0.5 * (nSample.length * (nSample.length + 1));
    }

    /**
     * Computes a result, or throws an exception if unable to do so.
     *
     * @return computed result
     * @throws Exception if unable to compute a result
     */
    @Override
    public Boolean call() throws Exception {
      PrintStream printStream = new PrintStream(contentResolver.openOutputStream(uri));
      long[] javaStats;
      long[] nativeStats;
      if (pairObservations) {
        long[] pairedStats = generatePairedObservation();
        int count = pairedStats.length >> 1;
        nativeStats = Arrays.copyOfRange(pairedStats, 0, count);
        javaStats = Arrays.copyOfRange(pairedStats, count, count + count);
      } else {
        javaStats = generateStatistics(true);
        nativeStats = generateStatistics(false);
      }

      unitTestViewModel.setStatus("Writing File");
      printStream.printf(",Native,Java\n");
      assert (nativeStats.length == javaStats.length);
      for (int i = 0; i < nativeStats.length; ++i) {
        printStream.printf(",%d,%d\n", nativeStats[i], javaStats[i]);
      }
      double[] mean = {mean(nativeStats), mean(javaStats)};
      double[] variance = {variance(nativeStats), variance(javaStats)};
      printStream.printf("mean,%g,%g\n", mean[0], mean[1]);
      printStream.printf("sd,%g,%g\n", Math.sqrt(variance[0]), Math.sqrt(variance[1]));
      double denom = variance[0] / nativeStats.length + variance[1] / javaStats.length;
      double t = (mean[0] - mean[1]) / Math.sqrt(denom);
      double nu =
          (denom * denom)
              / ((variance[0]
                      * variance[0]
                      / (nativeStats.length * nativeStats.length * (nativeStats.length - 1)))
                  + ((variance[1]
                      * variance[1]
                      / (javaStats.length * javaStats.length * (javaStats.length - 1)))));
      printStream.printf("t,%g\n", t);
      printStream.printf("nu,%g\n", nu);
      double u1 = uMannWhitneyWilcoxon(nativeStats, javaStats);
      printStream.printf("u1,%g\n", u1);
      double mu = (nativeStats.length * javaStats.length) / 2;
      double n1 = nativeStats.length;
      double n2 = javaStats.length;
      double su = Math.sqrt((n1 * n2 * (n1 + n2 + 1)) / 12.0);
      printStream.printf("z1,%g\n", (u1 - mu) / su);
      boolean ruhRohRaggy = printStream.checkError();
      printStream.close();
      unitTestViewModel.setStatus("File Complete");
      unitTestViewModel.setRunning(false);
      return !ruhRohRaggy;
    }

    private void showIteration(StatisticsFlavor source, long iteration, long total, double mean) {
      String flavorName = "Broken";
      switch (source) {
        case FROM_JAVA:
          flavorName = "Java";
          break;
        case FROM_NATIVE:
          flavorName = "Native";
          break;
        case PAIRED:
          flavorName = "Paired";
          break;
      }
      unitTestViewModel.setStatus(
          String.format(
              "%s iteration %d of %d mean %g",
              flavorName, iteration, total, mean));
    }

    public native long[] generateStatistics(boolean fromJava);

    public native long[] generatePairedObservation();
  }

  ExecutorService executorService = Executors.newFixedThreadPool(2);

  public FutureTask<Integer> executorRun(
      MaeUnitLogger logger, UnitTestViewModel unitTestViewModel) {
    FutureTask<Integer> tests = new FutureTask<Integer>(new CallTests(logger, unitTestViewModel));
    executorService.execute(tests);
    return tests;
  }

  public FutureTask<Boolean> collectStatistics(
      ContentResolver contentResolver,
      Uri uri,
      UnitTestViewModel unitTestViewModel,
      boolean pairedObservations) {
    FutureTask<Boolean> stats =
        new FutureTask(
            new CollectStats(contentResolver, uri, unitTestViewModel, pairedObservations));
    executorService.execute(stats);
    return stats;
  }

  public FutureTask<Integer> sendAnEvent(UnitTestViewModel unitTestViewModel) {
    FutureTask<Integer> sendEvent = new FutureTask(new SendEvent(unitTestViewModel));
    executorService.execute(sendEvent);
    return sendEvent;
  }

  public native int runNativeTests(MaeUnitLogger logger);
}
