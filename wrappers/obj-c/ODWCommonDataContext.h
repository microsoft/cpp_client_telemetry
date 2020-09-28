#include "objc_begin.h"
#import <Foundation/Foundation.h>

/*!
 @brief Common Privacy Data Contexts that should be searched for in the uploaded data for potential Privacy Concerns.
 */
struct ODWCommonDataContext
{

/*!
 @brief Domain Name for the current machine
 */
NSString* DomainName;

/*!
 @brief Friendly Machine Name
 */
NSString* MachineName;

/*!
 @brief User Name such as the Login Name.
 */
NSString* UserName;

/*!
 @brief User Alias, if different that UserName
 */
NSString* UserAlias;

/*!
 @brief IP Addresses for local network ports such as IPv4, IPv6, etc.
 */
NSArray* IpAddresses;

/*!
 @brief Collection of Language Identifiers
 */
NSArray* LanguageIdentifiers;

/*!
 @brief Collection of Machine Identifies such as Machine Name, Motherboard ID, MAC Address, etc.
 */
NSArray* MachineIds;

/*!
 @brief Collection of Out-of-Scope identifiers such as Client ID, etc.
 */
NSArray* OutOfScopeIdentifiers;

};


#include "objc_end.h"
