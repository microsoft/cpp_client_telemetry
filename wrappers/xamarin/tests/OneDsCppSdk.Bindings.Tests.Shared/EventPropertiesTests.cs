//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
extern alias NUnitLite;

using Microsoft.Applications.Events;
using NUnitLite::NUnit.Framework;
#if __ANDROID__
using Android.App;
using System.Collections.Generic;
#elif __IOS__
using Foundation;
#endif

namespace OneDsCppSdk.Bindings.Tests
{
    [TestFixture]
    public class EventPropertiesTests : BaseTests
    {
        private const string EventName = "eventname";
        private const string PropertyName = "propertyname";
        private EventProperties eventProperties;

        [SetUp]
        public void Setup()
        {
            LogManager.InitializeLogger("token");
            eventProperties = new EventProperties(EventName);
        }

        [Test]
        public void Name()
        {
            Assert.AreEqual(EventName, eventProperties.Name);
        }

        [Test]
        public void Priority()
        {
            Assert.AreNotEqual(EventPriority.High, eventProperties.Priority);
            eventProperties.Priority = EventPriority.High;
            Assert.AreEqual(EventPriority.High, eventProperties.Priority);
        }

        [Test]
        public void Properties()
        {
            Assert.IsNotNull(eventProperties.Properties);
        }

        [Test]
        public void ConstructorWithName()
        {
            var ep = new EventProperties(EventName);

            Assert.IsNotNull(ep);
        }

        [Test]
        public void ConstructorWithNameAndProperties()
        {
            var ep = new EventProperties(EventName, DataTypes.TestProperties);

            Assert.IsNotNull(ep);
        }

        [Test]
        public void SetPropertyWithStringValue()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, "value"));
        }

        [Test]
        public void SetPropertyWithStringValueAndPiiKind()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, "value", PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithDoubleValue()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, double.MaxValue, PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithDoubleValueAndPiiKind()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, double.MaxValue, PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithLongValue()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, long.MaxValue, PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithLongValueAndPiiKind()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, long.MaxValue, PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithByteValue()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, byte.MaxValue, PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithByteValueAndPiiKind()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, byte.MaxValue, PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithULongValue()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, ulong.MaxValue, PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithULongValueAndPiiKind()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, ulong.MaxValue, PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithBoolValue()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, true, PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithBoolValueAndPiiKind()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, true, PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithUUID()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, DataTypes.TestGuid));
        }

        [Test]
        public void SetPropertyWithUUIDAndPiiKind()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, DataTypes.TestGuid, PiiKind.GenericData));
        }

        [Test]
        public void SetPropertyWithDate()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, DataTypes.TestDate));
        }

        [Test]
        public void SetPropertyWithDatePiiKind()
        {
            ExecuteWithoutAssertion(() => eventProperties.SetProperty(PropertyName, DataTypes.TestDate, PiiKind.GenericData));
        }
    }
}
