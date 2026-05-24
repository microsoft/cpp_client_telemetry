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

#import <AvailabilityMacros.h>
#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <TargetConditionals.h>


/**
 * Create NS_ENUM macro if it does not exist on the targeted version of iOS or OS X.
 *
 * @see http://nshipster.com/ns_enum-ns_options/
 **/
#ifndef NS_ENUM
#define NS_ENUM(_type, _name) enum _name : _type _name; enum _name : _type
#endif

extern NSString* const kNetworkReachabilityChangedNotification;

// Older Apple deployment targets still need the legacy SCNetworkReachability
// backend at runtime. Newer targets can compile directly to the modern path.
#ifndef TARGET_OS_IOS
#define TARGET_OS_IOS 0
#endif

#if TARGET_OS_IOS
#define ODW_LEGACY_REACHABILITY_REQUIRED (__IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_12_0)
#elif TARGET_OS_OSX
#define ODW_LEGACY_REACHABILITY_REQUIRED (__MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_14)
#else
#define ODW_LEGACY_REACHABILITY_REQUIRED 0
#endif

#define ODW_REACHABILITY_HAS_WWAN TARGET_OS_IOS

typedef NS_ENUM(NSInteger, ODWNetworkStatus) {
    // Apple NetworkStatus Compatible Names.
    NotReachable = 0,
    ReachableViaWiFi = 2,
    ReachableViaWWAN = 1
};

@class ODWReachability;

typedef void (^NetworkReachable)(ODWReachability * reachability);
typedef void (^NetworkUnreachable)(ODWReachability * reachability);


@interface ODWReachability : NSObject

@property (nonatomic, copy) NetworkReachable    reachableBlock;
@property (nonatomic, copy) NetworkUnreachable  unreachableBlock;

@property (nonatomic, assign) BOOL reachableOnWWAN;

@property (nonatomic, strong) NSURL *url;

+(ODWReachability*)reachabilityWithHostname:(NSString*)hostname;
// This is identical to the function above, but is here to maintain
//compatibility with Apples original code. (see .m)
+(ODWReachability*)reachabilityWithHostName:(NSString*)hostname;
+(ODWReachability*)reachabilityForInternetConnection;
+(ODWReachability*)reachabilityWithAddress:(void *)hostAddress;
+(ODWReachability*)reachabilityForLocalWiFi;
+(void)setTimeoutDurationInSeconds:(int)timeoutDuration;

// -------------------------------------------------------------------------
// Behavior note for hostname / address constructors (modern Apple targets):
//
// Prior to the NWPathMonitor migration, instances created via
// +reachabilityWithHostname: or +reachabilityWithAddress: would, on
// iOS 12+ / macOS 10.14+, internally issue an NSURLSession HTTPS GET to
// the host's URL whenever -isReachable / -isReachableViaWiFi was queried,
// yielding a true per-endpoint reachability answer (DNS + TLS round-trip).
//
// After the migration, those queries return OS-level reachability for the
// hostname/address via SCNetworkReachabilityGetFlags on the SC ref the
// constructor created. That still involves DNS via the SC stack but is
// not the same as actually probing the endpoint with HTTPS. Callers that
// require an authoritative "can I reach this exact endpoint over HTTPS?"
// answer should perform that probe themselves rather than relying on
// these per-host instances. The SDK's own production network detection
// (NetworkInformationImpl) uses +reachabilityForInternetConnection on
// legacy targets and nw_path_monitor_create() directly on modern targets;
// neither path was ever per-endpoint, so this change does not affect the
// SDK's own behavior.
// -------------------------------------------------------------------------

-(ODWReachability *)initWithReachabilityRef:(SCNetworkReachabilityRef)ref;

-(BOOL)startNotifier;
-(void)stopNotifier;

-(BOOL)isReachable;
-(BOOL)isReachableViaWWAN;
-(BOOL)isReachableViaWiFi;

// WWAN may be available, but not active until a connection has been established.
// WiFi may require a connection for VPN on Demand.
-(BOOL)isConnectionRequired; // Identical DDG variant.
-(BOOL)connectionRequired; // Apple's routine.
// Dynamic, on demand connection?
-(BOOL)isConnectionOnDemand;
// Is user intervention required?
-(BOOL)isInterventionRequired;

-(ODWNetworkStatus)currentReachabilityStatus;
-(SCNetworkReachabilityFlags)reachabilityFlags;
-(NSString*)currentReachabilityString;
-(NSString*)currentReachabilityFlags;

@end
