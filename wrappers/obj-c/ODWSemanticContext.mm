//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#import <Foundation/Foundation.h>
#import "ODWSemanticContext.h"
#import "ODWSemanticContext_private.h"
#include "ISemanticContext.hpp"

using namespace MAT;

@implementation ODWSemanticContext
{
    ISemanticContext* _wrappedSemanticContext;
}

-(instancetype)initWithISemanticContext:(ISemanticContext*)context
{
    self = [super init];
    if(self){
        _wrappedSemanticContext = context;
    }
    return self;
}

-(void)setAppId:(nonnull NSString *)appId
{
    std::string strAppId = std::string([appId UTF8String]);
    _wrappedSemanticContext->SetAppId(strAppId);
}

-(void) setAppVersion:(nonnull NSString*)appVersion
{
    std::string strAppVersion = std::string([appVersion UTF8String]);
    _wrappedSemanticContext->SetAppVersion(strAppVersion);
}

-(void) setAppLanguage:(nonnull NSString*)appLanguage
{
    std::string strAppLanguage = std::string([appLanguage UTF8String]);
    _wrappedSemanticContext->SetAppLanguage(strAppLanguage);
}

-(void)setUserId:(nonnull NSString *)userId
{
    [self setUserId:userId piiKind:ODWPiiKindIdentity];
}

-(void) setUserId:(nonnull NSString*)userId
          piiKind:(enum ODWPiiKind)pii
{
    if (pii == ODWPiiKindNone || pii == ODWPiiKindIdentity) {
        PiiKind piiValue = PiiKind(pii);
        std::string strUserId = std::string([userId UTF8String]);
        _wrappedSemanticContext->SetUserId(strUserId, piiValue);
    } else {
        [NSException raise:@"1DSSDKException" format:[NSString stringWithFormat:@"Invalid Pii type is passed. Expected ODWPiiKindNone or ODWPiiKindIdentity only."]];
    }
}

-(void) setDeviceId:(nonnull NSString*)deviceId
{
    std::string strDeviceId = std::string([deviceId UTF8String]);
    _wrappedSemanticContext->SetDeviceId(strDeviceId);
}

-(void) setUserTimeZone:(nonnull NSString*)userTimeZone
{
    std::string strUserTimeZone = std::string([userTimeZone UTF8String]);
    _wrappedSemanticContext->SetUserTimeZone(strUserTimeZone);
}

-(void)setUserAdvertisingId:(nonnull NSString *)userAdvertisingId
{
    std::string strUserAdvertisingId = std::string([userAdvertisingId UTF8String]);
    _wrappedSemanticContext->SetUserAdvertisingId(strUserAdvertisingId);
}

-(void)setAppExperimentIds:(nonnull NSString*)experimentIds
{
    std::string strAppExperimentIds = std::string([experimentIds UTF8String]);
    _wrappedSemanticContext->SetAppExperimentIds(strAppExperimentIds);
}

-(void)setAppExperimentIds:(nonnull NSString*)experimentIds
                  forEvent:(nonnull NSString*)eventName
{
    std::string strAppExperimentIds = std::string([experimentIds UTF8String]);
    std::string stEventName = std::string([eventName UTF8String]);
    _wrappedSemanticContext->SetEventExperimentIds(stEventName, strAppExperimentIds);
}

-(void)setAppExperimentETag:(nonnull NSString *)eTag
{
    std::string strETag = std::string([eTag UTF8String]);
    _wrappedSemanticContext->SetAppExperimentETag(strETag);
}

-(void)setAppExperimentImpressionId:(nonnull NSString*)impressionId
{
    std::string strImpressionId = std::string([impressionId UTF8String]);
    _wrappedSemanticContext->SetAppExperimentImpressionId(strImpressionId);
}

@end
