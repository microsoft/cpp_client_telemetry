//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"

NS_ASSUME_NONNULL_BEGIN
/*!
 @brief Common Privacy Data Contexts that should be searched for in the uploaded data for potential Privacy Concerns.
 */
@interface ODWCommonDataContext : NSObject
/*!
 @brief Domain Name for the current machine
 */
@property(readwrite, copy, nonatomic) NSString* DomainName;

/*!
 @brief Friendly Machine Name
 */
@property(readwrite, copy, nonatomic) NSString* MachineName;

/*!
 @brief User Name such as the Login Name.
 */
@property(readwrite, copy, nonatomic) NSString* UserName;

/*!
 @brief User Alias, if different that UserName
 */
@property(readwrite, copy, nonatomic) NSString* UserAlias;

/*!
 @brief IP Addresses for local network ports such as IPv4, IPv6, etc.
 */
@property(readwrite, copy, nonatomic) NSMutableArray* IpAddresses;

/*!
 @brief Collection of Language Identifiers
 */
@property(readwrite, copy, nonatomic) NSMutableArray* LanguageIdentifiers;

/*!
 @brief Collection of Machine Identifies such as Machine Name, Motherboard ID, MAC Address, etc.
 */
@property(readwrite, copy, nonatomic) NSMutableArray* MachineIds;

/*!
 @brief Collection of Out-of-Scope identifiers such as Client ID, etc.
 */
@property(readwrite, copy, nonatomic) NSMutableArray* OutOfScopeIdentifiers;

@end
NS_ASSUME_NONNULL_END

#include "objc_end.h"
