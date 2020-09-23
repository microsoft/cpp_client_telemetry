#include "objc_begin.h"
#import "ODWLogger.h"
#include "IDataInspector.hpp"

NS_ASSUME_NONNULL_BEGIN

/*!
 @brief Common Privacy Data Contexts that should be searched for in the uploaded data for potential Privacy Concerns.
 */
@interface ODWCommonDataContexts : NSObject

/*!
 @brief Domain Name for the current machine
 */
@property NSString* DomainName;

/*!
 @brief Friendly Machine Name
 */
@property NSString* MachineName;

/*!
 @brief User Name such as the Login Name.
 */
@property NSString* UserName;

/*!
 @brief User Alias, if different that UserName
 */
@property NSString* UserAlias;

/*!
 @brief IP Addresses for local network ports such as IPv4, IPv6, etc.
 */
@property NSArray* IpAddresses;

/*!
 @brief Collection of Language Identifiers
 */
@property NSArray* LanguageIdentifiers;

/*!
 @brief Collection of Machine Identifies such as Machine Name, Motherboard ID, MAC Address, etc.
 */
@property NSArray* MachineIds;

/*!
 @brief Collection of Out-of-Scope identifiers such as Client ID, etc.
 */
@property NSArray* OutOfScopeIdentifiers;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
