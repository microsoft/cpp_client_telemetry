# Data Viewing as a 1SDK First Class Citizen

## Summary
Microsoft products require support for data viewing capabilities where we can show
the data being uploaded by the 1SDK in a custom viewer. A core principle of the viewing
capabilities is that we should not modify the data being uploaded by routing it through
a proxy or another machine. The data being uploaded should be comparable to the data
being displayed in the custom viewer. This approach allows Microsoft to earn customers
trust and guarantee them the highest level of transparency possible.

Office has implemented this support in DevMain, but the 1SDK does not natively support
this. As it stands today, consumers of the 1SDK must design and implement this feature
on their own, duplicating much of the work.

## Problem Space

Microsoft products must support data viewing capabilities to meet customer
promises with regards to data transparency.

## Proposed Solution
#### `IDataViewer` Interface

    class IDataViewer
    {
    public:
        virtual void ReceiveData(const std::vector<std::uint8_t>& packetData) noexcept = 0;
        virtual const char* const GetName() const noexcept = 0;
    };

This is intended to be a simple interface that can be integrated into the 1SDK. It will
allow for extension of data viewing capabilities by allowing consumer to extend this
interface class.

`ReceiveData` will receive a reference to the packet and allow data viewers to
consume it as they see appropriate. Having a `const` reference will allow us to save the cost of
creating another copy of the vector.

`GetName` returns the name of the data viewer to help in ensuring a given data viewer
is registered exactly once, being able to unregister a specific data viewer and
to validate if a given viewer is registered.

#### `IDataViewerCollection` Accessors

    class IDataViewerCollection
    {
    public:
       virtual void RegisterViewer(const std::shared_ptr<IDataViewer>& dataViewer) = 0;
       virtual void UnregisterViewer(const char* viewerName) = 0;
       virtual void UnregisterAllViewers() = 0;
       virtual bool IsViewerRegistered(const char* viewerName) const = 0;
       virtual bool IsViewerRegistered() const noexcept = 0;

       virtual bool DispatchDataViewerEvent(const std::vector<std::uint8_t>& dataPacket) const noexcept = 0;
    };

The `IDataViewerCollection` will allow for managing a set of `IDataViewer`
objects and checking their registration status. The `IsViewerEnabled` and
`AreAnyViewersEnabled` methods return the result of searching for a given
viewer in the viewer collection, without modifying the `IDataViewerCollection`.
Having functionalities such as registration, unregistration and ability to
check if any viewer is registered, directly on `IDataViewerCollection`
allows a unified viewer management capability that will affect all ILoggers
registered with the ILogManager.

The implementations of `IDataViewer` shall be passed as `const std::shared_ptr<IDataViewer>&`
as the SDK host should be able to access any custom methods within their implementation
of `IDataViewer`. Additionally, both the SDK and the SDK host should be able
to jointly manage the lifetime of the implementation as a last-user-cleans-up
approach.

The `DispatchDataViewerEvent` method will be used internally to dispatch
a packet to the `IDataViewerCollection` implementation, which subsequently
conveys it to all registered data viewers. The current plan is that
`HttpRequestEncoder` should call this method as it encodes data to be sent.
Given that we will send the event to the viewer before attempting network calls,
this approach allows us to send each packet exactly once to the viewer, without
having to add fall-back logic in case of retries or network failures. The
dispatching of the event to the registered viewers is not done in any particular order.

In terms of lock contention, `RegisterViewer` and `UnregisterViewer` methods take
exclusive locks as they are allowed to modify the internal map of registered data viewers.
The `DispatchDataViewerEvent` and `IsViewerRegistered` take shared locks as they
only need the knowledge regarding the current state of registered viewers and perform
relevant operations on that.

To provide access to `IDataViewerCollection`, `ILogManager` Accessors
will be added as below and the `LogManagerImpl` will store a unique pointer to the
implemented instance of `IDataViewerCollection`.

#### `ILogManager` Accessors

    class ILogManager
    {
    public:
       ...
        virtual IDataViewerCollection& GetDataViewerCollection() = 0;
        virtual const IDataViewerCollection& GetDataViewerCollection() const = 0;
    };

## Abstract Decision Tree

Once we have instantiated a `DataViewerCollection` instance, the following
decisions can be made.

#### Registering `IDataViewer`

* Invoke `IDataViewerCollection::RegisterViewer(ViewerImpl)`
    * If `ViewerImpl` is not registered, register it.
    * Else, throw exception for duplicate viewer registration.

#### Post-Registration

* 1SDK generates an HTTP encoded packet and sends it to `IDataViewerCollection`
before calling the upload method.
* Identify all registered viewers.
* For each viewer, synchronously invoke `IDataViewer::ReceiveData(packet)`.

#### Unregistering `IDataViewer`

* Invoke `IDataViewerCollection::UnregisterViewer("ViewerImplName")`
    * If ViewerImpl is registered, unregister it.
    * Else, throw exception for viewer not being registered.

#### Unregistering all registered instances of `IDataViewer`

* Invoke `IDataViewerCollection::UnregisterAllViewers()`
    * Clear data viewer collection map.

## Alternatives Considered

As alternatives to the proposed solution, it was considered if providing a
plug-in to for an external packet sniffing tool, such as Fiddler, would be
appropriate/sufficient. We decided against that as the tools are external
3rd party. We should not take a dependency on any external tools that may
change their usage or privacy policies and break our dependency on it. Additionally,
from our experience working with customers who have requested data transparency
capabilities, they do not prefer to install root level certificates and
perform Man-In-The-Middle sniffing approaches. For full transparency of
the uploaded data, we needed to establish a pipeline that will be available
in parallel to the upload pipeline.

## Microsoft Data Viewer Default Implementation

Office has currently implemented support for data viewing capabilities in
Windows Diagnostic Data Viewer (DDV). In the current form, it supports
making DDV connection via remote HTTP connection, or local via App Services
connection only for Windows 10 RS4+ builds.

In the 1SDK implementation, the implementation will be converged for all
consumers of the SDK in the Default implementation of the IDataViewer
component. Additionally, it will build support the existing connectivity
capabilities, but not limit the endpoint to just DDV. This will allow
consumers to decide how they wish their 1SDK data to be shown for any
transparency requirements they may have.

    class DefaultDataViewer
    {
        DefaultDataViewer(std::shared_ptr<IHttpClient> httpClient, const char* machineFriendlyIdentifier)
        void ReceiveData(const std::vector<std::uint8_t>& dataPacket) noexcept override;
        const char* GetName() const noexcept override;
        bool EnableRemoteViewer(const std::string& endpoint);
        bool EnableLocalViewer();
        bool EnableLocalViewer(const std::string& AppId, const std::string& AppPackage);
        bool DisableViewer();
        ...
    };

#### `DefaultDataViewer` Constructor
The `DefaultDataViewer` object allows the consumer to set the Http
client to be used for remote connections. This is especially helpful
when the SDK is using a different HTTP library than the one it provides
natively. Additionally, when sending data to the data viewer, it is
recommended to send a friendly machine identifier. This allows the
viewer to group incoming data by machine, and provide a user friendly
name to the group as well, improving consumer experience.

#### `EnableRemoteViewer`

In this method, the `endpoint` parameter would be the endpoint where the
data viewer is running. This will allow the SDK to know where to make
connections using the HTTP pipeline. The result from the method would
signify whether the connection was successful or not.

#### `EnableLocalViewer`

In this method, the SDK will attempt to connect to a locally running
instance of a diagnostic viewer on Windows. By default, it will try
to establish a connection with the Diagnostic Data Viewer. Optionally,
it can be provided `AppID` and `AppPackage` information to connect
to another application. The Local connection would be established using
App Services. Like the remote connection, the result here would signify
whether the connection was successful or not.

### Implementation Notes

From implementation perspective, the goal will be to utilize existing
HTTP stack used by 1SDK to support HTTP remote connection. 

>**Note**: Due to the complexities involved with App Services connection,
>its lack of backward compatibilities with older versions of Windows, and the
>resource overhead of using App Services, it has been decided to **not
>integrate App Services support for local data viewing** in DDV at the
>time of writing this document.

#### DDV Communication Protocol
For a successful communication with DDV, an `HTTP` POST connection must
be used. The connection must also provide the following information
as either header fields, or as query parameters:
1. Machine Identifier (`Machine-Identifier`)
2. App Name (`App-Name`)
3. App Platform (`App-Platform`)

The contents of the connection must be the binary packet that 1DS SDK
provided to `ReceiveData` method as parameter, and should not be modified
in any form (zipping, bonding, etc.).

>**Note** : As of writing this document, DDV *does not* support `HTTPS`
>connection. The Default Data Viewer implementation explicitly validates
>that the connection endpoint is `HTTP` due to this.

#### Connection Duration
Network connection to DDV should be resilient to short network outages.
It is recommended that in case of a failed network connection, we should
wait at most 30 seconds, and attempt of retries should be limited to 3.
This recommendation is intended to allow restarting of DDV, without
risking streaming of data to an endpoint that is not running DDV.

An additional connection recommendation is to expire the network connection
every 24 hours. This will ensure that the connection is not opened and
forgotten by the user.

## Related Note
To support alternative viewer implementations, we should also
provide an appropriate Aria Packet Parser. The parser should contain
enough logic to unbond the network packet and allow the viewer implementer
to determine how they want to consume the data. Additionally, this should
be made available in the same location as the SDK for easy access to the
parser. This should be addressed at a later point when we allow consumption
via an application other than Windows DDV.