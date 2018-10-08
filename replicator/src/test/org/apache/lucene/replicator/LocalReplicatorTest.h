#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/replicator/LocalReplicator.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
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
namespace org::apache::lucene::replicator
{

using IndexWriter = org::apache::lucene::index::IndexWriter;
using Directory = org::apache::lucene::store::Directory;

class LocalReplicatorTest : public ReplicatorTestCase
{
  GET_CLASS_NAME(LocalReplicatorTest)

private:
  static const std::wstring VERSION_ID;

  std::shared_ptr<LocalReplicator> replicator;
  std::shared_ptr<Directory> sourceDir;
  std::shared_ptr<IndexWriter> sourceWriter;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before @Override public void setUp() throws Exception
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After @Override public void tearDown() throws Exception
  void tearDown()  override;

private:
  std::shared_ptr<Revision> createRevision(int const id) ;

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<LocalReplicatorTest> outerInstance;

    int id = 0;

  public:
    HashMapAnonymousInnerClass(
        std::shared_ptr<LocalReplicatorTest> outerInstance, int id);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCheckForUpdateNoRevisions() throws
  // Exception
  virtual void testCheckForUpdateNoRevisions() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testObtainFileAlreadyClosed() throws
  // java.io.IOException
  virtual void testObtainFileAlreadyClosed() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPublishAlreadyClosed() throws
  // java.io.IOException
  virtual void testPublishAlreadyClosed() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testUpdateAlreadyClosed() throws
  // java.io.IOException
  virtual void testUpdateAlreadyClosed() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPublishSameRevision() throws
  // java.io.IOException
  virtual void testPublishSameRevision() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPublishOlderRev() throws
  // java.io.IOException
  virtual void testPublishOlderRev() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testObtainMissingFile() throws
  // java.io.IOException
  virtual void testObtainMissingFile() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSessionExpiration() throws
  // java.io.IOException, InterruptedException
  virtual void testSessionExpiration() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testUpdateToLatest() throws
  // java.io.IOException
  virtual void testUpdateToLatest() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRevisionRelease() throws Exception
  virtual void testRevisionRelease() ;

protected:
  std::shared_ptr<LocalReplicatorTest> shared_from_this()
  {
    return std::static_pointer_cast<LocalReplicatorTest>(
        ReplicatorTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/replicator/
