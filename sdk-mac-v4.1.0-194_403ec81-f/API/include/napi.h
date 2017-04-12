/*! \mainpage Nymi API (NAPI) Reference
 *
 * \section intro_sec Introduction
 *
 * ## Async Operation
 *
 * NAPI is an asynchronous API that operates by sending stringified JSON objects to NAPI and receiving stringified JSON objects from NAPI.
 *
 * Making a request to NAPI (napi::put) is a non-blocking call returning immediately, and it always succeeds if NAPI has
 * been started (napi::PutOutcome). This success even after napi::terminate has been called is an historical detail
 * that will be addressed in a future release.
 *
 * Responses to requests are obtained by calling napi::get. Both responses to NEA requests and NAPI-generated
 * notifications are returned from this function. NAPI will include the `exchange` attribute when defined in the
 * request in all responses to the request. This is intended to allow the NEA to match responses to requests.
 *
 * Calls to napi::get are blocking -- they will wait for a result unless interrupted. Calls to napi::try_get return immediately.
 *
 * \note
 * -# napi::get succeeds even after napi::terminate has been called.
 * -# napi::get returns a mix of responses and NAPI generated notifications -- use the `exchange` attribute to identify responses to requests.
 *
 *
 * ## Basic NEA approaches to NAPI
 *
 * NAPI is an asynchronous API that works by an interchange of JSON strings between the NEA and NAPI. The details of the JSON are discussed <a href="../../json/html/index.html">here</a>.
 *
 * There are three common approaches to writing an NEA using NAPI:
 *
 * -# Intersperse calls to napi::put and napi::get.
 * -# Run napi::put and napi::get in their own threads, and link the response to the request using the `exchange`.
 * -# Run napi::put and napi::get in their own threads, and dispatch the responses by the `exchange`.
 *
 * The first approach is most useful in simple or short running NEAs. The second, and especially third, approaches are more useful in longer running or complex NEAs.
 *
 * ## Coordinating napi::get and napi::configure
 *
 * It is not possible for napi::get to succeed before napi::configure has completed successfully. During the startup phase napi::get will return
 * napi::GetOutcome::notRunning. If the NEA is trying to call both simultaneously, or nearly, then if the NEA receives a napi::GetOutcome::notRunning it is reasonable to
 * wait for a few milliseconds and try again. If the NEA is trying to call napi::get well before napi::configure you can use the same strategy, or choose some
 * other kind of coordination mechanism. Either way no JSON messages are dropped.
 *
 * \note
 * -# The time between the first successful call to napi::get and napi::configure does not matter, no messages are dropped.
 * -# napi::configure should only be called once.
 * -# Restarting NAPI is achieved by calling napi::terminate() followed by another call to napi::configure.
 * -# Errors that occur during NAPI startup may be reported as a general error notification. These are obtained by calling napi::get.
 *
 * ## Logs
 *
 * LogLevel::normal reports only warnings, errors, and fatal log messages. There is no sensitive information in these messages.
 * **However** if the LogLevel::info, LogLevel::debug, LogLevel::verbose levels are used then sensitive information almost certainly will be written
 * to the logs. This may be an acceptable risk when developing a NEA; it is **never** an acceptable risk when the NEA is deployed.
 *
 * The log files are crated in the config directory specified in napi::configure
 *
 * \warning
 * - Logs are written to the filesystem in **clear text**.
 * - All log levels, other than `LogLevel::normal`, write sensitive material, such as symmetric keys, to the log in **clear text**.
 * - Only `LogLevel::normal` should be used in a deployed NEA.
 * - Log files can become **very large** -- if used outside of a development and debugging context they **will** fill the disk.
 *
 */

#pragma once
#ifndef JSON_NAPI_X
#define JSON_NAPI_X

#if defined(_MSC_VER)
#define API __declspec(dllexport)
#else
#define API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus

#include <atomic>
#include <string>
#include <sstream>

typedef long long int milliseconds;


/*! \namespace napi
* \brief NAPI C++ Reference
*
* For an overview of the basic operations and recommended approaches for working with NAPI, see the <a href="index.html">Main Page</a>.
*
*/
namespace napi{

  template< typename DataType >
    std::wostream& add_the_w( std::wostream& out, const DataType data ){
      std::stringstream subout;
      subout << data;
      return out << subout.str().c_str();
    }

  /**
   * \brief The logging level to use for this run of the NEA.
   *
   * Use when calling `napi::configure`.
   *
   * \warning
   * - Logs are written to the filesystem in **clear text**.
   * - All log levels, other than `LogLevel::normal`, write sensitive material, such as symmetric keys, to the log in **clear text**.
   * - Log files can become **very large**. Log levels other than `LogLevel::normal` are intended for short duration use when investigating specific issues.
   *
   */
  enum class LogLevel{
      none, //!< no logging
      normal, //!< Normal logging of important events like errors and warnings. The default log level.
      info, //!< Logs significantly more information about the internals of NAPI.
      debug, //!< The log level that will likely be used when working with Nymi Support.
      verbose //!< Logs pretty much everything down to the Bluetooth level.
  };
  ///@private
  API std::ostream& operator<<( std::ostream& out, const LogLevel v );

  ///@private
  inline std::wostream& operator<<( std::wostream& out, const LogLevel data ){ return add_the_w< LogLevel >( out, data ); }

  /**
   * \brief Represents the possible outcomes of a call to `napi::configure`.
   *
   * If the outcome is anything other than `ConfigOutcome::okay` there may be
   * a general-error notification sent by NAPI with additional information
   * -- See the <a href="http://downloads.nymi.com/sdkDoc/latest/jsonreference/index.html">NAPI JSON Reference</a>.
   */
  enum class ConfigOutcome{
      okay, //!< Configured successfully.
      failedToInit, //!< Configuration infomation is okay, but NAPI was unable to start successfully.
      invalidProvisionJSON, //!< Provision information provided is invalid (likely invalid JSON).
      missingNEAName, //!< Provision information does not include neaName.
      error, //!< an error occurred, likely an exception, possibly involving the parameters provided
  };
  ///@private
  API std::ostream& operator<<( std::ostream& out, const ConfigOutcome v );

  ///@private
  inline std::wostream& operator<<( std::wostream& out, const ConfigOutcome data ){ return add_the_w< ConfigOutcome >( out, data ); }

  /**
   * \brief Represents the outcome of a call to `napi::get`.
   *
   */
  enum class GetOutcome{
      okay, //!< A JSON string has been returned in the `&json` parameter.
      notRunning, //!< NAPI is not running -- either napi::configure did not complete or napi::terminate was already called.
      bufferTooSmall, //!< The provided char* buffer is not long enough; the length value will contain the minimum required size.
      terminated, //!< napi::terminate was called. This outcome will be returned once. Afterwards, the outcome is napi::notRunning.
      error, //!< an error occurred, likely an exception
  };
  ///@private
  API std::ostream& operator<<( std::ostream& out, const GetOutcome v ); ///@private
  ///@private
  inline std::wostream& operator<<( std::wostream& out, const GetOutcome data ){ return add_the_w< GetOutcome >( out, data ); }

  /**
   * \brief Represents the outcome of a call to `napi::try_get`.
   *
   */
  enum class TryGetOutcome{
      okay, //!< A JSON string has been returned in the `&json` parameter.
      nothing, //!< there is no JSON available at the time of the call.
      notRunning, //!< NAPI is not running -- either napi::configure has not completed or napi::terminate was already called.
      bufferTooSmall, //!< The provided char* buffer is not long enough; the length value will contain the minimum required size.
      terminated, //!< napi::terminate was called. This outcome will be returned once. Afterwards, the outcome is napi::notRunning.
      error, //!< an error occurred, likely an exception
  };
  ///@private
  API std::ostream& operator<<( std::ostream& out, const TryGetOutcome v ); ///@private
  ///@private
  inline std::wostream& operator<<( std::wostream& out, const TryGetOutcome data ){ return add_the_w< TryGetOutcome >( out, data ); }

  /**
   * \brief Represents the outcome of a call to `napi::put`.
   *
   */
  enum class PutOutcome{
      okay, //!< Successful
      notRunning, //!< NAPI is not running
      unparseableJSON, //!< The provided string is not parseable as JSON.
      error, //!< an error occurred, likely an exception
  };
  ///@private
  API std::ostream& operator<<( std::ostream& out, const PutOutcome v );

  ///@private
  inline std::wostream& operator<<( std::wostream& out, const PutOutcome data ){ return add_the_w< PutOutcome >( out, data ); }

  /**
   * \brief A (partially) type checked, non-literal way to specify paths with `napi::put`.
   *
   */
  enum class Path{
      NotificationGet,
      NotificationSet,
      EventOnFoundChangeData,
      EventOnPresenceChangeData,
      EventOnGeneralErrorData,
      EventOnProvisionsChangedData,
      InfoGet,
      RevokeRun,
      RandomRun,
      InitGet,
      Buzz,
      SignSetup,
      SignRun,
      SymmetricKeyRun,
      SymmetricKeyGet,
      TOTPRun,
      TOTPGet,
      CDFRun,
      CDFGet,
      ProvisionManagement,
      ProvisionRunStart,
      ProvisionRunStop,
      ProvisionPattern,
      EventOnLEDPatternChangeData,
      EventOnProvisionedData,
      RoamingAuthSetup,
      RoamingAuthRun,
      EventRANonceData,
      RoamingAuthSig,
      KeyDelete
  };
  ///@private
  API std::ostream& operator<<( std::ostream& out, const Path v );

  ///@private
  inline std::wostream& operator<<( std::wostream& out, const Path data ){ return add_the_w< Path >( out, data ); }

  /**
   * \brief Configure and start NAPI
   *
   * \param[in] neaName Name of this NEA (used when provisioning).
   * \param[in] logDirectory Path to a directory that will contain log files.
   * \param[in] provisions The provision data saved by previous runs of the NEA.
   * \param[in] logLevel The log level to use (see napi::LogLevel).
   * \param[in] port The default port for networked Nymi Bands (on Windows) or the Nymulator.
   * \param[in] host The default host for networked Nymi Bands (on Windows) or the Nymulator.
   *
   * For most NEAs the default arguments are correct, so the call would be similar to `napi::configure("root-directory-path");`.
   * The default host of `""` is treated as `"127.0.0.1"`. The default port of -1 will choose the port depending on platform (OS X or Windows) and libary (native or networked).
   *
   * The value of `provisions` should be the same as the last saved value.
   *
   * \note
   * -# This should only be called once for every run of the NEA.
   * -# Restarting NAPI is achieved by calling napi::terminate() followed by another call to napi::configure.
   */
  API ConfigOutcome configure( const char* neaName,
                               const char* logDirectory,
                               const char* provisions = "{}",
                               LogLevel logLevel = LogLevel::normal,
                               int port = -1,
                               const char* host = "" );
  /**
   * \brief Send a JSON message to NAPI
   */
  API PutOutcome put( const char* json_in );

  /**
   * \brief Send a JSON message to NAPI
   */
  API PutOutcome put( Path path, const char* json_in );

  /**
  * \brief Receive a JSON message from NAPI, blocks if nothing is available yet; standard usage.
  *
  * \param[in, out] buffer a char* buffer, allocated by the NEA, into which the received JSON will be copied.
  * \param[in] max maximum length of buffer
  * \param[out] len the length of the buffer. If the outcome is napi::GetOutcome::bufferTooSmall then it will contain the minimum required buffer size.
  *
  * napi::get is a blocking call. This variant returns when:
  * - A message has been received from NAPI (napi::GetOutcome::okay)
  * - A message from NAPI is ready, but the provided buffer is too small (napi::GetOutcome::bufferTooSmall)
  * - NAPI is not running (napi::GetOutcome::notRunning)
  * - NAPI has terminated (napi::GetOutcome::napiTerminated)
  *
  * If NAPI is not running, wait a short time and call napi::get again. No JSON messages are lost. See discussion of napi::configure.
  *
  * \note
  * This function is blocking, the non-blocking version is napi::try_get.
   */
  API GetOutcome get( char* buffer, unsigned long long max, unsigned long long* len );

  /**
   * \brief Receive a JSON message from NAPI if one is available, non-blocking; standard usage.
   *
   * \param[in, out] buffer a char* buffer, allocated by the NEA, into which the received JSON will be copied.
   * \param[in] max maximum length of buffer
   * \param[out] len the length of the buffer. If the outcome is napi::GetOutcome::bufferTooSmall then it will contain the minimum required buffer size.
   *
   * napi::get is a blocking call. This variant returns when:
   * - A message has been received from NAPI (napi::GetOutcome::okay)
   * - A message from NAPI is ready, but the provided buffer is too small (napi::GetOutcome::bufferTooSmall)
   * - NAPI is not running (napi::GetOutcome::notRunning)
   * - NAPI has terminated (napi::GetOutcome::napiTerminated)
   *
   * If NAPI is not running, wait a short time and call napi::get again. No JSON messages are lost. See discussion of napi::configure.
   *
   * \note
   * This function is non-blocking, the blocking version is napi::get.
   */
  API TryGetOutcome try_get( char* buffer, unsigned long long max, unsigned long long* len );

  /**
   * \brief translate the path field in a JSON object returned by `napi::get` or `napi::try_get` to the Path enum.
   * \param[in] literal the string from the JSON path attribute
   * \param[out] literal translated to a napi::Path enum value (set only if return value is true)
   * \return true if translation could be made
   */
  API bool translateLiteralPath( const char* literal, Path* path );

  /**
   * \brief Shutdown NAPI.
   *
   * The NEA should call this function before exiting.
   *
   * \note
   * Calling this function, followed by a second call to napi::configure, may now work (consider it beta functionality).
   *
   * \attention
   * If this function is **not** called before the NEA exits in a way that calls C++ destructors, there is a possibility of a crash.
   * If you don't want to call this function -- it does take a few 10s of milliseconds -- use an exit that does not call the C++ destructors
   * unless you can accommodate the chance of a crash.
   */
  API void terminate();
}

extern "C" {

#endif // __cplusplus
#ifndef cplusspluss_only_x

typedef enum{
    NAPI_DL_NONE,
    NAPI_DL_NORMAL,
    NAPI_DL_INFO,
    NAPI_DL_DEBUG,
    NAPI_DL_VEBOSE
}                     napiLogLevel;

typedef enum{
    NAPI_CO_OKAY,
    NAPI_CO_INVALID_PROVISION_JSON,
    NAPI_CO_MISSING_NEA_NAME,
    NAPI_CO_FAILED_TO_INIT,
    NAPI_CO_ERROR,
    NAPI_CO_IMPOSSIBLE
}                     napiConfigOutcome;
typedef enum{
    NAPI_PO_OKAY,
    NAPI_PO_NAPI_NOT_RUNNING,
    NAPI_PO_UNPARSEABLE_JSON,
    NAPI_PO_ERROR,
    NAPI_PO_IMPOSSIBLE
}                     napiPutOutcome;
typedef enum{
    NAPI_GO_OKAY,
    NAPI_GO_NAPI_NOT_RUNNING,
    NAPI_GO_BUFFER_TOO_SMALL,
    NAPI_GO_NAPI_TERMINATED,
    NAPI_GO_ERROR,
    NAPI_GO_IMPOSSIBLE
}                     napiGetOutcome;
typedef enum{
    NAPI_TGO_OKAY,
    NAPI_TGO_NOTHING,
    NAPI_TGO_NAPI_NOT_RUNNING,
    NAPI_TGO_BUFFER_TOO_SMALL,
    NAPI_TGO_NAPI_TERMINATED,
    NAPI_TGO_ERROR,
    NAPI_TGO_IMPOSSIBLE
}                     napiTryGetOutcome;

API extern napiConfigOutcome napiConfigure( const char* neaName,
                                            const char* logDirectory,
                                            const char* provisions,
                                            napiLogLevel logLevel,
                                            int port,
                                            const char* host );
API extern napiPutOutcome napiPut( const char* json_in );
API extern napiGetOutcome napiGet( char* buffer, unsigned long long max, unsigned long long* len );
API extern napiTryGetOutcome napiTryGet( char* buffer, unsigned long long max, unsigned long long* len );
API extern void napiTerminate();

#endif // cplusspluss_only_x
#ifdef __cplusplus
}

#endif // __cplusplus
#endif // JSON_NAPI_X
