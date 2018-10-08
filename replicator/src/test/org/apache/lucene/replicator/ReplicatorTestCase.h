#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::replicator
{

using org::apache::http::conn::HttpClientConnectionManager;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::eclipse::jetty::server::Handler;
using org::eclipse::jetty::server::Server;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @ThreadLeakLingering(linger = 80000) public abstract class
// ReplicatorTestCase extends org.apache.lucene.util.LuceneTestCase
class ReplicatorTestCase : public LuceneTestCase
{

private:
  static std::shared_ptr<HttpClientConnectionManager> clientConnectionManager;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void
  // afterClassReplicatorTestCase() throws Exception
  static void afterClassReplicatorTestCase() ;

  /**
   * Returns a new {@link Server HTTP Server} instance. To obtain its port, use
   * {@link #serverPort(Server)}.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  static std::shared_ptr<Server>
  newHttpServer(std::shared_ptr<Handler> handler) ;

  /** Returns a {@link Server}'s port. */
  static int serverPort(std::shared_ptr<Server> server);

  /** Returns a {@link Server}'s host. */
  static std::wstring serverHost(std::shared_ptr<Server> server);

  /**
   * Stops the given HTTP Server instance. This method does its best to
   * guarantee that no threads will be left running following this method.
   */
  static void
  stopHttpServer(std::shared_ptr<Server> httpServer) ;

  /**
   * Returns a {@link HttpClientConnectionManager}.
   * <p>
   * <b>NOTE:</b> do not {@link HttpClientConnectionManager#shutdown()} this
   * connection manager, it will be close automatically after all tests have
   * finished.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  static std::shared_ptr<HttpClientConnectionManager>
  getClientConnectionManager();

protected:
  std::shared_ptr<ReplicatorTestCase> shared_from_this()
  {
    return std::static_pointer_cast<ReplicatorTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::replicator
