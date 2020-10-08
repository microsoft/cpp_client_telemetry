//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package main

import (
	"fmt"
	"strconv"
	"time"
	"Microsoft/Telemetry/EventLogger"
	"runtime"
	"os"
	"os/signal"
	"syscall"
	"runtime/debug"
)

func getuptime() int64 {
    return time.Now().UnixNano() / int64(time.Millisecond)
}

func printMemoryStats() {
	var m runtime.MemStats
	runtime.ReadMemStats(&m)
	fmt.Printf("\tAlloc = %10vK\tTotalAlloc = %10vK\tSys = %10vK\tNumGC = %10v\n", m.Alloc / 1024, m.TotalAlloc / 1024, m.Sys / 1024, m.NumGC)
}


/* This code example shows how to register a signal handler routine in GoLang.
 * In case if native code experiences an issue, the signal can be trapped here
 * and decision made what to do next - restart the process or attempt to recover
 * by reinitializing Aria, etc.
 */
func registerSignalHandler() {
	go func() {
	    sigs := make(chan os.Signal, 1)
	    signal.Notify(sigs, syscall.SIGABRT, syscall.SIGTERM)
	    buf := make([]byte, 1<<20)
	    for {
		<-sigs
		stacklen := runtime.Stack(buf, true)
		// Print out detailed stack info to facilitate debugging
		fmt.Printf("=== received terminate ===\n*** goroutine dump...\n%s\n*** end\n", buf[:stacklen])
		debug.PrintStack()
		os.Exit(-1)
	    }
	}()
}

func main() {
	// trap SIGABRT and SIGTERM
	registerSignalHandler()

	// 1DSCppSdkTest sandbox key
	token  := "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx-xxxx"

	fmt.Println("Hello from Microsoft Telemetry-Go!\n")
	logger := EventLogger.NewEventLogger()
	logger.Init(token)
	done := false
        seq := 0
	for done != true {
	    uptime_start := getuptime()
	    for i := 0 ; i<10000; i++ {
		event := EventLogger.NewStringMap()
		event.Set("name", "GoLangTest")
		event.Set("key1", "value1")
		event.Set("key2", "value2")
		event.Set("seq", strconv.Itoa(i))
		logger.LogEvent(event)
		/* no defer!! */ EventLogger.DeleteStringMap(event)
	    }
	    uptime_end := getuptime()
	    delta := (uptime_end - uptime_start)
	    fmt.Printf("[%d] sent a batch of 10000 events in %d ms\n", seq, delta)
	    printMemoryStats()
	    time.Sleep(time.Second)
	    seq++
	}

	logger.Done()

}
