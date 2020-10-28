//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using Microsoft.Applications.Events;
using NUnit.Framework;
#if __ANDROID__
using Android.App;
using System.Collections.Generic;
#elif __IOS__
using Foundation;
#endif

namespace OneDsCppSdk.Bindings.Tests
{
    [TestFixture]
    public class EventPropertiesTests
    {
        private const string EventName = "eventname";
        private const string PropertyName = "propertyname";
        private EventProperties eventProperties;

        [SetUp]
        public void Setup()
        {
#if __ANDROID__
            LogManager.Initialize(Application.Context, "token");
#elif __IOS__
            LogManager.Initialize("token");
#endif
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
        }

        [Test]
        public void ConstructorWithNameAndProperties()
        {
            var ep = new EventProperties(EventName, DataTypes.TestProperties);
        }

        [Test]
        public void SetPropertyWithStringValue()
        {
            eventProperties.SetProperty(PropertyName, "value");
        }

        [Test]
        public void SetPropertyWithStringValueAndPiiKind()
        {
            eventProperties.SetProperty(PropertyName, "value", PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithDoubleValue()
        {
            eventProperties.SetProperty(PropertyName, double.MaxValue, PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithDoubleValueAndPiiKind()
        {
            eventProperties.SetProperty(PropertyName, double.MaxValue, PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithLongValue()
        {
            eventProperties.SetProperty(PropertyName, long.MaxValue, PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithLongValueAndPiiKind()
        {
            eventProperties.SetProperty(PropertyName, long.MaxValue, PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithByteValue()
        {
            eventProperties.SetProperty(PropertyName, byte.MaxValue, PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithByteValueAndPiiKind()
        {
            eventProperties.SetProperty(PropertyName, byte.MaxValue, PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithULongValue()
        {
            eventProperties.SetProperty(PropertyName, ulong.MaxValue, PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithULongValueAndPiiKind()
        {
            eventProperties.SetProperty(PropertyName, ulong.MaxValue, PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithBoolValue()
        {
            eventProperties.SetProperty(PropertyName, true, PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithBoolValueAndPiiKind()
        {
            eventProperties.SetProperty(PropertyName, true, PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithUUID()
        {
            eventProperties.SetProperty(PropertyName, DataTypes.TestGuid);
        }

        [Test]
        public void SetPropertyWithUUIDAndPiiKind()
        {
            eventProperties.SetProperty(PropertyName, DataTypes.TestGuid, PiiKind.GenericData);
        }

        [Test]
        public void SetPropertyWithDate()
        {
            eventProperties.SetProperty(PropertyName, DataTypes.TestDate);
        }

        [Test]
        public void SetPropertyWithDatePiiKind()
        {
            eventProperties.SetProperty(PropertyName, DataTypes.TestDate, PiiKind.GenericData);
        }
    }
}
