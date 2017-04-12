# Nymi SDK 4.1 

Release Date: April 12, 2017

## Release Overview

This release contains significant improvements and functionality changes. NEA developers are advised to read and understand these changes before updating their projects to this release of the Nymi SDK. NEA code updates are required to consume these changes.

## Improvements

* Provision management has been restructured so that NEAs are now responsible for managing the provision data. The `provisions.json` file has been deprecated. There is provision management functionality that allows the NEA to add and remove provisions from consideration without restarting. NEAs can provide the existing saved provision data to NAPI as part of the call to `napi::configure`. <br>
For details, see [Migrating to NAPI 4.1](https://downloads.nymi.com/sdkDoc/latest/#migrating-to-napi-41).

* Important: Provisions can be shared only across Windows machines. A provision created on a macOS/OSX machine cannot be shared to other Mac or Windows machines.

* C++/C entry points have been renamed to simplify and standardize the interface.<br>
    For example:<br>
    OLD: `nymi:jsonNapiGet`<br>
    NEW: `napi:get`<br>
For details, see [Migrating to NAPI 4.1](https://downloads.nymi.com/sdkDoc/latest/#migrating-to-napi-41).

* The `config.json` file has been deprecated. Configuration options are now provided in the call to `napi::configure`.

* The `operation` field has been removed from JSON exchanges.

* NAPI for Windows is now available as a dynamic library. For details, see [Migrating to NAPI 4.1](https://downloads.nymi.com/sdkDoc/latest/#migrating-to-napi-41).

* NAPI for Mac is now available as a framework. For details, see [Migrating to NAPI 4.1](https://downloads.nymi.com/sdkDoc/latest/#migrating-to-napi-41).

* The SDK is now supported on macOS Sierra **10.12.2 or later**. 
   Note: Version 10.12.0 and 10.12.1 are NOT supported. 

* Improved Bluetooth error handling <BR>
    After a Bluetooth failure, NAPI now continues to attempt restarting indefinitely. Retries are 0.1s apart for the first 5 seconds, 1s apart for the first minute, and 5s apart after that. Two new error messages have been added:<br>
	* "An attempted restart of NAPI has failed, will try again" – issued on the general error channel after a failed restart
	* "NAPI is unable to restart, NEA must exit" – if something happened that excluded restarting<BR><BR>

* NEAs can now choose to explicitly accept or reject a provision pattern through the action (accept, reject) argument to the `napi::ProvisionPattern` call. For details, see [napi::ProvisionPattern](https://downloads.nymi.com/sdkDoc/latest/jsonreference/index.html) in the NAPI JSON Reference.

* The local signing function has been updated to require a setup call to NAPI before the signing function can be used. For details, see [napi::SignSetup](https://downloads.nymi.com/sdkDoc/latest/jsonreference/index.html) in the NAPI JSON Reference.


## Fixes

* A number of existing and/or potential memory leaks have been fixed.

* NAPI internals now uses a secure JSON memory allocator.

* `napi::terminate` is considered safe to use, and will allow you to restart NAPI without restarting the entire NEA.

## Known Issues

If there are operations in progress when terminate() is called, there may be a delay of up to 30 seconds before it completes.

Important: NEAs on Windows OS **must** call napi::terminate() before exiting. If terminate() is not called before the NEA exits, the dynamic libraries will crash on unload. 
