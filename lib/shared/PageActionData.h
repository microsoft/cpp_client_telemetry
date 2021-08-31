//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include "PlatformHelpers.h"
#include "SchemaStub.hpp"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry  {
            namespace Windows
            {
                public ref struct PageActionData sealed
                {
                    /// [Required] ID of the page view with which this action is associated
                    property String^ PageViewId;

                    /// [Required] Generic abstraction of the type of the action
                    property ActionType ActionType;

                    /// [Optional] the type of Physical action
                    property RawActionType RawActionType;

                    /// [Optional] the type of input device that generates this action
                    property InputDeviceType InputDeviceType;

                    /// [Optional] ID of the item on which this action acts
                    property String^ TargetItemId;

                    /// [Optional] Name of the data source item on which this action acts
                    property String^ TargetItemDataSourceName;

                    /// [Optional] Name of the data source category the item belongs to
                    property String^ TargetItemDataSourceCategory;

                    /// [Optional] Name of the data source colletion the item belongs to
                    property String^ TargetItemDataSourceCollection;

                    /// [Optional] Name of the layout container to which the item belongs
                    property String^ TargetItemLayoutContainer;

                    /// [Optional] Relative ordering/ranking/positioning within the layout container item has
                    property unsigned short TargetItemLayoutRank;

                    /// [Optional] Destination Uri resulted by this action
                    property String^ DestinationUri;
                };
            }
        }
    }
}
