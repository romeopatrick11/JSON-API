# Nymi SDK 4.0 

Release Date: October 2016

## Release Overview

This release contains general improvements and bug fixes for Nymi-enabled application development.

## Improvements

* If an operation is submitted for a provision that has not yet been authenticated or that is associated with a Nymi Band that has been unclasped, the work is now immediately refused with a descriptive error
message. Formerly the operation would either fail, timeout, or silently be ignored.

* Documentation of JSON data improved.

* Documentation of C++ and C interface has been added.
** [NAPI C++ Reference](http://downloads.nymi.com/sdkDoc/latest/napi-cpp/index.html)
** [NAPI C Reference](http://downloads.nymi.com/sdkDoc/latest/napi-c/index.html)

* Some preconditions for NAPI requests are checked earlier, resulting in better error reporting.

## Fixes

* It was possible, but very unlikely, for an the outcome of an operation to not be reported to the NEA.

* A possible deadlock has been removed from the revoke provision code.

* On rare occasions, NAPI would incorrectly report terminate/finish as having timed out.

* Some unexpected/illegal BLE states that used to cause NAPI to stop working are now handled properly, and force a restart of the NEA. 

* If an attempt to connect to a Nymi Band (over Bluetooth) failed but advertising packets from the Nymi Band were still being received (that is, the Nymi Band was still `Present::yes`), it was possible that NAPI would not retry to connect. <br>
*Fix:* The retry will now happen after 15 seconds. The delay in retry is long in 'user-time' and is something that we want to reduce in a future release. The timeout on an operation, if set, is respected
(as it was before this fix) and the error result will be 'timeout' -- it will not say that the connection could not be made.

## Other Changes

* Roaming Authentication requires the Nymi Band to be disconnected. This does not affect how the function is used, since NAPI handles connections transparently.

## Known Issues 

* The SDK is not currently supported on macOS Sierra.


<hr width=600>


# Release Date: September 2016

## Release Overview

The Nymi SDK has changed significantly since the previous release. 

* In SDK 4.0, the Nymi JSON API now runs as an in-process static library, instead of as a websocket server as in SDK 3.1. 
* The JSON protocol has also changed. For details, see the [Nymi API JSON Reference](http://downloads.nymi.com/sdkDoc/latest/jsonreference/index.html) documentation.
* SDK 4.0 also includes support for a new Bluetooth® advertisement protocol that enhances performance and reliable communication with the Nymi Band.
* SDK 4.0 Documentation is available at: http://downloads.nymi.com/sdkDoc/latest/index.html

Nymi-enabled applications (NEAs) written with SDK 3.1 must be updated to account for the new in-process library and the JSON protocol changes.


## Known Issues 

### Detecting State Changes

* *Important:* After provisioning a Nymi Band, an NEA *must* wait for an `onFoundChange` event containing the provision ID (`pid`) of the target Nymi Band *and* an `after` state of `Found::Authenticated` *before* it attempts any other provision-specific actions on the Nymi Band. <br>
When an `onFoundChange` `after` state of any value other than `Found::Authenticated` is received, the NEA must stop attempting any provision-specific actions.
* The `onPresenceChange` notification is triggered when the *presence* state of a Nymi Band changes. Presence indicates whether a Bluetooth advertisement has been received from the Nymi Band and how recently. Therefore, the `authenticated` attribute of the `onPresenceChange` notification may indicate an unauthenticated Nymi Band, even if the Nymi Band then immediately authenticates. <br>
In order to properly detect a newly-authenticated Nymi Band, the NEA should rely on the `onFoundChange` callback.

### Bluetooth 

* An issue exists that may occur on some OS X machines that may cause Bluetooth events to not be seen by the SDK. To detect if this issue has occurred: open the `ncl.log` file and look for entries such as `handle 0 ignored cmd 00 00 00 00`. The Nymulator will work, but Nymi Bands will fail to respond.




