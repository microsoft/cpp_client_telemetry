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

#import <Network/Network.h>

#import <sys/socket.h>
#import <netinet/in.h>
#import <arpa/inet.h>


NSString *const kNetworkReachabilityChangedNotification = @"NetworkReachabilityChangedNotification";
static char ODWReachabilityQueueKey;

@class ODWReachability;

@interface ODWReachabilityMonitorContext : NSObject

#if __has_feature(objc_arc)
@property (nonatomic, weak) ODWReachability *owner;
#else
@property (nonatomic, unsafe_unretained) ODWReachability *owner;
#endif

@end

@implementation ODWReachabilityMonitorContext
@end


@interface ODWReachability ()

@property (nonatomic, assign) SCNetworkReachabilityRef  reachabilityRef;
@property (nonatomic, strong) dispatch_queue_t          reachabilitySerialQueue;
@property (nonatomic, strong) id                        reachabilityObject;
@property (nonatomic, strong) nw_path_monitor_t         pathMonitor;
@property (nonatomic, strong) ODWReachabilityMonitorContext *pathMonitorContext;
@property (nonatomic, strong) dispatch_semaphore_t      initialPathSemaphore;
@property (nonatomic, assign) nw_path_status_t          currentPathStatus;
@property (nonatomic, assign) BOOL                      currentPathUsesWiFi;
@property (nonatomic, assign) BOOL                      currentPathUsesWWAN;
@property (nonatomic, assign) BOOL                      hasObservedPath;
@property (nonatomic, assign) BOOL                      monitorLocalWiFiOnly;

-(void)reachabilityChanged:(SCNetworkReachabilityFlags)flags;
-(BOOL)isReachableWithFlags:(SCNetworkReachabilityFlags)flags;
-(BOOL)getReachabilityFlags:(SCNetworkReachabilityFlags *)flags;
-(BOOL)startLegacyNotifier;
-(void)stopLegacyNotifier;
-(BOOL)ensureModernPathMonitor API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0));
-(BOOL)awaitModernPathSnapshot API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0));
-(void)handleModernPathUpdate:(nw_path_t)path API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0));
-(void)notifyModernPathChange API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0));

@end


static NSString *reachabilityFlags(SCNetworkReachabilityFlags flags)
{
    return [NSString stringWithFormat:@"%c%c %c%c%c%c%c%c%c",
#if ODW_REACHABILITY_HAS_WWAN
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

static BOOL ODWModernPathIsReachable(nw_path_status_t status) API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
{
    return status == nw_path_status_satisfied || status == nw_path_status_satisfiable;
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
    return [ODWReachability reachabilityWithHostname:hostname];
}

+(instancetype)reachabilityWithHostname:(NSString*)hostname
{
    if (hostname == nil || [hostname length] == 0)
    {
        NSLog(@"Invalid hostname '%@': hostname is empty", hostname);
        return nil;
    }

    NSString *reachabilityHost = hostname;
    NSURL *url = nil;
    NSURLComponents *components = [NSURLComponents componentsWithString:hostname];
    if ([components.scheme length] > 0)
    {
        if ([components.host length] == 0)
        {
            NSLog(@"Invalid hostname '%@': URL has no host", hostname);
            return nil;
        }

        reachabilityHost = components.host;
        url = components.URL;
    }

    if (url == nil)
    {
        url = [NSURL URLWithString:[NSString stringWithFormat:@"https://%@", reachabilityHost]];
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    SCNetworkReachabilityRef ref = SCNetworkReachabilityCreateWithName(NULL, [reachabilityHost UTF8String]);
#pragma clang diagnostic pop
    if (ref)
    {
        ODWReachability *reachability = [[self alloc] initWithReachabilityRef:ref];
        reachability.url = url;

        return reachability;
    }

    const char *errorString = SCErrorString(SCError());
    NSLog(@"Invalid hostname '%@': SCNetworkReachabilityCreateWithName failed for '%@' (%s)",
          hostname,
          reachabilityHost,
          errorString != NULL ? errorString : "unknown error");
    return nil;
}

+(ODWReachability *)reachabilityWithAddress:(void *)hostAddress
{
    if (hostAddress == NULL)
    {
        NSLog(@"Invalid address: address pointer is null");
        return nil;
    }

    struct sockaddr_storage addressStorage;
    bzero(&addressStorage, sizeof(addressStorage));
    struct sockaddr *address = (struct sockaddr *)hostAddress;
    NSURL *url = nil;
    if (address->sa_family == AF_INET)
    {
        char addressString[INET_ADDRSTRLEN] = { 0 };
        struct sockaddr_in *ipv4Address = (struct sockaddr_in *)&addressStorage;
        *ipv4Address = *(struct sockaddr_in *)hostAddress;
        if (ipv4Address->sin_len == 0)
        {
            ipv4Address->sin_len = sizeof(*ipv4Address);
        }
        address = (struct sockaddr *)ipv4Address;
        if (inet_ntop(AF_INET, &ipv4Address->sin_addr, addressString, sizeof(addressString)) != NULL)
        {
            url = [NSURL URLWithString:[NSString stringWithFormat:@"https://%s", addressString]];
        }
    }
    else if (address->sa_family == AF_INET6)
    {
        char addressString[INET6_ADDRSTRLEN] = { 0 };
        struct sockaddr_in6 *ipv6Address = (struct sockaddr_in6 *)&addressStorage;
        *ipv6Address = *(struct sockaddr_in6 *)hostAddress;
        if (ipv6Address->sin6_len == 0)
        {
            ipv6Address->sin6_len = sizeof(*ipv6Address);
        }
        address = (struct sockaddr *)ipv6Address;
        if (inet_ntop(AF_INET6, &ipv6Address->sin6_addr, addressString, sizeof(addressString)) != NULL)
        {
            url = [NSURL URLWithString:[NSString stringWithFormat:@"https://[%s]", addressString]];
        }
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    SCNetworkReachabilityRef ref = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (const struct sockaddr*)address);
#pragma clang diagnostic pop
    if (ref)
    {
        ODWReachability *reachability = [[self alloc] initWithReachabilityRef:ref];
        reachability.url = url;

        return reachability;
    }

    const char *errorString = SCErrorString(SCError());
    NSLog(@"Invalid address: SCNetworkReachabilityCreateWithAddress failed (%s)",
          errorString != NULL ? errorString : "unknown error");
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
#if ODW_LEGACY_REACHABILITY_REQUIRED
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, watchOS 5.0, *))
    {
        return [[self alloc] init];
    }
#else
    return [[self alloc] init];
#endif

    struct sockaddr_in zeroAddress;
    bzero(&zeroAddress, sizeof(zeroAddress));
    zeroAddress.sin_len = sizeof(zeroAddress);
    zeroAddress.sin_family = AF_INET;

    return [self reachabilityWithAddress:&zeroAddress];
}

+(ODWReachability*)reachabilityForLocalWiFi
{
#if ODW_LEGACY_REACHABILITY_REQUIRED
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, watchOS 5.0, *))
    {
        ODWReachability *reachability = [[self alloc] init];
        reachability.monitorLocalWiFiOnly = YES;
        return reachability;
    }
#else
    ODWReachability *reachability = [[self alloc] init];
    reachability.monitorLocalWiFiOnly = YES;
    return reachability;
#endif

    struct sockaddr_in localWifiAddress;
    bzero(&localWifiAddress, sizeof(localWifiAddress));
    localWifiAddress.sin_len            = sizeof(localWifiAddress);
    localWifiAddress.sin_family         = AF_INET;
    // IN_LINKLOCALNETNUM is defined in <netinet/in.h> as 169.254.0.0
    localWifiAddress.sin_addr.s_addr    = htonl(IN_LINKLOCALNETNUM);

    return [self reachabilityWithAddress:&localWifiAddress];
}


// Initialization methods

-(instancetype)init
{
    self = [super init];
    if (self != nil)
    {
        self.reachableOnWWAN = YES;
        self.reachabilitySerialQueue = dispatch_queue_create("com.tonymillion.reachability", NULL);
        dispatch_queue_set_specific(self.reachabilitySerialQueue,
                                    &ODWReachabilityQueueKey,
                                    &ODWReachabilityQueueKey,
                                    NULL);
    }

    return self;
}

-(ODWReachability *)initWithReachabilityRef:(SCNetworkReachabilityRef)ref
{
    self = [self init];
    if (self != nil)
    {
        self.reachabilityRef = ref;
    }

    return self;
}

-(BOOL)ensureModernPathMonitor
{
    if (self.pathMonitor != nil)
    {
        return YES;
    }

    self.hasObservedPath = NO;
    self.currentPathStatus = nw_path_status_invalid;
    self.currentPathUsesWiFi = NO;
    self.currentPathUsesWWAN = NO;
    self.initialPathSemaphore = dispatch_semaphore_create(0);
    self.pathMonitor = self.monitorLocalWiFiOnly
        ? nw_path_monitor_create_with_type(nw_interface_type_wifi)
        : nw_path_monitor_create();

    if (self.pathMonitor == nil)
    {
        return NO;
    }

    ODWReachabilityMonitorContext *context = [[ODWReachabilityMonitorContext alloc] init];
    context.owner = self;
    self.pathMonitorContext = context;
#if !__has_feature(objc_arc)
    [context release];
#endif

    nw_path_monitor_set_queue(self.pathMonitor, self.reachabilitySerialQueue);
    nw_path_monitor_set_update_handler(self.pathMonitor, ^(nw_path_t path) {
        ODWReachability *owner = context.owner;
        if (owner == nil)
        {
            return;
        }

        [owner handleModernPathUpdate:path];
    });
    nw_path_monitor_start(self.pathMonitor);

    return YES;
}

-(BOOL)awaitModernPathSnapshot
{
    if (![self ensureModernPathMonitor])
    {
        return NO;
    }

    if (self.hasObservedPath)
    {
        return YES;
    }

    // Capture the semaphore into a local so a concurrent -stopNotifier on
    // another thread cannot release the property between the nil-check and
    // the wait below.
    dispatch_semaphore_t semaphore = self.initialPathSemaphore;
    if (semaphore == nil)
    {
        return NO;
    }

    // Avoid blocking reachability queries on the main thread before the first
    // NWPathMonitor update arrives. Callers get a conservative "unknown yet"
    // result until the async update handler records the first snapshot.
    if ([NSThread isMainThread])
    {
        return NO;
    }
    // The update handler runs on this serial queue, so waiting here would deadlock it.
    if (dispatch_get_specific(&ODWReachabilityQueueKey) == &ODWReachabilityQueueKey)
    {
        return NO;
    }

    long waitResult = dispatch_semaphore_wait(
        semaphore,
        dispatch_time(DISPATCH_TIME_NOW, kTimeoutDurationInSeconds * NSEC_PER_SEC));
    return waitResult == 0 && self.hasObservedPath;
}

-(void)handleModernPathUpdate:(nw_path_t)path
{
    self.currentPathStatus = nw_path_get_status(path);
    self.currentPathUsesWiFi = nw_path_uses_interface_type(path, nw_interface_type_wifi);
#if ODW_REACHABILITY_HAS_WWAN
    self.currentPathUsesWWAN = nw_path_uses_interface_type(path, nw_interface_type_cellular);
#else
    self.currentPathUsesWWAN = NO;
#endif

    BOOL firstPath = !self.hasObservedPath;
    self.hasObservedPath = YES;
    if (firstPath)
    {
        // Capture the semaphore into a local so a concurrent -stopNotifier
        // on another thread cannot release the property between the
        // nil-check and the signal below.
        dispatch_semaphore_t semaphore = self.initialPathSemaphore;
        if (semaphore != nil)
        {
            dispatch_semaphore_signal(semaphore);
        }
    }

    if (self.reachabilityObject == self)
    {
        [self notifyModernPathChange];
    }
}

-(void)notifyModernPathChange
{
    if (ODWModernPathIsReachable(self.currentPathStatus))
    {
        if (self.reachableBlock)
        {
            self.reachableBlock(self);
        }
    }
    else if (self.unreachableBlock)
    {
        self.unreachableBlock(self);
    }

    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotificationName:kNetworkReachabilityChangedNotification
                                                            object:self];
    });
}

+(void)setTimeoutDurationInSeconds:(int)timeoutDuration
{
    if (timeoutDuration >= kTimeoutDurationInSeconds)
    {
        kTimeoutDurationInSeconds = timeoutDuration;
    }
    else
    {
        NSLog(@"Timeout duration must be at least 10.");
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
    if (self.reachabilityRef != nil)
    {
        return [self startLegacyNotifier];
    }

#if ODW_LEGACY_REACHABILITY_REQUIRED
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, watchOS 5.0, *))
    {
#endif
        // Use NWPathMonitor for macOS 10.14 or higher.
        if ([self ensureModernPathMonitor])
        {
            self.reachabilityObject = self;
            if ([self awaitModernPathSnapshot])
            {
                [self notifyModernPathChange];
            }
            return YES;
        }
        return NO;
#if ODW_LEGACY_REACHABILITY_REQUIRED
    }

    return NO;
#endif
}

-(BOOL)startLegacyNotifier
{
    // Allow start notifier to be called multiple times.
    if (self.reachabilityObject && (self.reachabilityObject == self))
    {
        return YES;
    }

    SCNetworkReachabilityContext context = { 0, NULL, NULL, NULL, NULL };
    context.info = (__bridge void *)self;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    BOOL callbackSet = SCNetworkReachabilitySetCallback(self.reachabilityRef, TMReachabilityCallback, &context);
    BOOL queueSet = callbackSet && SCNetworkReachabilitySetDispatchQueue(self.reachabilityRef, self.reachabilitySerialQueue);
#pragma clang diagnostic pop
    if (callbackSet)
    {
        if (queueSet)
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
    if (self.reachabilityRef != nil)
    {
        [self stopLegacyNotifier];
        return;
    }

#if ODW_LEGACY_REACHABILITY_REQUIRED
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, watchOS 5.0, *))
    {
#endif
        // Use NWPathMonitor for macOS 10.14 or higher.
        self.reachabilityObject = nil;
        if (self.pathMonitor != nil)
        {
            self.pathMonitorContext.owner = nil;
            nw_path_monitor_cancel(self.pathMonitor);
            self.pathMonitor = nil;
        }
        self.pathMonitorContext = nil;
        self.initialPathSemaphore = nil;
        self.hasObservedPath = NO;
        self.currentPathStatus = nw_path_status_invalid;
        self.currentPathUsesWiFi = NO;
        self.currentPathUsesWWAN = NO;
#if ODW_LEGACY_REACHABILITY_REQUIRED
        return;
    }

    return;
#endif
}

-(void)stopLegacyNotifier
{
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

#if ODW_REACHABILITY_HAS_WWAN
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

-(BOOL)getReachabilityFlags:(SCNetworkReachabilityFlags *)flags
{
    if (self.reachabilityRef == nil)
    {
        return NO;
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    BOOL result = SCNetworkReachabilityGetFlags(self.reachabilityRef, flags);
#pragma clang diagnostic pop
    return result;
}

-(BOOL)isReachable
{
    if (self.reachabilityRef != nil)
    {
        SCNetworkReachabilityFlags flags;
        return [self getReachabilityFlags:&flags] && [self isReachableWithFlags:flags];
    }

#if ODW_LEGACY_REACHABILITY_REQUIRED
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, watchOS 5.0, *))
    {
#endif
        return [self awaitModernPathSnapshot] && ODWModernPathIsReachable(self.currentPathStatus);
#if ODW_LEGACY_REACHABILITY_REQUIRED
    }

    return NO;
#endif
}


-(BOOL)isReachableViaWWAN
{
#if ODW_REACHABILITY_HAS_WWAN
    if (self.reachabilityRef != nil)
    {
        SCNetworkReachabilityFlags flags = 0;
        return [self getReachabilityFlags:&flags] &&
               (flags & kSCNetworkReachabilityFlagsReachable) &&
               (flags & kSCNetworkReachabilityFlagsIsWWAN);
    }

#if ODW_LEGACY_REACHABILITY_REQUIRED
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, watchOS 5.0, *))
    {
#endif
        return [self awaitModernPathSnapshot] &&
               ODWModernPathIsReachable(self.currentPathStatus) &&
               self.currentPathUsesWWAN;
#if ODW_LEGACY_REACHABILITY_REQUIRED
    }

    return NO;
#endif
#else
    return NO;
#endif
}

-(BOOL)isReachableViaWiFi
{
    if (self.reachabilityRef != nil)
    {
        SCNetworkReachabilityFlags flags = 0;
        if ([self getReachabilityFlags:&flags] && (flags & kSCNetworkReachabilityFlagsReachable))
        {
#if ODW_REACHABILITY_HAS_WWAN
            if (flags & kSCNetworkReachabilityFlagsIsWWAN)
            {
                return NO;
            }
#endif
            return YES;
        }

        return NO;
    }

#if ODW_LEGACY_REACHABILITY_REQUIRED
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, watchOS 5.0, *))
    {
#endif
        if (![self awaitModernPathSnapshot] || !ODWModernPathIsReachable(self.currentPathStatus))
        {
            return NO;
        }
#if ODW_REACHABILITY_HAS_WWAN
        if (self.monitorLocalWiFiOnly)
        {
            return self.currentPathUsesWiFi;
        }

        return !self.currentPathUsesWWAN;
#else
        return self.monitorLocalWiFiOnly ? self.currentPathUsesWiFi : YES;
#endif
#if ODW_LEGACY_REACHABILITY_REQUIRED
    }

    return NO;
#endif
}


// WWAN may be available, but not active until a connection has been established.
// WiFi may require a connection for VPN on Demand.
-(BOOL)isConnectionRequired
{
    return [self connectionRequired];
}

-(BOOL)connectionRequired
{
    if (self.reachabilityRef != nil)
    {
        SCNetworkReachabilityFlags flags;
        return [self getReachabilityFlags:&flags] &&
               (flags & kSCNetworkReachabilityFlagsConnectionRequired);
    }

#if ODW_LEGACY_REACHABILITY_REQUIRED
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, watchOS 5.0, *))
    {
#endif
        return [self awaitModernPathSnapshot] &&
               self.currentPathStatus == nw_path_status_satisfiable;
#if ODW_LEGACY_REACHABILITY_REQUIRED
    }

    return NO;
#endif
}


// Dynamic, on demand connection?
-(BOOL)isConnectionOnDemand
{
    if (self.reachabilityRef != nil)
    {
        SCNetworkReachabilityFlags flags;
        if (![self getReachabilityFlags:&flags])
        {
            return NO;
        }

        return ((flags & kSCNetworkReachabilityFlagsConnectionRequired) &&
                (flags & (kSCNetworkReachabilityFlagsConnectionOnTraffic | kSCNetworkReachabilityFlagsConnectionOnDemand)));
    }

    return NO;
}


// Is user intervention required?
-(BOOL)isInterventionRequired
{
    if (self.reachabilityRef != nil)
    {
        SCNetworkReachabilityFlags flags;
        if (![self getReachabilityFlags:&flags])
        {
            return NO;
        }

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

#if ODW_REACHABILITY_HAS_WWAN
        return ReachableViaWWAN;
#endif
    }

    return NotReachable;
}

-(SCNetworkReachabilityFlags)reachabilityFlags
{
    if (self.reachabilityRef != nil)
    {
        SCNetworkReachabilityFlags flags = 0;
        return [self getReachabilityFlags:&flags] ? flags : 0;
    }

#if ODW_LEGACY_REACHABILITY_REQUIRED
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, watchOS 5.0, *))
    {
#endif
        if (![self awaitModernPathSnapshot])
        {
            return 0;
        }

        SCNetworkReachabilityFlags flags = 0;
        if (ODWModernPathIsReachable(self.currentPathStatus))
        {
            flags |= kSCNetworkReachabilityFlagsReachable;
        }
        if (self.currentPathStatus == nw_path_status_satisfiable)
        {
            flags |= kSCNetworkReachabilityFlagsConnectionRequired;
        }
#if ODW_REACHABILITY_HAS_WWAN
        if (self.currentPathUsesWWAN)
        {
            flags |= kSCNetworkReachabilityFlagsIsWWAN;
        }
#endif
        return flags;
#if ODW_LEGACY_REACHABILITY_REQUIRED
    }

    return 0;
#endif
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
