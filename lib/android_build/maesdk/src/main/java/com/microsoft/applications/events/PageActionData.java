package com.microsoft.applications.events;

public class PageActionData {
    /// <summary>
    ///
    /// </summary>
    /**
     * [Required] The ID of the page view associated with this action.
     */
    public String pageViewId;

    /// <summary>
    ///
    /// </summary>
    /**
     * [Required] A generic abstraction of the type of page action.
     */
    public ActionType actionType;

    /// <summary>
    /// [Optional] The type of physical action, as one of the RawActionType enumeration values.
    /// </summary>
    public RawActionType rawActionType;

    /// <summary>
    ///
    /// </summary>
    /**
     * [Optional] The type of input device that generates this page action.
     */
    public InputDeviceType inputDeviceType;

    /// <summary>
    ///
    /// </summary>
    /**
     * [Optional] The ID of the item on which this action acts.
     */
    public String targetItemId;

    /// <summary>
    /// [Optional] The name of the data source item upon which this action acts.
    /// </summary>
    public String targetItemDataSourceName;

    /// <summary>
    /// [Optional] The name of the data source category that the item belongs to.
    /// </summary>
    public String targetItemDataSourceCategory;

    /// <summary>
    /// [Optional] The name of the data source collection that the item belongs to.
    /// </summary>
    public String targetItemDataSourceCollection;

    /// <summary>
    /// [Optional] The name of the layout container the item belongs to.
    /// </summary>
    public String targetItemLayoutContainer;

    /// <summary>
    /// [Optional] The relative ordering/ranking/positioning within the layout container.
    /// </summary>
    public short targetItemLayoutRank;

    /// <summary>
    /// [Optional] The destination URI resulted by this action.
    /// </summary>
    public String destinationUri;

    /// <summary>
    /// A constructor that takes a page view ID, and an action type.
    /// </summary>
    PageActionData(final String pvId, final ActionType actType) {
        pageViewId = pvId;
        actionType = actType;
        rawActionType = RawActionType.Unspecified;
        inputDeviceType = InputDeviceType.Unspecified;
        targetItemLayoutRank = 0;
    }
}
