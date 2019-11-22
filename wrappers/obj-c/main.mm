#import <Foundation/Foundation.h>
#import "ODWLogManager.h"
#import "ODWLogger.h"
#import "ODWEventProperties.h"

int main(int argc, char** argv){
    @autoreleasepool{
        // 1DSCppSdkTest sandbox key. Replace with your own iKey!
        NSString* token = @"7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991";
        
        ODWLogger* myLogger = [ODWLogManager loggerWithTenant: token];
        if(myLogger){
            [myLogger logEventWithName: @"Simple_ObjC_Event"];
        }
        [ODWLogManager uploadNow];

        ODWEventProperties* event = [[ODWEventProperties alloc] initWithName: @"WEvtProps_ObjC_Event"
                                                           properties: @{
                                                                            @"result": @"Success", 
                                                                            @"seq": @2, 
                                                                            @"random": @3,
                                                                            @"secret": @5.75 
                                                                            } ];

        ODWLogger* logger2 = [ODWLogManager loggerForSource: @"source2"];
        if(logger2){
            [logger2 logEventWithEventProperties: event];
        }
        [ODWLogManager uploadNow];

        ODWEventProperties* event2 = [[ODWEventProperties alloc] initWithName:@"SetProps_ObjC_Event"];
        [event2 setProperty: @"result" withValue: @"Failure"];
        [event2 setProperty: @"intVal" withInt64Value: (int64_t)8165];
        [event2 setProperty: @"doubleVal" withDoubleValue: (double)1.24];
        [event2 setProperty: @"wasSuccessful" withBoolValue: YES];

        [logger2 logEventWithEventProperties: event2];

        [ODWLogManager flushAndTeardown];
    }
    return 0;
}
