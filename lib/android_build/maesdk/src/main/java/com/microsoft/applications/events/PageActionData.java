//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * The PageActionData structure represents the data of a page action event.
 */
public class PageActionData {
    /**
     * [Required] The ID of the page view associated with this action.
     */
    public String pageViewId;

    /**
     * [Required] A generic abstraction of the type of page action.
     */
    public ActionType actionType;

    /**
     * [Optional] The type of physical action, as one of the RawActionType enumeration values.
     */
    public RawActionType rawActionType;

    /**
     * [Optional] The type of input device that generates this page action.
     */
    public InputDeviceType inputDeviceType;

    /**
     * [Optional] The ID of the item on which this action acts.
     */
    public String targetItemId = "";

    /**
     * [Optional] The name of the data source item upon which this action acts.
     */
    public String targetItemDataSourceName = "";

    /**
     * [Optional] The name of the data source category that the item belongs to.
     */
    public String targetItemDataSourceCategory = "";

    /**
     * [Optional] The name of the data source collection that the item belongs to.
     */
    public String targetItemDataSourceCollection = "";

    /**
     * [Optional] The name of the layout container the item belongs to.
     */
    public String targetItemLayoutContainer = "";

    /**
     * [Optional] The relative ordering/ranking/positioning within the layout container.
     */
    public short targetItemLayoutRank;

    /**
     * The destination URI resulted by this action.
     */
    public String destinationUri = "";

    /**
     * A constructor that takes a page view ID, and an action type.
     *
     * @param pvId page view ID
     * @param actType action type
     */
    public PageActionData(final String pvId, final ActionType actType) {
        pageViewId = pvId;
        actionType = actType;
        rawActionType = RawActionType.Unspecified;
        inputDeviceType = InputDeviceType.Unspecified;
        targetItemLayoutRank = 0;
    }
}

