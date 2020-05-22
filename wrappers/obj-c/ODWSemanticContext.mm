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

-(void)setUserId:(nonnull NSString *)userId
{
    std::string strUserId = std::string([userId UTF8String]);
    _wrappedSemanticContext->SetUserId(strUserId);
}

-(void)setUserAdvertisingId:(nonnull NSString *)userAdvertisingId
{
    std::string strUserAdvertisingId = std::string([userAdvertisingId UTF8String]);
    _wrappedSemanticContext->SetUserAdvertisingId(strUserAdvertisingId);
}

@end
