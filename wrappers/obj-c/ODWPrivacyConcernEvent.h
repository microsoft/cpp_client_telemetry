//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"

NS_ASSUME_NONNULL_BEGIN

/*! 
 @brief Represents a Privacy Concern Event.
 This class is used to store and manage privacy concern event data including details
 like the event name, server name, database name, and various other privacy-related
 attributes.
 */
@interface ODWPrivacyConcernEvent : NSObject

/*! 
 @brief The name of the privacy concern event.
 Used as a unique identifier for the event. Example: "PrivacyConcerns".
 */
@property (nonatomic, strong) NSString *PG_ConcernEventName;

/*! 
 @brief The name of the server associated with the privacy concern event.
 This field is optional and may be nil.
 */
@property (nonatomic, strong, nullable) NSString *PG_ServerName;

/*! 
 @brief The name of the database associated with the privacy concern event.
 This field is optional and may be nil.
 */
@property (nonatomic, strong, nullable) NSString *PG_DatabaseName;

/*! 
 @brief The table name that contains privacy concern event data.
 This field is mandatory and used to locate the specific data in a table.
 */
@property (nonatomic, strong) NSString *PG_TableName;

/*! 
 @brief The column name in the table that stores the privacy concern data.
 This field is mandatory and specifies which column to examine.
 */
@property (nonatomic, strong) NSString *PG_ColumnName;

/*! 
 @brief The locator name to uniquely identify the row in the table containing the privacy concern.
 This field is optional and can be nil if not needed.
 */
@property (nonatomic, strong, nullable) NSString *PG_EventLocatorName;

/*! 
 @brief The locator value associated with the PG_EventLocatorName, which can uniquely identify
 the specific row containing the privacy concern.
 This field is optional and can be nil if not needed.
 */
@property (nonatomic, strong, nullable) NSString *PG_EventLocatorValue;

/*! 
 @brief The timestamp of the privacy concern event.
 This field is mandatory and represents the time the event occurred.
 */
@property (nonatomic, assign) int64_t PG_EventTime;

/*! 
 @brief The type of privacy concern event as a string.
 This field is mandatory and can describe the concern (e.g., "Sensitive Data Leak").
 */
@property (nonatomic, strong) NSString *PG_ConcernTypeText;

/*! 
 @brief Whether the privacy concern event should be ignored.
 This boolean determines if the concern will be processed or ignored in reporting.
 */
@property (nonatomic, assign) BOOL PG_ShouldIgnore;

/*! 
 @brief Indicates whether the data field is considered as a semantic context.
 This boolean indicates if this concern applies universally to the table or to a subset of data.
 */
@property (nonatomic, assign) BOOL PG_IsContext;

/*! 
 @brief Indicates whether the semantic context applies to all records or just a subset.
 This boolean helps categorize whether the context is global or partial.
 */
@property (nonatomic, assign) BOOL PG_IsGlobalContext;

/*! 
 @brief The tenant ID associated with the privacy concern event.
 This field is optional and can be used to filter events based on tenant.
 */
@property (nonatomic, strong, nullable) NSString *PG_AssociatedTenant;

/*! 
 @brief The environment or deployment ring for the service (e.g., "Production", "PPE").
 This field is optional and can specify the deployment context of the privacy concern.
 */
@property (nonatomic, strong, nullable) NSString *PG_Environment;

/*! 
 @brief Additional metadata that can be attached to the privacy concern event.
 This field is optional and allows for flexible extension of the event data.
 */
@property (nonatomic, strong, nullable) NSString *PG_Metadata;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
