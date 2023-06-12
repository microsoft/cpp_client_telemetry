//
//  ODWSignalsOptions.m
//  wrappers
//
//  Created by Leonardo Alves on 24/04/23.
//

#import <Foundation/Foundation.h>
#import "ODWSignalsOptions.h"

/*!
 @brief Signals Initialization Options
 */
@implementation ODWSignalsOptions : NSObject
-(id)init {
    _baseUrl = nil;
    _timeoutMs = 90000;
    _retryTimes = 3;
    _retryTimesToWait = 3000;
    _retryStatusCodes = @[@429, @500, @503, @507, @0];
    return self;
}
@end
