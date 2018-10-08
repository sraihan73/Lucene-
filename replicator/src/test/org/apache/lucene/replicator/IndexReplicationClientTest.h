#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::store
{
class MockDirectoryWrapper;
}
namespace org::apache::lucene::replicator
{
class Replicator;
}
namespace org::apache::lucene::replicator
{
class ReplicationClient;
}
namespace org::apache::lucene::replicator
{
class SourceDirectoryFactory;
}
namespace org::apache::lucene::replicator
{
class ReplicationHandler;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::replicator
{
class Revision;
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
namespace org::apache::lucene::replicator
{

using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using ReplicationHandler =
    org::apache::lucene::replicator::ReplicationClient::ReplicationHandler;
using SourceDirectoryFactory =
    org::apache::lucene::replicator::ReplicationClient::SourceDirectoryFactory;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;

class IndexReplicationClientTest : public ReplicatorTestCase
{
  GET_CLASS_NAME(IndexReplicationClientTest)

private:
  class IndexReadyCallback
      : public std::enable_shared_from_this<IndexReadyCallback>,
        public Callable<bool>
  {
    GET_CLASS_NAME(IndexReadyCallback)

  private:
    const std::shared_ptr<Directory> indexDir;
    std::shared_ptr<DirectoryReader> reader;
    int64_t lastGeneration = -1;

  public:
    IndexReadyCallback(std::shared_ptr<Directory> indexDir) ;

    std::optional<bool> call()  override;

    virtual ~IndexReadyCallback();
  };

private:
  std::shared_ptr<MockDirectoryWrapper> publishDir, handlerDir;
  std::shared_ptr<Replicator> replicator;
  std::shared_ptr<SourceDirectoryFactory> sourceDirFactory;
  std::shared_ptr<ReplicationClient> client;
  std::shared_ptr<ReplicationHandler> handler;
  std::shared_ptr<IndexWriter> publishWriter;
  std::shared_ptr<IndexReadyCallback> callback;

  static const std::wstring VERSION_ID;

  void assertHandlerRevision(int expectedID,
                             std::shared_ptr<Directory> dir) ;

  std::shared_ptr<Revision> createRevision(int const id) ;

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<IndexReplicationClientTest> outerInstance;

    int id = 0;

  public:
    HashMapAnonymousInnerClass(
        std::shared_ptr<IndexReplicationClientTest> outerInstance, int id);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Before public void setUp() throws Exception
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After @Override public void tearDown() throws Exception
  void tearDown()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNoUpdateThread() throws Exception
  virtual void testNoUpdateThread() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testUpdateThread() throws Exception
  virtual void testUpdateThread() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRestart() throws Exception
  virtual void testRestart() ;

  /*
   * This test verifies that the client and handler do not end up in a corrupt
   * index if exceptions are thrown at any point during replication. Either when
   * a client copies files from the server to the temporary space, or when the
   * handler copies them to the index directory.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testConsistencyOnExceptions() throws
  // Exception
  virtual void testConsistencyOnExceptions() ;

private:
  class SourceDirectoryFactoryAnonymousInnerClass
      : public std::enable_shared_from_this<
            SourceDirectoryFactoryAnonymousInnerClass>,
        public SourceDirectoryFactory
  {
    GET_CLASS_NAME(SourceDirectoryFactoryAnonymousInnerClass)
  private:
    std::shared_ptr<IndexReplicationClientTest> outerInstance;

    std::shared_ptr<SourceDirectoryFactory> in_;
    std::shared_ptr<AtomicInteger> failures;

  public:
    SourceDirectoryFactoryAnonymousInnerClass(
        std::shared_ptr<IndexReplicationClientTest> outerInstance,
        std::shared_ptr<SourceDirectoryFactory> in_,
        std::shared_ptr<AtomicInteger> failures);

  private:
    int64_t clientMaxSize = 0;
    double clientExRate = 0;

  public:
    void
    cleanupSession(const std::wstring &sessionID)  override;

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override public
    // org.apache.lucene.store.Directory getDirectory(std::wstring sessionID, std::wstring
    // source) throws java.io.IOException
    std::shared_ptr<Directory>
    getDirectory(const std::wstring &sessionID,
                 const std::wstring &source)  override;
  };

private:
  class CallableAnonymousInnerClass : public Callable<bool>
  {
    GET_CLASS_NAME(CallableAnonymousInnerClass)
  private:
    std::shared_ptr<IndexReplicationClientTest> outerInstance;

    std::shared_ptr<AtomicInteger> failures;

  public:
    CallableAnonymousInnerClass(
        std::shared_ptr<IndexReplicationClientTest> outerInstance,
        std::shared_ptr<AtomicInteger> failures);

    std::optional<bool> call()  override;

  protected:
    std::shared_ptr<CallableAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CallableAnonymousInnerClass>(
          java.util.concurrent.Callable<bool>::shared_from_this());
    }
  };

private:
  class ReplicationClientAnonymousInnerClass : public ReplicationClient
  {
    GET_CLASS_NAME(ReplicationClientAnonymousInnerClass)
  private:
    std::shared_ptr<IndexReplicationClientTest> outerInstance;

    std::shared_ptr<AtomicInteger> failures;

  public:
    ReplicationClientAnonymousInnerClass(
        std::shared_ptr<IndexReplicationClientTest> outerInstance,
        std::shared_ptr<org::apache::lucene::replicator::Replicator> replicator,
        std::shared_ptr<ReplicationHandler> handler,
        std::shared_ptr<SourceDirectoryFactory> sourceDirFactory,
        std::shared_ptr<AtomicInteger> failures);

  protected:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override protected
    // void handleUpdateException(Throwable t)
    void handleUpdateException(std::runtime_error t) override;

  protected:
    std::shared_ptr<ReplicationClientAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ReplicationClientAnonymousInnerClass>(
          ReplicationClient::shared_from_this());
    }
  };

protected:
  std::shared_ptr<IndexReplicationClientTest> shared_from_this()
  {
    return std::static_pointer_cast<IndexReplicationClientTest>(
        ReplicatorTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::replicator
