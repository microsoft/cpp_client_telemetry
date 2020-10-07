//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include <stdexcept>
#include "LogManager.hpp"
#include "DefaultDataViewer.hpp"
#import <Foundation/Foundation.h>
#import "ODWDiagnosticDataViewer.h"
#import "ODWLogConfiguration.h"
#import "ODWLogger_private.h"

using namespace MAT;

@implementation ODWDiagnosticDataViewer

std::shared_ptr<DefaultDataViewer> _viewer;

+(void)initializeViewerWithMachineIdentifier:(NSString *)machineIdentifier
{
    const std::string identifier = { [machineIdentifier UTF8String] };
    try
    {
        _viewer = std::make_shared<DefaultDataViewer> (nullptr, identifier);
        LogManager::GetDataViewerCollection().RegisterViewer(_viewer);
    }
    catch (const std::exception &e)
    {
        if ([ODWLogConfiguration surfaceCppExceptions])
        {
            [ODWLogger raiseException: e.what()];
        }
        [ODWLogger traceException: e.what()];
    }
}

+(void)enableRemoteViewer:(NSString *)endpoint completionWithResult:(void(^)(bool result))completion
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        bool result = [ODWDiagnosticDataViewer enableRemoteViewer: endpoint];
        if (completion)
        {
            completion(result);
        }
    } );
}

+(bool)enableRemoteViewer:(NSString *)endpoint
{
    bool result = false;
    try
    {
        result = _viewer->EnableRemoteViewer(std::string([endpoint UTF8String]));
        if ([ODWLogConfiguration enableConsoleLogging])
        {
            NSLog(@"RemoteDataViewer enabled on endpoint: %@ and result: %@", endpoint, result ? @"success" : @"failure");
        }
    }
    catch (const std::exception &e)
    {
        if ([ODWLogConfiguration surfaceCppExceptions])
        {
            [ODWLogger raiseException: e.what()];
        }
        [ODWLogger traceException: e.what()];
    }

    return result;
}

+(void)disableViewer:(void(^)(bool result))completion
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        bool result = [ODWDiagnosticDataViewer disableViewer];
        if (completion)
        {
            completion(result);
        }
    } );
}

+(bool)disableViewer
{
    bool result = _viewer->DisableViewer();
    if ([ODWLogConfiguration enableConsoleLogging])
    {
        NSLog(@"RemoteDataViewer disabled with result: %@", result ? @"success" : @"failure");
    }

    return result;
}

+(bool)viewerEnabled
{
    bool result = false;
    try
    {
        result = LogManager::GetDataViewerCollection().IsViewerEnabled(_viewer->GetName());
    }
    catch (const std::exception &e)
    {
        if ([ODWLogConfiguration surfaceCppExceptions])
        {
            [ODWLogger raiseException: e.what()];
        }
        [ODWLogger traceException: e.what()];
    }

    return result;
}

+(nullable NSString *)currentEndpoint
{
    std::string endpoint = _viewer->GetCurrentEndpoint();
    if (endpoint.empty())
    {
        // Empty endpoint means there is not a current endpoint
        return nil;
    }

    return [NSString stringWithCString:endpoint.c_str() encoding:NSUTF8StringEncoding];
}

+(void)registerOnDisableNotification:(void(^)(void))callback
{
    std::function<void()> disableNotification = std::bind(callback);
    _viewer->RegisterOnDisableNotification(disableNotification);
    if ([ODWLogConfiguration enableConsoleLogging])
    {
        NSLog(@"Registered OnDisableNotification");
    }
}

@end
