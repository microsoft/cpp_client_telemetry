# Custom Transmit Profiles

The SDK packages and uploads most events periodically. As long as there are events waiting and uploading is not disabled or paused, this process runs on a fixed cadence, set by the current transmit profile. A client may either choose one of the built-in profiles named `REAL_TIME`, `NEAR_REAL_TIME`, or `BEST_EFFORT`, or it can provide and select a custom profile. The built-in profiles are defined near the beginning of `lib/tpm/TransmitProfiles.cpp` if you wish to see how they are defined.

A profile can include multiple rules to determine how it behaves based on the current state of device network and device power. For instance, a profile can (and the default profiles do) cause the SDK to upload less often when the device is charging or on a higher-cost network.

To load custom profiles, pass a JSON string containing the profiles (described below) to `TransmitProfiles::load()`. To select a profile by name, pass the name (as a string) to `TransmitProfiles::setProfile()`. The name may be the name of one of the built-in profiles or the name of a profile previously passed to `TransmitProfiles::load()`. Both `load` and `setProfile` return a boolean, where true indicates success and false indicates failure.

## Example Json

```json
[
    {
        "name": "Fred",
        "rules": [
            {
              "netcost": "metered",
              "timers": [-1, -1, 60]
            },
            {
              "powerState": "battery",
              "timers": [ 60, 60, 30 ]
            },
            {
              "timers": [ 10, 10, 5 ]
            }
        ]
    }
]
```

This defines a single profile named Fred. This JSON contains a one-element array; the `load` method will parse one or more profiles from a single string as a JSON array. In this case we are defining a single profile with three rules. Each rule is a hash with two optional members (`netcost` and `powerState`) and one required member (`timers`).

## Example Decoded

The `Fred` profile has three rules. The first is selected whenever the device is on a metered network. It turns off upload for `EventLatency_Normal` and `EventLatency_CostDeferred` events, and accumulates and uploads `EventLatency_RealTime` events 60 seconds at a time.

The second rule is selected if the first rule does not match and the device is running on battery. It uploads `EventLatency_RealTime` events every 30 seconds, and lower priority events every 60 seconds.

The final rule is selected if no earlier rule matched. It uploads `EventLatency_RealTime` events every 5 seconds, and other events every 10 seconds.

### Netcost selector

A rule may include a `"netcost":"value"` key-value pair. Including this will restrict the rule by network cost:

Selector | Meaning
-------- | -------
roaming | Network marked as roaming (highest cost). Some platforms do not report this cost.
restricted | Synonym for roaming
metered | Network marked as higher cost
high | Synonym for metered
unmetered | Lower-cost network
low | Synonym for unmetered
unknown | Device has not determined network cost
any | Matches any network cost (omitting the selector also matches any network cost)

### PowerState selector

A rule may include a `"powerState":"value"` key-value pair. Including this will restrict the rule by power state:

Selector | Meaning
-------- | -------
battery | Device is operating on battery power
charging | Device is connected to outside power
unknown | Device has not determined its power state.
any | Matches any power state (omitting the selector also matches any power state)

### Selectors and Rule Precedence

If a rule includes selectors, all selectors must match in order to choose the rule. The first rule that matches will be chosen. The example JSON includes a default rule that will always match (because it has no selectors). A match-everything default rule should always be the last rule (rules following it can never be selected since it matches everything, and the first matching rule wins).

### Timers

Each rule must include a timers array. The timers array should contain three values. The first value controls upload of `EventLatency_Normal` and `EventLatency_CostDeferred` events. The second value is not used and is retained for backwards compatibility. The third value controls and sets the cadence for `EventLatency_RealTime` events. For example, if it is 5, the SDK will accumulate and upload `EventLatency_RealTime` events every 5 seconds.

#### Negative Timer Values

If the first timer value is negative, no `EventLatency_Normal` or `EventLatency_CostDeferred` events will be uploaded. If the third timer value is negative, only `EventLatency_Max` events will be uploaded.

#### Cadence And EventLatency

When the first and third timer values are both positive, the value of the first value does not matter (as of this writing). `EventLatency_Normal` and `EventLatency_CostDeferred` events will be collected and uploaded half as often as `EventLatency_RealTime` events.

#### Best Practice

Following these suggestions will help make the rule definition and SDK behavior clear:

* The first and second timer values should be equal. The SDK ignores the second timer value, but it once controlled `EventLatency_CostDeferred` events. Setting the two numbers to the same value documents that the two latencies are treated identically.
* If the first and third timer values are positive, the first and second should be twice the third. For example, `[4, 4, 2]` follows this rule. This matches the SDK's behavior: lower priority events are uploaded half as often as `EventLatency_RealTime` events.
* If the third timer value is negative, the first and second should also be negative. The SDK will not upload any of these events if the third timer value is negative, so setting all three to the same negative value matches this behavior.
* Use -1 consistently to disable upload and avoid the appearance of any special meaning for other negative values.
