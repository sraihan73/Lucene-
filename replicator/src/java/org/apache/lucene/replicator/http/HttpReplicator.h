#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/replicator/SessionToken.h"

#include  "core/src/java/org/apache/lucene/replicator/Revision.h"

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

using org::apache::http::conn::HttpClientConnectionManager;
using Replicator = org::apache::lucene::replicator::Replicator;
using Revision = org::apache::lucene::replicator::Revision;
using SessionToken = org::apache::lucene::replicator::SessionToken;

/**
 * An HTTP implementation of {@link Replicator}. Assumes the API supported by
 * {@link ReplicationService}.
 *
 * @lucene.experimental
 */
class HttpReplicator : public HttpClientBase, public Replicator
{
  GET_CLASS_NAME(HttpReplicator)

  /** Construct with specified connection manager. */
public:
  HttpReplicator(const std::wstring &host, int port, const std::wstring &path,
                 std::shared_ptr<HttpClientConnectionManager> conMgr);

  std::shared_ptr<SessionToken>
  checkForUpdate(const std::wstring &currVersion)  override;

private:
  class CallableAnonymousInnerClass
      : public Callable<std::shared_ptr<SessionToken>>
  {
    GET_CLASS_NAME(CallableAnonymousInnerClass)
  private:
    std::shared_ptr<HttpReplicator> outerInstance;

    std::shared_ptr<HttpResponse> response;

  public:
    CallableAnonymousInnerClass(std::shared_ptr<HttpReplicator> outerInstance,
                                std::shared_ptr<HttpResponse> response);

    std::shared_ptr<SessionToken> call()  override;

  protected:
    std::shared_ptr<CallableAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CallableAnonymousInnerClass>(
          java.util.concurrent.Callable<
              org.apache.lucene.replicator.SessionToken>::shared_from_this());
    }
  };

public:
  std::shared_ptr<InputStream>
  obtainFile(const std::wstring &sessionID, const std::wstring &source,
             const std::wstring &fileName)  override;

private:
  class CallableAnonymousInnerClass2
      : public Callable<std::shared_ptr<InputStream>>
  {
    GET_CLASS_NAME(CallableAnonymousInnerClass2)
  private:
    std::shared_ptr<HttpReplicator> outerInstance;

    std::shared_ptr<HttpResponse> response;

  public:
    CallableAnonymousInnerClass2(std::shared_ptr<HttpReplicator> outerInstance,
                                 std::shared_ptr<HttpResponse> response);

    std::shared_ptr<InputStream> call()  override;

  protected:
    std::shared_ptr<CallableAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<CallableAnonymousInnerClass2>(
          java.util.concurrent
              .Callable<java.io.InputStream>::shared_from_this());
    }
  };

public:
  void publish(std::shared_ptr<Revision> revision)  override;

  void release(const std::wstring &sessionID)  override;

private:
  class CallableAnonymousInnerClass3 : public Callable<std::any>
  {
    GET_CLASS_NAME(CallableAnonymousInnerClass3)
  private:
    std::shared_ptr<HttpReplicator> outerInstance;

  public:
    CallableAnonymousInnerClass3(std::shared_ptr<HttpReplicator> outerInstance);

    std::any call()  override;

  protected:
    std::shared_ptr<CallableAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<CallableAnonymousInnerClass3>(
          java.util.concurrent.Callable<Object>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<HttpReplicator> shared_from_this()
  {
    return std::static_pointer_cast<HttpReplicator>(
        HttpClientBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/replicator/http/
