#include "objc_begin.h"

/*!
 The <b>ODWLogConfiguration</b> static class represents general logging properties.
*/
@interface ODWLogConfiguration : NSObject

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
