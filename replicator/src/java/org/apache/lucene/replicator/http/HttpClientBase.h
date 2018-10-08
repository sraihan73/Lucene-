#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class AlreadyClosedException;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::replicator::http
{

using org::apache::http::HttpEntity;
using org::apache::http::HttpResponse;
using org::apache::http::StatusLine;
using org::apache::http::client::config::RequestConfig;
using org::apache::http::conn::HttpClientConnectionManager;
using org::apache::http::impl::client::CloseableHttpClient;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;

/**
 * Base class for Http clients.
 *
 * @lucene.experimental
 * */
class HttpClientBase : public std::enable_shared_from_this<HttpClientBase>
{
  GET_CLASS_NAME(HttpClientBase)

  /** Default connection timeout for this client, in milliseconds. */
public:
  static constexpr int DEFAULT_CONNECTION_TIMEOUT = 1000;

  /** Default socket timeout for this client, in milliseconds. */
  static constexpr int DEFAULT_SO_TIMEOUT = 60000;

  // TODO compression?

  /** The URL stting to execute requests against. */
protected:
  const std::wstring url;

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool closed = false;
  bool closed = false;

  const std::shared_ptr<CloseableHttpClient> httpc;
  const std::shared_ptr<RequestConfig> defaultConfig;

  /**
   * @param conMgr
   *          connection manager to use for this http client. <b>NOTE:</b>The
   *          provided {@link HttpClientConnectionManager} will not be
   *          {@link HttpClientConnectionManager#shutdown()} by this class.
   * @param defaultConfig
   *          the default {@link RequestConfig} to set on the client. If
   *          {@code null} a default config is created w/ the default connection
   *          and socket timeouts.
   */
protected:
  HttpClientBase(const std::wstring &host, int port, const std::wstring &path,
                 std::shared_ptr<HttpClientConnectionManager> conMgr,
                 std::shared_ptr<RequestConfig> defaultConfig);

  /** Throws {@link AlreadyClosedException} if this client is already closed. */
  void ensureOpen() ;

  /**
   * Create a URL out of the given parameters, translate an empty/null path to
   * '/'
   */
private:
  static std::wstring normalizedURL(const std::wstring &host, int port,
                                    const std::wstring &path);

  /**
   * <b>Internal:</b> response status after invocation, and in case or error
   * attempt to read the exception sent by the server.
   */
protected:
  virtual void
  verifyStatus(std::shared_ptr<HttpResponse> response) ;

  virtual void
  throwKnownError(std::shared_ptr<HttpResponse> response,
                  std::shared_ptr<StatusLine> statusLine) ;

  /**
   * <b>internal:</b> execute a request and return its result
   * The <code>params</code> argument is treated as:
   * name1,value1,name2,value2,...
   */
  virtual std::shared_ptr<HttpResponse>
  executePOST(const std::wstring &request, std::shared_ptr<HttpEntity> entity,
              std::deque<std::wstring> &params) ;

  /**
   * <b>internal:</b> execute a request and return its result
   * The <code>params</code> argument is treated as:
   * name1,value1,name2,value2,...
   */
  virtual std::shared_ptr<HttpResponse>
  executeGET(const std::wstring &request,
             std::deque<std::wstring> &params) ;

private:
  std::wstring queryString(
      const std::wstring &request,
      std::deque<std::wstring> &params) ;

  /** Internal utility: input stream of the provided response */
public:
  virtual std::shared_ptr<InputStream> responseInputStream(
      std::shared_ptr<HttpResponse> response) ;

  // TODO: can we simplify this Consuming !?!?!?
  /**
   * Internal utility: input stream of the provided response, which optionally
   * consumes the response's resources when the input stream is exhausted.
   */
  virtual std::shared_ptr<InputStream>
  responseInputStream(std::shared_ptr<HttpResponse> response,
                      bool consume) ;

private:
  class InputStreamAnonymousInnerClass : public InputStream
  {
    GET_CLASS_NAME(InputStreamAnonymousInnerClass)
  private:
    std::shared_ptr<HttpClientBase> outerInstance;

    std::shared_ptr<HttpEntity> entity;
    std::shared_ptr<InputStream> in_;

  public:
    InputStreamAnonymousInnerClass(
        std::shared_ptr<HttpClientBase> outerInstance,
        std::shared_ptr<HttpEntity> entity, std::shared_ptr<InputStream> in_);

  private:
    bool consumed = false;

  public:
    int read()  override;
    virtual ~InputStreamAnonymousInnerClass();
    int read(std::deque<char> &b)  override;
    int read(std::deque<char> &b, int off,
             int len)  override;

  private:
    void consume(int minusOne);

  protected:
    std::shared_ptr<InputStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<InputStreamAnonymousInnerClass>(
          java.io.InputStream::shared_from_this());
    }
  };

  /**
   * Returns true iff this instance was {@link #close() closed}, otherwise
   * returns false. Note that if you override {@link #close()}, you must call
   * {@code super.close()}, in order for this instance to be properly closed.
   */
protected:
  bool isClosed();

  /**
   * Same as {@link #doAction(HttpResponse, bool, Callable)} but always do
   * consume at the end.
   */
  template <typename T>
  T doAction(std::shared_ptr<HttpResponse> response,
             std::shared_ptr<Callable<T>> call) ;

  /**
   * Do a specific action and validate after the action that the status is still
   * OK, and if not, attempt to extract the actual server side exception.
   * Optionally release the response at exit, depending on <code>consume</code>
   * parameter.
   */
  template <typename T>
  T doAction(std::shared_ptr<HttpResponse> response, bool consume,
             std::shared_ptr<Callable<T>> call) ;

public:
  virtual ~HttpClientBase();
};

} // namespace org::apache::lucene::replicator::http
