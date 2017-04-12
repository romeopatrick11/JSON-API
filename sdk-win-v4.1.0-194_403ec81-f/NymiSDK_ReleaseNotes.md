# Nymi SDK 4.1

Release Date: April 12, 2017

## Release Overview

This release contains significant improvements and functionality changes. NEA developers are advised to read and understand these changes before updating their projects to this release of the Nymi SDK. NEA code updates are required to consume these changes.

## Improvements

* Provision management has been restructured so that NEAs are now responsible for managing the provision data. The `provisions.json` file has been deprecated. There is provision management functionality that allows the NEA to add and remove provisions from consideration without restarting. Provide the saved provision data to NAPI as part of the call to `napi::configure`. 

For details, see the [documentation](TBD).

* C++/C entry points have been renamed to simplify and standardize the interface.
    For example:
    OLD: `nymi:jsonNapiGet`
    NEW: `napi:get`

For details, see the [documentation](TBD).

* The config.json file has been removed. Configuration options are now provided in the call to `napi::configure`.

* The `operation` field has been removed from JSON exchanges.

* NAPI for Windows is now available as a dynamic library. For details, see the [documentation](TBD).

* NAPI for Mac is now available as a framework. For details, see the [documentation](TBD).

* The SDK is now supported on macOS Sierra **10.12.2 or later**. 
   Note: Version 10.12.0 and 10.12.1 are NOT supported. 

* Improved Bluetooth error handling
    After a Bluetooth failure, NAPI now continues to attempt restarting indefinitely. Retries are 0.1s apart for the first 5 seconds, 1s apart for the first minute, 5s apart after that.
    Two new error messages have been added:
    ** "An attempted restart of NAPI has failed, will try again" – issued on the general error channel after a failed restart
    ** "NAPI is unable to restart, NEA must exit" – if something happened that excluded restarting

* NEAs can now choose to explicitly accept or reject a provision pattern through the action (accept, reject) argument to the `ProvisionPattern` call.

* The local signing function has been updated to require a setup call to NAPI before the signing function can be used. For details... 




## Fixes

* A number of existing and/or potential memory leaks have been fixed.

* NAPI internals now uses a secure JSON memory allocator.

* `napi::terminate` is considered safe to use, and will allow you to restart NAPI without restarting the entire NEA.




