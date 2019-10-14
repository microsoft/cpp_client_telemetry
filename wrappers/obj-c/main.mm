#import <Foundation/Foundation.h>
#import "WLogManager.h"
#import "WLogger.h"
#import "WEventProperties.h"

int main(int argc, char** argv){
    @autoreleasepool{
        // 1DSCppSdkTest sandbox key. Replace with your own iKey!
        NSString* token = @"7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991";
        
        WLogger* myLogger = [WLogManager initForTenant: token];
        if(myLogger){
            [myLogger logEventWithName: @"Simple_ObjC_Event"];
        }
        [WLogManager uploadNow];

        WEventProperties* event = [[WEventProperties alloc] initWithName: @"WEvtProps_ObjC_Event" 
                                                           andProperties: @{
                                                                            @"result": @"Success", 
                                                                            @"seq": @2, 
                                                                            @"random": @3,
                                                                            @"secret": @5.75 
                                                                            } ];

        WLogger* logger2 = [WLogManager getLoggerForSource: @"source2"];
        if(logger2){
            [logger2 logEventWithWEventProperties: event];
        }
        [WLogManager uploadNow];

        WEventProperties* event2 = [[WEventProperties alloc] init];
        [event2 setName: @"SetProps_ObjC_Event"];
        [event2 setPropertyWithName: @"result" withStringValue: @"Failure"];
        [event2 setPropertyWithName: @"intVal" withInt64Value: (int64_t)8165];
        [event2 setPropertyWithName: @"doubleVal" withDoubleValue: (double)1.24];
        [event2 setPropertyWithName: @"wasSuccessful" withBoolValue: YES];

        [logger2 logEventWithWEventProperties: event2];

        [WLogManager flushAndTeardown];
    }
    return 0;
}
