//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 @enum ODWEventPriority
 @brief The <b>ODWEventPriority</b> enumeration contains a set of values that specify event priority.
 */
typedef NS_ENUM(NSInteger, ODWEventPriority)
{
    ODWEventPriorityUnspecified = -1,
    ODWEventPriorityOff = 0,
    ODWEventPriorityLow = 1,
    ODWEventPriorityNormal = 2,
    ODWEventPriorityHigh = 3,
    ODWEventPriorityImmediate = 4
};

/*!
 @enum ODWPiiKind
 @brief The <b>ODWPiiKind</b> enumeration contains a set of values that indicate the kind of PII (Personal Identifiable Information).
 */
typedef NS_ENUM(NSInteger, ODWPiiKind)
{
    ODWPiiKindNone = 0,              /**< None. */
    ODWPiiKindDistinguishedName = 1, /**< Distinguished name. */
    ODWPiiKindGenericData = 2,       /**< Generic data. */
    ODWPiiKindIPv4Address = 3,       /**< IPv4 Internet address. */
    ODWPiiKindIPv6Address = 4,       /**< IPv6 Internet address. */
    ODWPiiKindMailSubject = 5,       /**< Mail subject. */
    ODWPiiKindPhoneNumber = 6,       /**< Phone number. */
    ODWPiiKindQueryString = 7,       /**< Query string. */
    ODWPiiKindSipAddress = 8,        /**< SIP address. */
    ODWPiiKindSmtpAddress = 9,       /**< SMTP address. */
    ODWPiiKindIdentity = 10,         /**< Identify. */
    ODWPiiKindUri = 11,              /**< URI. */
    ODWPiiKindFqdn = 12,             /**< Fully qualified domain name. */
    ODWPiiKindIPV4AddressLegacy = 13 /**< Legacy IPv4 Internet address. */
};

/*!
 @enum ODWPrivacyDataType
 @brief The <b>ODWPrivacytDataType</b> enumeration contains a set of values that indicate various Privacy Data Types that data may be used for an event's privacy classification.
 @remark Explicitly specified as <b>uint64_t</b> as it needs to be able to hold 64-bit values.
 */
typedef NS_OPTIONS(uint64_t, ODWPrivacyDataType)
{
    ODWPrivacyDataTypeBrowsingHistory                    = 0x0000000000000002u,
    ODWPrivacyDataTypeDeviceConnectivityAndConfiguration = 0x0000000000000800u ,
    ODWPrivacyDataTypeInkingTypingAndSpeechUtterance     = 0x0000000000020000u,
    ODWPrivacyDataTypeProductAndServicePerformance       = 0x0000000001000000u,
    ODWPrivacyDataTypeProductAndServiceUsage             = 0x0000000002000000u,
    ODWPrivacyDataTypeSoftwareSetupAndInventory          = 0x0000000080000000u
};

/*!
 @enum ODWDiagLevel
 @brief The <b>ODWDiagLevel</b> enumeration contains a set of values that indicate the Diagnostic Level that an event may be classified as.
 @remarks By default, in the absence of explicit markup, an event is considered ODWDiagLevelOptionalDiagnosticData.
 */
typedef NS_ENUM(uint8_t, ODWDiagLevel)
{
    ODWDiagLevelRequiredDiagnosticData                  = 1,
    ODWDiagLevelOptionalDiagnosticData                  = 2,
    ODWDiagLevelRequiredServiceData                     = 110,
    ODWDiagLevelRequiredServiceDataForEssentialServices = 120
};

/*!
 The <b>ODWEventProperties</b> class represents an event's properties.
*/
@interface ODWEventProperties : NSObject

/*!
@brief Event name.
*/
@property(readwrite, copy, nonatomic) NSString* name;

/*!
@brief Event priority.
*/
@property(readwrite, nonatomic) ODWEventPriority priority;

/*!
 @brief Event properties. Key is property name, value is property value.
*/
@property(readonly, copy, nonatomic) NSDictionary<NSString *, id> * properties;

/*!
 @brief Event PII (personal identifiable information ) tags. Key is property name, value is ODWPiiKind value.
*/
@property(readonly, copy, nonatomic) NSDictionary<NSString*, NSNumber*> * piiTags;

/*!
 @brief Constructs an ODWEventProperties object, taking an event name.
 @param name A string that contains the name of the event.
 @return An instance of the ODWEventProperties interface.
 */
-(instancetype)initWithName:(NSString *)name;

/*!
 @brief Constructs an ODWEventProperties object, taking an event name and properties.
 @param name A string that contains the name of the event.
 @param properties A dictionary containing property name and value pairs.
 @return An instance of the ODWEventProperties interface.
 */
-(instancetype)initWithName:(NSString *)name
     properties:(NSDictionary<NSString *,id>*)properties;

/*!
 @brief Constructs an ODWEventProperties object, taking an event name and properties.
 @param name A string that contains the name of the event.
 @param properties A dictionary containing property name and value pairs.
 @param piiTags A dictionary containing property name and ODWPiiKind pairs.
 @return An instance of the ODWEventProperties interface.
 */
-(instancetype)initWithName:(nonnull NSString *)name
     properties:(NSDictionary<NSString*,id>*) properties
     piiTags:(NSDictionary<NSString*,NSNumber*>*) piiTags NS_DESIGNATED_INITIALIZER;

-(instancetype)init NS_UNAVAILABLE;

/*!
 @brief Sets a string property for an event.
 @param name A string that contains the name of the property.
 @param value A string that contains the property value.
 */
-(void)setProperty:(NSString *)name withValue:(id)value;

/*!
 @brief Sets a string property for an event.
 @param name A string that contains the name of the property.
 @param value A string that contains the property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
*/
- (void)setProperty:(NSString*)name withValue:(id)value withPiiKind:(ODWPiiKind)piiKind;

/*!
 @brief Sets a double property for an event.
 @param name A string that contains the name of the property.
 @param value A double that contains the property value.
 */
-(void)setProperty:(NSString*)name withDoubleValue:(double)value;

/*!
 @brief Sets a double property for an event.
 @param name A string that contains the name of the property.
 @param value A double that contains the property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setProperty:(NSString*)name withDoubleValue:(double)value withPiiKind:(ODWPiiKind)piiKind;

/*!
 @brief Sets an integer property for an event.
 @param name A string that contains the name of the property.
 @param value An integer that contains the property value.
 */
-(void)setProperty:(NSString*)name withInt64Value:(int64_t)value;

/*!
 @brief Sets an integer property for an event.
 @param name A string that contains the name of the property.
 @param value An integer that contains the property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setProperty:(NSString*)name withInt64Value:(int64_t)value withPiiKind:(ODWPiiKind)piiKind;

/*!
 @brief Sets an integer property for an event.
 @param name A string that contains the name of the property.
 @param value An integer that contains the property value.
 */
-(void)setProperty:(NSString*)name withUInt8Value:(uint8_t)value;

/*!
 @brief Sets an integer property for an event.
 @param name A string that contains the name of the property.
 @param value An integer that contains the property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setProperty:(NSString*)name withUInt8Value:(uint8_t)value withPiiKind:(ODWPiiKind)piiKind;


/*!
 @brief Sets an integer property for an event.
 @param name A string that contains the name of the property.
 @param value An integer that contains the property value.
 */
-(void)setProperty:(NSString*)name withUInt64Value:(uint64_t)value;

/*!
 @brief Sets an integer property for an event.
 @param name A string that contains the name of the property.
 @param value An integer that contains the property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setProperty:(NSString*)name withUInt64Value:(uint64_t)value withPiiKind:(ODWPiiKind)piiKind;


/*!
 @brief Sets a BOOL property for an event.
 @param name A string that contains the name of the property.
 @param value A BOOL that contains the property value.
 */
-(void)setProperty:(NSString*)name withBoolValue:(BOOL)value;

/*!
 @brief Sets a BOOL property for an event.
 @param name A string that contains the name of the property.
 @param value A BOOL that contains the property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setProperty:(NSString*)name withBoolValue:(BOOL)value withPiiKind:(ODWPiiKind)piiKind;

/*!
 @brief Sets a UUID property for an event.
 @param name A string that contains the name of the property.
 @param value A UUID that contains the property value.
 */
-(void)setProperty:(NSString*)name withUUIDValue:(NSUUID*)value;

/*!
 @brief Sets a UUID property for an event.
 @param name A string that contains the name of the property.
 @param value A UUID that contains the property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setProperty:(NSString*)name withUUIDValue:(NSUUID*)value withPiiKind:(ODWPiiKind)piiKind;

/*!
 @brief Sets a date property for an event.
 @param name A string that contains the name of the property.
 @param value A date that contains the property value.
 */
-(void)setProperty:(NSString*)name withDateValue:(NSDate*)value;

/*!
 @brief Sets the privacy Metadata for an event.
 @param privTags Privacy Data Type of the event.
 @param privLevel Privacy Diagnostic Level of the event.
 */
-(void)setPrivacyMetadata:(ODWPrivacyDataType)privTags withODWDiagLevel:(ODWDiagLevel)privLevel;

/*!
 @brief Sets a date property for an event.
 @param name A string that contains the name of the property.
 @param value A date that contains the property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setProperty:(NSString*)name withDateValue:(NSDate*)value withPiiKind:(ODWPiiKind)piiKind;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
