//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"
#import "ODWCommonDataContext.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 @enum ODWDataConcernType
 @brief The <b>ODWDataConcernType</b> enumeration contains a set of values that specify applicable Data Concerns that Privacy Guard can detect.
 */
typedef NS_ENUM(NSUInteger, ODWDataConcernType)
{
    ODWNone = 0,                          //DefaultValue

    ODWContent = 1,                       // Formatted text: HTML, MIME, RTF, Xml, etc.
    ODWDemographicInfoCountryRegion = 2,  // Country/region
    ODWDemographicInfoLanguage = 3,       // The users language ID. Example: En-Us
    ODWDirectory = 4,                     // Any directory or file share
    ODWExternalEmailAddress = 5,          // SMTP not ending in <span>microsoft.com</span>
    ODWFieldNameImpliesLocation = 6,      // Field name sounds like location data
    ODWFileNameOrExtension = 7,           // A file extension from the reportable list of extensions (ignores code files)
    ODWFileSharingUrl = 8,                // A URL referencing a common file-sharing site or service.
    ODWInScopeIdentifier = 9,             // Any authenticated identifier of the same types used for DSR.
    ODWInScopeIdentifierActiveUser = 10,  // The current users EUPI for DSR
    ODWInternalEmailAddress = 11,         // SMTP ending with <span>microsoft.com</span>
    ODWIpAddress = 12,                    // Machine's current IP address
    ODWLocation = 13,                     // Data appears to specify a location in the real world
    ODWMachineName = 14,                  // Machine name
    ODWOutOfScopeIdentifier = 15,         // Identifier that is potentially out-of-scope for DSR
    ODWPIDKey = 16,                       // Product key
    ODWSecurity = 17,                     // A URL containing parameters "access_token", "password", etc.
    ODWUrl = 18,                          // Any URL
    ODWUserAlias = 19,                    // Current user's alias
    ODWUserDomain = 20,                   // User/Machine domain
    ODWUserName = 21                      // Current user's name or part of it.
};

/*!
 The <b>ODWPrivacyGuard</b> class represents Privacy Guard Hook.
*/
@interface ODWPrivacyGuard : NSObject

#pragma mark Behavior methods
/*!
 @brief Set Privacy Guard Enabled state.
 @param enabled A boolean representing the enabled state for Privacy Guard.
 */
+(void)setEnabled:(bool)enabled;

/*!
 @brief Check whether Privacy Guard is enabled.
 @return True if Privacy Guard is enabled, false otherwise.
*/
+(bool)enabled;

/*!
 @brief Append fresh Common Data Contexts to the existing instance of Privacy Guard.
 @param freshCommonDataContext Fresh Common Data Contexts instance.
 */
+(void)appendCommonDataContext:(ODWCommonDataContext *) freshCommonDataContext;

/*!
 @brief Add ignored concern to prevent generation of notification signals when this
 concern is found for the given EventName and Field combination.
 @param EventName Event that the ignored concern should apply to. <b>Note:</b> If the ignored concern applies to Semantic Context field, set the Event name to 'SemanticContext'.
 @param FieldName Field that the ignored concern should apply to.
 @param IgnoredConcern The concern that is expected and should be ignored.
 */
+(void)addIgnoredConcern:(NSString *) EventName withNSString:(NSString *)FieldName withODWDataConcernType:(ODWDataConcernType)IgnoredConcern;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
