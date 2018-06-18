OneCollector Bond schema
==========================

### Finding the right schema

There are numerous variants of OneCollector schema available in
existing client telemetry projects. One can find all of them by
performing a VSTS code search e.g. for `ext:bond PIIExtensions`.

Many of these files have a header like this or similar:

``` cpp
// This class contains the only accepted format for the Skype Data RV
// Collector servers. Client libraries should use *only* this schema to send
// logs to the Skype Data RV.
```

But there is no version number or anything in this header and there are
many versions with different local changes laying around...

So instead of trying to find the correct client-side schema, it seemed
better to go from the other side, because what the server accepts is
ultimately the right schema to use.

OneCollector service source code lives in the
[infrastructure\_data\_pipe](https://skype.visualstudio.com/ARIA/_git/infrastructure_data_pipe)
repository and the Bond schema itself lies in file
[Services/Models/Models/Bond/BondDataPackage.bond](https://skype.visualstudio.com/ARIA/_git/infrastructure_data_pipe?path=%2FServices%2FModels%2FModels%2FBond%2FBondDataPackage.bond&version=GBmaster&_a=contents).

### Custom modifications

The collector service schema seems quite stable, changed just a few
times in the last years, but it also contains a lot of unrelated fields
for legacy DataRV compatibility or further server-side processing only.
Because these fields would generate unnecessary code and could also
sometimes cause problems with the limited capabilities of the Bond Lite
serializer, the schema is further processed with a custom Python script
`tools/AriaProtocol-generate.py`.

The script copies over just enumerations, structures and their fields
which are necessary for modern 1DS SDK operation (based on the limited
set of fields present in the [ReferenceSDK branch of
data\_infrastructure\_clienttelemetry](https://skype.visualstudio.com/SCC/_git/infrastructure_data_clienttelemetry?path=%2Fservicetelemetry%2Fjava%2Faria-core%2Fsrc%2Fmain%2Fjava%2Fcom%2Fmicrosoft%2Fapplications%2Ftelemetry%2Feventsubscribers%2Faria%2Fdatamodels%2FClients.bond&version=GBReferenceSDK&_a=contents)).
It also performs a few other cleanup tasks and stores the result into
`lib/bond/schema/AriaProtocol.bond`. The script is self-describing, all
its operations are noted in the output file as comments. The new, unused
name `AriaProtocol` was chosen instead of the previous `DataPackage`
etc. to avoid confusion with other 1DS schema files in existence.

### Schema updates

If the schema needs to be updated, follow this process:

1.  Update the local copy `BondDataPackage.bond` from
    [infrastructure\_data\_pipe](https://skype.visualstudio.com/ARIA/_git/infrastructure_data_pipe).
2.  Run `AriaProtocol-generate.py` (update the script if necessary to
    generate good output).
3.  If some bigger changes were necessary, document them in this file.
4.  Regenerate files in `../generated` as described in Bond Lite
    documentation.
5.  Run SDK tests, fix any incompatibilities in the C++ code.
6.  Commit everything related to the schema update in one commit with a
    clear commit message about updating collector Bond schema.
7.  Continue with implementing any new code functionality in a separate
    commit.

