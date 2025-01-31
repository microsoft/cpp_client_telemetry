/*
 Copyright (c) 2011, Tony Million.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#import "ODWReachability.h"

#import <sys/socket.h>
#import <netinet/in.h>
#import <netinet6/in6.h>
#import <arpa/inet.h>
#import <ifaddrs.h>
#import <netdb.h>
#import <Foundation/Foundation.h>


NSString *const kNetworkReachabilityChangedNotification = @"NetworkReachabilityChangedNotification";


@interface ODWReachability ()

@property (nonatomic, assign) SCNetworkReachabilityRef  reachabilityRef;
@property (nonatomic, strong) dispatch_queue_t          reachabilitySerialQueue;
@property (nonatomic, strong) id                        reachabilityObject;

-(void)reachabilityChanged:(SCNetworkReachabilityFlags)flags;
-(BOOL)isReachableWithFlags:(SCNetworkReachabilityFlags)flags;

@end


static NSString *reachabilityFlags(SCNetworkReachabilityFlags flags)
{
    return [NSString stringWithFormat:@"%c%c %c%c%c%c%c%c%c",
#if TARGET_OS_IPHONE
            (flags & kSCNetworkReachabilityFlagsIsWWAN)               ? 'W' : '-',
#else
            'X',
#endif
            (flags & kSCNetworkReachabilityFlagsReachable)            ? 'R' : '-',
            (flags & kSCNetworkReachabilityFlagsConnectionRequired)   ? 'c' : '-',
            (flags & kSCNetworkReachabilityFlagsTransientConnection)  ? 't' : '-',
            (flags & kSCNetworkReachabilityFlagsInterventionRequired) ? 'i' : '-',
            (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic)  ? 'C' : '-',
            (flags & kSCNetworkReachabilityFlagsConnectionOnDemand)   ? 'D' : '-',
            (flags & kSCNetworkReachabilityFlagsIsLocalAddress)       ? 'l' : '-',
            (flags & kSCNetworkReachabilityFlagsIsDirect)             ? 'd' : '-'];
}

// Start listening for reachability notifications on the current run loop
static void TMReachabilityCallback(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void* info)
{
#pragma unused (target)

    ODWReachability *reachability = ((__bridge ODWReachability*)info);

    // We probably don't need an autoreleasepool here, as GCD docs state each queue has its own autorelease pool,
    // but what the heck eh?
    @autoreleasepool
    {
        [reachability reachabilityChanged:flags];
    }
}


@implementation ODWReachability

static int kTimeoutDurationInSeconds = 10;

#pragma mark - Class Constructor Methods

+(ODWReachability*)reachabilityWithHostName:(NSString*)hostname
{
    if (hostname == nil || [hostname length] == 0)
    {
        NSLog(@"Invalid hostname");
        return nil;
    }
    return [ODWReachability reachabilityWithHostname:hostname];
}

+(instancetype)reachabilityWithHostname:(NSString*)hostname
{
    if (@available(macOS 10.14, iOS 12.0, *))
    {
        // Use URLSession for macOS 10.14 or higher
        NSString *formattedHostname = hostname;
        if (![formattedHostname hasPrefix:@"https://"] && ![formattedHostname hasPrefix:@"http://"]) {
            formattedHostname = [NSString stringWithFormat:@"https://%@", hostname];
        }
        NSURL *url = [NSURL URLWithString:formattedHostname];

        NSURLSession *session = [NSURLSession sharedSession];
        __block ODWReachability *reachabilityInstance = [[self alloc] init];
        reachabilityInstance.url = url;
        NSURLSessionDataTask *dataTask = [session dataTaskWithURL:url completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
            reachabilityInstance = [self handleReachabilityResponse:response error:error url:reachabilityInstance.url];
        }];
        [dataTask resume];
        return reachabilityInstance;
    }

    // Use SCNetworkReachability for macOS 10.14 or lower
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    SCNetworkReachabilityRef ref = SCNetworkReachabilityCreateWithName(NULL, [hostname UTF8String]);
#pragma clang diagnostic pop
    if (ref)
    {
        id reachability = [[self alloc] initWithReachabilityRef:ref];

        return reachability;
    }

    return nil;
}

+(ODWReachability *)reachabilityWithAddress:(void *)hostAddress
{
    if (hostAddress == NULL) {
        NSLog(@"Invalid address");
        return nil;
    }

    if (@available(macOS 10.14, iOS 12.0, *))
    {
        // Use URLSession for macOS 10.14 or higher
        NSString *addressString = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)hostAddress)->sin_addr)];
        NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"https://%@", addressString]];
        NSURLSession *session = [NSURLSession sharedSession];
        __block ODWReachability *reachabilityInstance = [[self alloc] init];
        reachabilityInstance.url = url;
        NSURLSessionDataTask *dataTask = [session dataTaskWithURL:url completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
            reachabilityInstance = [self handleReachabilityResponse:response error:error url:reachabilityInstance.url];
        }];
        [dataTask resume];
        return reachabilityInstance; // Return the instance after resuming the data task
    }
    
    // Use SCNetworkReachability for macOS 10.14 or lower
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    SCNetworkReachabilityRef ref = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (const struct sockaddr*)hostAddress);
#pragma clang diagnostic pop
    if (ref)
    {
        id reachability = [[self alloc] initWithReachabilityRef:ref];

        return reachability;
    }

    return nil;
}

+(ODWReachability *)handleReachabilityResponse:(NSURLResponse *)response error:(NSError *)error url:(NSURL *)url
{
    __block ODWReachability *reachabilityInstance = nil;

    if (error == nil) {
        // Handle successful reachability
        NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
        if (httpResponse.statusCode == 200)
        {
            NSLog(@"Reachability success: %@", url);
            reachabilityInstance = [[self alloc] init];
            reachabilityInstance.url = url;
        }
        else
        {
            NSLog(@"Reachability failed with status code: %ld", (long)httpResponse.statusCode);
        }
        return reachabilityInstance;
    }
    
    // Handle reachability failure
    NSLog(@"Reachability error: %@", error.localizedDescription);
    
    return nil;
}


+(ODWReachability *)reachabilityForInternetConnection
{
    struct sockaddr_in zeroAddress;
    bzero(&zeroAddress, sizeof(zeroAddress));
    zeroAddress.sin_len = sizeof(zeroAddress);
    zeroAddress.sin_family = AF_INET;

    return [self reachabilityWithAddress:&zeroAddress];
}

+(ODWReachability*)reachabilityForLocalWiFi
{
    struct sockaddr_in localWifiAddress;
    bzero(&localWifiAddress, sizeof(localWifiAddress));
    localWifiAddress.sin_len            = sizeof(localWifiAddress);
    localWifiAddress.sin_family         = AF_INET;
    // IN_LINKLOCALNETNUM is defined in <netinet/in.h> as 169.254.0.0
    localWifiAddress.sin_addr.s_addr    = htonl(IN_LINKLOCALNETNUM);

    return [self reachabilityWithAddress:&localWifiAddress];
}


// Initialization methods

-(ODWReachability *)initWithReachabilityRef:(SCNetworkReachabilityRef)ref
{
    self = [super init];
    if (self != nil)
    {
        self.reachableOnWWAN = YES;
        self.reachabilityRef = ref;

        // We need to create a serial queue.
        // We allocate this once for the lifetime of the notifier.

        self.reachabilitySerialQueue = dispatch_queue_create("com.tonymillion.reachability", NULL);
    }

    return self;
}

+(void)setTimeoutDurationInSeconds:(int)timeoutDuration
{
    if (timeoutDuration > 0)
    {
        kTimeoutDurationInSeconds = timeoutDuration;
    }
    else
    {
        NSLog(@"Timeout duration must be positive.");
    }
}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-missing-super-calls" // Not fixing third_party components.
#endif

-(void)dealloc
{
    [self stopNotifier];

    if(self.reachabilityRef)
    {
        CFRelease(self.reachabilityRef);
        self.reachabilityRef = nil;
    }

    self.reachableBlock          = nil;
    self.unreachableBlock        = nil;
    self.reachabilitySerialQueue = nil;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#pragma mark - Notifier Methods

// Notifier
// NOTE: This uses GCD to trigger the blocks - they *WILL NOT* be called on THE MAIN THREAD
// - In other words DO NOT DO ANY UI UPDATES IN THE BLOCKS.
//   INSTEAD USE dispatch_async(dispatch_get_main_queue(), ^{UISTUFF}) (or dispatch_sync if you want)

-(BOOL)startNotifier
{
    if (@available(macOS 10.14, iOS 12.0, *))
    {
        // Use URLSession for macOS 10.14 or higher
        NSURLSession *session = [NSURLSession sharedSession];
        NSURLSessionDataTask *task = [session dataTaskWithURL:[self url] completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
            if (error) {
                NSLog(@"URLSession failed: %@", error.localizedDescription);
                self.reachabilityObject = nil;
            } else {
                self.reachabilityObject = self;
                [[NSNotificationCenter defaultCenter] postNotificationName:kNetworkReachabilityChangedNotification object:self];
            }
        }];
        if (task) {
            [task resume];
            return YES;
        } else {
            NSLog(@"Failed to create URLSessionDataTask");
            return NO;
        }
    }
    
    // Use SCNetworkReachability for macOS 10.14 or lower
    // allow start notifier to be called multiple times
    if (self.reachabilityObject && (self.reachabilityObject == self))
    {
        return YES;
    }

    SCNetworkReachabilityContext context = { 0, NULL, NULL, NULL, NULL };
    context.info = (__bridge void *)self;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (SCNetworkReachabilitySetCallback(self.reachabilityRef, TMReachabilityCallback, &context))
    {
        if (SCNetworkReachabilitySetDispatchQueue(self.reachabilityRef, self.reachabilitySerialQueue))
#pragma clang diagnostic pop
        {
            self.reachabilityObject = self;
            return YES;
        } else {
#ifdef DEBUG
            NSLog(@"SCNetworkReachabilitySetDispatchQueue() failed: %s", SCErrorString(SCError()));
#endif
                
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                // UH OH - FAILURE - stop any callbacks!
                SCNetworkReachabilitySetCallback(self.reachabilityRef, NULL, NULL);
#pragma clang diagnostic pop
        }
    }
    else
    {
#ifdef DEBUG
        NSLog(@"SCNetworkReachabilitySetCallback() failed: %s", SCErrorString(SCError()));
#endif
    }

    // if we get here we fail at the internet
    self.reachabilityObject = nil;
    return NO;
}

-(void)stopNotifier
{
    if (@available(macOS 10.14, iOS 12.0, *))
    {
        // Use URLSession for macOS 10.14 or higher, no specific action is needed for URLSession
        self.reachabilityObject = nil;
    }

    // Use SCNetworkReachability for macOS 10.14 or lower
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // First stop, any callbacks!
    SCNetworkReachabilitySetCallback(self.reachabilityRef, NULL, NULL);

    // Unregister target from the GCD serial dispatch queue.
    SCNetworkReachabilitySetDispatchQueue(self.reachabilityRef, NULL);
#pragma clang diagnostic pop
    self.reachabilityObject = nil;
}


#pragma mark - reachability tests

// This is for the case where you flick the airplane mode;
// you end up getting something like this:
//Reachability: WR ct-----
//Reachability: -- -------
//Reachability: WR ct-----
//Reachability: -- -------
// We treat this as 4 UNREACHABLE triggers - really apple should do better than this

#define testcase (kSCNetworkReachabilityFlagsConnectionRequired | kSCNetworkReachabilityFlagsTransientConnection)

-(BOOL)isReachableWithFlags:(SCNetworkReachabilityFlags)flags
{
    BOOL connectionUP = YES;

    if(!(flags & kSCNetworkReachabilityFlagsReachable))
        connectionUP = NO;

    if( (flags & testcase) == testcase )
        connectionUP = NO;

#if TARGET_OS_IPHONE
    if(flags & kSCNetworkReachabilityFlagsIsWWAN)
    {
        // We're on 3G.
        if(!self.reachableOnWWAN)
        {
            // We don't want to connect when on 3G.
            connectionUP = NO;
        }
    }
#endif

    return connectionUP;
}

-(BOOL)isReachable
{
    if (@available(macOS 10.14, iOS 12.0, *))
    {
        return [self checkNetworkReachability:true];
    }

    // for macOS 10.14 or lower
    SCNetworkReachabilityFlags flags;
        
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if(!SCNetworkReachabilityGetFlags(self.reachabilityRef, &flags))
        return NO;
#pragma clang diagnostic pop

    return [self isReachableWithFlags:flags];
}


-(BOOL)isReachableViaWWAN
{
#if TARGET_OS_IPHONE

    SCNetworkReachabilityFlags flags = 0;

    if(SCNetworkReachabilityGetFlags(self.reachabilityRef, &flags))
    {
        // Check we're REACHABLE
        if(flags & kSCNetworkReachabilityFlagsReachable)
        {
            // Now, check we're on WWAN
            if(flags & kSCNetworkReachabilityFlagsIsWWAN)
            {
                return YES;
            }
        }
    }
#endif

    return NO;
}

-(BOOL)isReachableViaWiFi
{
    if (@available(macOS 10.14, iOS 12.0, *))
    {
        return [self checkNetworkReachability:true];
    }
    
    // for macOS 10.14 or lower
    SCNetworkReachabilityFlags flags = 0;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if(SCNetworkReachabilityGetFlags(self.reachabilityRef, &flags))
#pragma clang diagnostic pop
    {
        // Check we're reachable
        if((flags & kSCNetworkReachabilityFlagsReachable))
        {
#if TARGET_OS_IPHONE
            // Check we're NOT on WWAN
            if((flags & kSCNetworkReachabilityFlagsIsWWAN))
            {
                return NO;
            }
#endif
            return YES;
        }
    }

    return NO;
}


// WWAN may be available, but not active until a connection has been established.
// WiFi may require a connection for VPN on Demand.
-(BOOL)isConnectionRequired
{
    return [self connectionRequired];
}

-(BOOL)connectionRequired
{
    if (@available(macOS 10.14, iOS 12.0, *))
    {
        return [self checkNetworkReachability:false];
    }
    
    // for macOS 10.14 or lower
    SCNetworkReachabilityFlags flags;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if(SCNetworkReachabilityGetFlags(self.reachabilityRef, &flags))
#pragma clang diagnostic pop
    {
        return (flags & kSCNetworkReachabilityFlagsConnectionRequired);
    }

    return NO;
}


// Dynamic, on demand connection?
-(BOOL)isConnectionOnDemand
{
    if (@available(macOS 10.14, iOS 12.0, *))
    {
        return [self checkNetworkReachability:true];
    }

    // for macOS 10.14 or lower
    SCNetworkReachabilityFlags flags;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (SCNetworkReachabilityGetFlags(self.reachabilityRef, &flags))
#pragma clang diagnostic pop
    {
        return ((flags & kSCNetworkReachabilityFlagsConnectionRequired) &&
                (flags & (kSCNetworkReachabilityFlagsConnectionOnTraffic | kSCNetworkReachabilityFlagsConnectionOnDemand)));
    }

    return NO;
}


// Is user intervention required?
-(BOOL)isInterventionRequired
{
    if (@available(macOS 10.14, iOS 12.0, *))
    {
        return [self checkNetworkReachability:false];
    }
    
    // for macOS 10.14 or lower
    SCNetworkReachabilityFlags flags;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (SCNetworkReachabilityGetFlags(self.reachabilityRef, &flags))
#pragma clang diagnostic pop
    {
        return ((flags & kSCNetworkReachabilityFlagsConnectionRequired) &&
                (flags & kSCNetworkReachabilityFlagsInterventionRequired));
    }

    return NO;
}



#pragma mark - reachability status stuff

-(ODWNetworkStatus)currentReachabilityStatus
{
    if([self isReachable])
    {
        if([self isReachableViaWiFi])
            return ReachableViaWiFi;

#if TARGET_OS_IPHONE
        return ReachableViaWWAN;
#endif
    }

    return NotReachable;
}

-(SCNetworkReachabilityFlags)reachabilityFlags
{
    if (@available(macOS 10.14, iOS 12.0, *))
    {
        __block SCNetworkReachabilityFlags flags = 0;
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
                
        NSURLSession *session = [NSURLSession sharedSession];
        NSURLSessionDataTask *task = [session dataTaskWithURL:[self url] completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
        if (error == nil && data != nil) {
            flags = kSCNetworkReachabilityFlagsReachable;
        }
            dispatch_semaphore_signal(semaphore);
        }];
                
        [task resume];
        dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, kTimeoutDurationInSeconds * NSEC_PER_SEC));
                
        return flags;
    }

    // for macOS 10.14 or lower
    SCNetworkReachabilityFlags flags = 0;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (SCNetworkReachabilityGetFlags(self.reachabilityRef, &flags))
#pragma clang diagnostic pop
    {
        return flags;
    }

    return 0;
}

-(NSString*)currentReachabilityString
{
    ODWNetworkStatus temp = [self currentReachabilityStatus];

    if(temp == ReachableViaWWAN)
    {
        // Updated for the fact that we have CDMA phones now!
        return NSLocalizedString(@"Cellular", @"");
    }
    if (temp == ReachableViaWiFi)
    {
        return NSLocalizedString(@"WiFi", @"");
    }

    return NSLocalizedString(@"No Connection", @"");
}

-(NSString*)currentReachabilityFlags
{
    return reachabilityFlags([self reachabilityFlags]);
}

- (SCNetworkReachabilityFlags)checkNetworkReachability:(BOOL)checkData
{
    __block BOOL connection = NO;
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    
    NSURLSession *session = [NSURLSession sharedSession];
    NSURLSessionDataTask *task = [session dataTaskWithURL:[self url] completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
        if (error == nil && !checkData)
        {
            connection = YES;
        }
        else if (error == nil && checkData && data != nil)
        {
            connection = YES;
        }
        dispatch_semaphore_signal(semaphore);
    }];
    
    [task resume];
    dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, kTimeoutDurationInSeconds * NSEC_PER_SEC));
    
    return connection;
}

#pragma mark - Callback function calls this method

-(void)reachabilityChanged:(SCNetworkReachabilityFlags)flags
{
    if([self isReachableWithFlags:flags])
    {
        if(self.reachableBlock)
        {
            self.reachableBlock(self);
        }
    }
    else
    {
        if(self.unreachableBlock)
        {
            self.unreachableBlock(self);
        }
    }

    // this makes sure the change notification happens on the MAIN THREAD
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotificationName:kNetworkReachabilityChangedNotification
                                                            object:self];
    });
}

#pragma mark - Debug Description

- (NSString *) description
{
    NSString *description = [NSString stringWithFormat:@"<%@: %p (%@)>",
                             NSStringFromClass([self class]), self, [self currentReachabilityFlags]];
    return description;
}

@end
