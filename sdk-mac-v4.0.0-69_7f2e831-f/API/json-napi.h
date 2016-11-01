/*! \mainpage Nymi API (NAPI) C++ Reference
 *
 * \section intro_sec Introduction
 *
 * ## Async Operation
 *
 * NAPI is an asynchronous API that operates by sending stringified JSON objects to NAPI and receiving stringified JSON objects from NAPI.
 *
 * Making a request to NAPI (nymi::jsonNapiPut) is a non-blocking call returning immediately, and it always succeeds if NAPI has
 * been started (nymi::JsonPutOutcome). This success even after nymi::jsonNapiTerminate has been called is an historical detail
 * that will be addressed in a future release.
 *
 * Responses to requests are obtained by calling nymi::jsonNapiGet. Both responses to NEA requests and NAPI-generated
 * notifications are returned from this function. NAPI will include the `exchange` attribute when defined in the
 * request in all responses to the request. This is intended to allow the NEA to match responses to requests.
 *
 * Calls to the nymi::jsonNapiGet are blocking -- they will wait for a result unless interrupted. There are three versions of nymi::jsonNapiGet included
 * in NAPI -- the standard call, and two deprecated variants that should not be used. These provide different mechanisms to return early (without
 * a JSON object). The standard call returns early when nymi::jsonNapiTerminate (or nymi::jsonNapiFinish) has been called. The variants provide
 * mechanisms the NEA can control to return early (timeout, and signaling).
 *
 * \note
 * -# nymi::jsonNapiGet succeeds even after nymi::jsonNapiFinish or nymi::jsonNapiTerminate has been called.
 * -# nymi::jsonNapiGet returns a mix of responses and NAPI generated notifications -- use the `exchange` attribute to identify responses to requests.
 * -# There are two deprecated variants of nymi::jsonNapiGet that should not be used
 *
 *
 * ## Basic NEA approaches to NAPI
 *
 * NAPI is an asynchronous API that works by an interchange of JSON strings between the NEA and NAPI. The details of the JSON are discussed <a href="../../json/html/index.html">here</a>.
 *
 * There are three common approaches to writing an NEA using NAPI:
 *
 * -# Intersperse calls to nymi::jsonNapiPut and nymi::jsonNapiGet.
 * -# Run nymi::jsonNapiPut and nymi::jsonNapiGet in their own threads, and link the response to the request using the `exchange`.
 * -# Run nymi::jsonNapiPut and nymi::jsonNapiGet in their own threads, and dispatch the responses by the `exchange`.
 *
 * The first approach is most useful in simple or short running NEAs. The second, and especially third, approaches are more useful in longer running or complex NEAs.
 *
 * ## Coordinating nymi::jsonNapiGet and nymi::jsonNapiConfigure
 *
 * It is not possible for nymi::jsonNapiGet to succeed before nymi::jsonNapiConfigure has completed successfully. During the startup phase nymi::jsonNapiGet will return
 * JsonGetOutcome::napiNotRunning. If the NEA is trying to call both simultaneously, or nearly, then if the NEA receives a JsonGetOutcome::napiNotRunning it is reasonable to
 * wait for a few milliseconds and try again. If the NEA is trying to call nymi::jsonNapiGet well before nymi::jsonNapiConfigure you can use the same strategy, or choose some
 * other kind of coordination mechanism. Either way no JSON messages are dropped.
 *
 * \note
 * -# The time between the first successful call to nymi::jsonNapiGet and nymi::jsonNapiConfigure does not matter, no messages are dropped.
 * -# nymi::jsonNapiConfigure should only be called once.
 * -# Restarting NAPI is not supported and does not work reliably. Calling napi::jsonNapiFinish() followed by a second call to
 *    nymi::jsonNapiConfigure function may appear to work, but it will eventually break in unpredictable ways.
 *    A future release will support this or provide other means to achieve the same effect.
 * -# Errors that occur during NAPI startup may be reported as a general error notification. These are obtained by calling nymi::jsonNapiGet.
 *
 * ## Logs
 *
 * LogLevel::normal reports only warnings, errors, and fatal log messages. There is no sensitive information in these messages.
 * **However** if the LogLevel::info, LogLevel::debug, LogLevel::verbose levels are used then sensitive information almost certainly will be written
 * to the logs. This may be an acceptable risk when developing a NEA, it is **never** an acceptable risk when the NEA is deployed.
 *
 * The log files are crated in the config directory specified in nymi::jsonNapiConfigure
 *
 * \warning
 * - Logs are written to the filesystem in **clear text**.
 * - All log levels, other than `LogLevel::normal`, write sensitive material, such as symmetric keys, to the log in **clear text**.
 * - Only `LogLevel::normal` should be used in a deployed NEA.
 * - Log files can become **very large** -- if used outside of a development and debugging context they **will** fill the disk.
 *
 */
 
#pragma once
#ifndef JSON_NAPI
#define JSON_NAPI

//#define API __attribute__((visibility("default")))
//#define API

#ifdef __cplusplus

#include <atomic>
#include <string>
#include <sstream>
//#include <src/api/command.h>
//#include <src/api/enums.h>

typedef long long int milliseconds;

template< typename DataType >
  std::wostream& add_the_w( std::wostream& out, const DataType data ){
    std::stringstream subout;
    subout << data;
    return out << subout.str().c_str();
  }

 

 /*! \namespace nymi
 * \brief NAPI C++ Reference
 * 
 * For an overview of the basic operations and recommended approaches for working with NAPI, see the <a href="index.html">Main Page</a>.
 * 
 */
namespace nymi{

  /**
   * \brief The logging level to use for this run of the NEA.
   *
   * Use when calling `nymi::jsonNapiConfigure`.
   *
   * \warning
   * - Logs are written to the filesystem in **clear text**.
   * - All log levels, other than `LogLevel::normal`, write sensitive material, such as symmetric keys, to the log in **clear text**.
   * - Log files can become **very large**. Log levels other than `LogLevel::normal` are intended for short duration use when investigating specific issues.
   *
   */
  enum class LogLevel{
      normal, //!< Limited logging of important events like errors and warnings. The default log level.
      info, //!< Log significantly more information about the internals of NAPI.
      debug, //!< The log level that will likely be used when working with Nymi Support.
      verbose //!< Log pretty much everything down to the Bluetooth level.
  };
  ///@private
  std::ostream& operator<<( std::ostream& out, const LogLevel v );

  ///@private
  inline std::wostream& operator<<( std::wostream& out, const LogLevel data ){ return add_the_w< LogLevel >( out, data ); }

  /**
   * \brief Represents the possible outcomes of a call to `nymi::jsonNapiConfigure`.
   *
   * If the outcome is anything other than `ConfigOutcome::okay` there may be
   * a general-error notification sent by NAPI with additional information
   * -- See the <a href="#http://downloads.nymi.com/sdkDoc/latest/jsonreference/index.html">NAPI JSON Reference</a>.
   */
  enum class ConfigOutcome{
      okay, //!< Configured successfully.
      failedToInit, //!< Configuration infomation is okay, but unable to start successfully.
      configurationFileNotFound, //!< The provided configuration file could not be found.
      configurationFileNotReadable, //!< The provided configuration file could not be read.
      configurationFileNotParsed //!< The provided configuration file was not valid JSON.
  };
  ///@private
  std::ostream& operator<<( std::ostream& out, const ConfigOutcome v );

  ///@private
  inline std::wostream& operator<<( std::wostream& out, const ConfigOutcome data ){ return add_the_w< ConfigOutcome >( out, data ); }

  /**
   * \brief Represents the outcome of a call to one of the three variants of `nymi::jsonNapiGet`.
   *
   */
  enum class JsonGetOutcome{
      okay, //!< A JSON string has been returned in the `&json` parameter.
      napiNotRunning, //!< NAPI is not running -- either nymi::jsonNapiConfigure did not complete or nymi::jsonNapiTerminate was already called.
      timedout, //!< The second variant of nymi::jsonNapiGet timed out.
      quitSignaled, //!< The third variant of nymi::jsonNapiGet was called and `quit` was signaled.
      napiFinished //!< nymi::jsonNapiTerminate was called.
  };
  ///@private
  std::ostream& operator<<( std::ostream& out, const JsonGetOutcome v ); ///@private
  ///@private
  inline std::wostream& operator<<( std::wostream& out, const JsonGetOutcome data ){ return add_the_w< JsonGetOutcome >( out, data ); }

  /**
   * \brief Reprents the outcome of a call to `nymi::jsonNapiPut`.
   *
   */
  enum class JsonPutOutcome{
      okay, //!< Successful
      napiNotRunning //!< NAPI is not running
  };
  ///@private
  std::ostream& operator<<( std::ostream& out, const JsonPutOutcome v );

  ///@private
  inline std::wostream& operator<<( std::wostream& out, const JsonPutOutcome data ){ return add_the_w< JsonPutOutcome >( out, data ); }

  /**
   * \brief Configure and start NAPI
   *
   * \param[in] rootDirectory Path to a directory that contains the `config.json` file and to which NAPI writes `provisions.json` and any log files.
   * \param[in] logLevel The log level to use (see nymi::LogLevel).
   * \param[in] port The default port for networked Nymi Bands (on Windows) or the Nymulator.
   * \param[in] host The default host for networked Nymi Bands (on Windows) or the Nymulator.
   *
   * For most NEAs the default arguments are correct so the call would be similar to `nymi::jsonNapiConfigure("root-directory-path");`.
   * The default host of `""` is treated as `"127.0.0.1"`. The default port of -1 will choose the port depending on platform (OS X or Windows) and libary (native or networked).
   *
   * The **rootDirectory** is a directory that must contain a file called `config.json`. When the NEA runs, it saves provision information into a file called `provisions.json`.
   * The NEA will also create log files in that directory.
   *
   * \par The config.json file
   *
   * The `config.json` file is a configuration file that NAPI expects to find in `rootDirectory`.
   *
   * The configuration file defines two fields:
   * - `neaName` is the name of this NEA. This must be between 6 and 18 characters in length. It appears in the Nymi Band app and can be seen by the users provisioned to this NEA. It should not be changed once the value is established.
   * - `sigAlgorithm` is either `SECP256K` the default, or `NIST256P`. This also should not be changed after the value is established.
   *
   * @code{.json}
   *     {
   *       "neaName" : "sampleNEA",
   *       "sigAlgorithm" : "SECP256K"
   *     }
   * @endcode
   *
   *
   * \note
   * -# This should only be called once
   * -# Restarting NAPI is not supported and does not work reliably. Calling napi::jsonNapiFinish() followed by a second call
   *    this function may appear to work, but it will eventually break in unpredictable ways.
   *    A future release will support this or provide other means to achieve the same effect.
   */
  ConfigOutcome jsonNapiConfigure( std::string rootDirectory, LogLevel logLevel = LogLevel::normal, int port = -1, std::string host = "" );

  /**
   * \brief Send a JSON message to NAPI
   */
  JsonPutOutcome jsonNapiPut( std::string json_in );

  /**
   * \brief Receive a JSON message from NAPI; standard usage.
   *
   * \param[out] json stringified JSON object to send to NAPI
   *
   * nymi::jsonNapiGet is a blocking call. This variant returns when:
   * - A message is available from NAPI (JsonGetOutcome::okay)
   * - NAPI is not running (JsonGetOutcome::napiNotRunning)
   * - NAPI has finished (JsonGetOutcome::napiFinished)
   *
   * If NAPI is not running, wait a short time and call nymi::jsonNapiGet again. No JSON messages are lost. See discussion of nymi::jsonNapiConfigure.
   *
   * \note
   * This is the variant that almost every NEA will use, the other variants are for rare cases.
   */
  JsonGetOutcome jsonNapiGet( std::string& json );

  /**
   * \brief Receive a JSON message from NAPI; variant one.
   *
   * \param[in] json Stringified JSON object to send to NAPI.
   * \param[in] timeout The timeout in milliseconds.
   * \param[in] sleep The timeout is implemented by waiting for `sleep` milliseconds until the timeout is exceeded.
   *
   * \deprecated
   * This variant will be removed in a future release of NAPI.
   *
   * nymi::jsonNapiGet is a blocking call. This variant returns when:
   * - A message is available from NAPI (JsonGetOutcome::okay)
   * - NAPI is not running (JsonGetOutcome::napiNotRunning)
   * - NAPI has finished (JsonGetOutcome::napiFinished)
   * - A timeout has expired without a message or finish (JsonGetOutcome::timedout)
   *
   * \attention
   * - There are almost always better ways of doing this. Use one of these alternative techniques with the standard usage of nymi::jsonNapiGet.
   * - This variant will almost always make your NEA harder to work with; it is for very special cases. \internal [@bob - such as what?] \endinternal
   *
   * If NAPI is not running, wait a short time and call nymi::jsonNapiGet again. No JSON messages are lost. See discussion of nymi::jsonNapiConfigure.
   */
  JsonGetOutcome jsonNapiGet( std::string& json, milliseconds timeout, milliseconds sleep = 100 );

  /**
   * \brief Receive a JSON message from NAPI; variant two.
   *
   *
   * \param[in] json Stringified JSON object to send to NAPI
   * \param[in] quit An `atomic<bool>` that causes this function to return when it is set to `true`
   * \param[in] sleep Quit is checked every `sleep` milliseconds
   *
   * \deprecated
   * This variant will be removed in a future release of NAPI.
   *
   * nymi::jsonNapiGet is a blocking call. This variant returns when:
   * - A message is available from NAPI (JsonGetOutcome::okay)
   * - NAPI is not running (JsonGetOutcome::napiNotRunning)
   * - NAPI has finished (JsonGetOutcome::napiFinished)
   * - Quit has been signaled (JsonGetOutcome::quitSignaled)
   *
   * \attention
   * - There are almost always better ways of doing this. Use one of these alternative techniques with the standard usage of nymi::jsonNapiGet.
   * - This variant will almost always make your NEA harder to work with; it is for very special cases. \internal [@bob - such as what?] \endinternal
   *
   * If NAPI is not running, wait a short time and call nymi::jsonNapiGet again. No JSON messages are lost. See discussion of nymi::jsonNapiConfigure.
   */
  JsonGetOutcome jsonNapiGet( std::string& json, std::atomic< bool >& quit, milliseconds sleep = 100 );

  /**
   * \brief Shutdown NAPI, a alias for nymi::jsonNapiTerminate.
   *
   * \deprecated
   * This function is here for historical reasons.
   */
  void jsonNapiFinish();

  /**
   * \brief Shutdown NAPI.
   *
   * The NEA should call this function before exiting.
   *
   * \note
   * Calling this function, followed by a second call to napi::jsonNapiConfig, may appear to work, but it will eventually break in unpredictable ways.
   * A future release will support this or provide other means to achieve the same effect.
   *
   * \attention
   * If this function is **not** called before the NEA exits in a way that calls C++ destructors, there is a possibility of a crash.
   * If you don't want to call this function -- it does take a few 10s of milliseconds -- use an exit that does not call the C++ destructors
   * unless you can accommodate the chance of a crash.
   */
  void jsonNapiTerminate();
}

extern "C" {

#endif // __cplusplus
#ifndef cplusspluss_only

typedef enum{
    NAPI_DL_NORMAL,
    NAPI_DL_INFO,
    NAPI_DL_DEBUG,
    NAPI_DL_VEBOSE
}                     logLevel;

typedef enum{
    NAPI_CO_OKAY,
    NAPI_CO_FAILED_TO_INIT,
    NAPI_CO_CONFIGURATION_FILE_NOT_FOUND,
    NAPI_CO_CONFIGURATION_FILE_NOT_READABLE,
    NAPI_CO_CONFIGURATION_FILE_NOT_PARSED,
    NAPI_CO_IMPOSSIBLE
}                     configOutcome;
typedef enum{
    NAPI_PO_OKAY,
    NAPI_PO_NAPI_NOT_RUNNING,
    NAPI_PO_IMPOSSIBLE
}                     jsonPutOutcome;
typedef enum{
    NAPI_GO_OKAY,
    NAPI_GO_NAPI_NOT_RUNNING,
    NAPI_GO_TIMED_OUT,
    NAPI_GO_QUIT_SIGNALED,
    NAPI_GO_NAPI_FINISHED,
    NAPI_GO_IMPOSSIBLE
}                     jsonGetOutcome;

extern configOutcome jsonNapiConfigure( const char* rootDirectory, logLevel logLevel, int port, const char* host );
extern jsonPutOutcome jsonNapiPut( const char* json_in );
extern jsonGetOutcome jsonNapiGet( char* json_out, unsigned long long max );
extern jsonGetOutcome jsonNapiGetTimeout( char* json_out, unsigned long long max, long long int timeout, long long int sleep );
extern void jsonNapiFinish();
extern void jsonNapiTerminate();

#endif // cplusspluss_only
#ifdef __cplusplus
}

#endif // __cplusplus
#endif // JSON_NAPI
