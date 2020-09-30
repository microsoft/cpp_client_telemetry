#include "objc_begin.h"

/*!
 The <b>ODWLogConfiguration</b> static class represents general logging properties.
*/
@interface ODWLogConfiguration : NSObject

/*!
@brief Sets the URI of the event collector.
@param eventCollectorUri A string for event collector uri.
*/
+(void)setEventCollectorUri:(nonnull NSString *)eventCollectorUri;

/*!
@brief Sets the RAM queue size limit in bytes.
@param cacheMemorySizeLimitInBytes  An integer for memory size limit in bytes.
*/
+(void)setCacheMemorySizeLimitInBytes:(int)cacheMemorySizeLimitInBytes;

/*!
@brief Sets the size limit of the disk file used to cache events on the client side.
@param cacheFileSizeLimitInBytes  A n integer value for cache file size limit.
*/
+(void)setCacheFileSizeLimitInBytes:(int)cacheFileSizeLimitInBytes;

/*!
@brief Sets max teardown upload time in seconds.
@param maxTeardownUploadTimeInSec An integer that time in seconds.
*/
+(void)setMaxTeardownUploadTimeInSec:(int)maxTeardownUploadTimeInSec;

/*!
@brief Sets if tracing is enabled.
@param enableTrace True if tracing is enabled.
*/
+(void)setEnableTrace:(bool)enableTrace;

/*!
@brief Sets the internal SDK debugging trace level.
@param one of the ACTTraceLevel values.
*/
+(void)setTraceLevel:(int)TraceLevel;

/*!
@brief Returns true if tracing is enabled.
*/
+(bool)enableTrace;

/*!
@brief Sets if inner C++ exceptions should be surfaced to Wrapper consumers.
@param surfaceCppExceptions True if C++ exceptions should be surfaced.
*/
+(void)setSurfaceCppExceptions:(bool)surfaceCppExceptions;

/*!
@brief Returns true if inner C++ exceptions are surfaced to Wrapper consumers.
*/
+(bool)surfaceCppExceptions;

@end

#include "objc_end.h"
