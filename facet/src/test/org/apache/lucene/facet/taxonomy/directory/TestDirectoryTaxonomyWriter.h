#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/taxonomy/writercache/TaxonomyWriterCache.h"

#include  "core/src/java/org/apache/lucene/facet/taxonomy/FacetLabel.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/directory/DirectoryTaxonomyWriter.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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
namespace org::apache::lucene::facet::taxonomy::directory
{

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using Directory = org::apache::lucene::store::Directory;

class TestDirectoryTaxonomyWriter : public FacetTestCase
{
  GET_CLASS_NAME(TestDirectoryTaxonomyWriter)

  // A No-Op TaxonomyWriterCache which always discards all given categories, and
  // always returns true in put(), to indicate some cache entries were cleared.
private:
  static std::shared_ptr<TaxonomyWriterCache> NO_OP_CACHE;

private:
  class TaxonomyWriterCacheAnonymousInnerClass
      : public std::enable_shared_from_this<
            TaxonomyWriterCacheAnonymousInnerClass>,
        public TaxonomyWriterCache
  {
    GET_CLASS_NAME(TaxonomyWriterCacheAnonymousInnerClass)
  public:
    TaxonomyWriterCacheAnonymousInnerClass();

    virtual ~TaxonomyWriterCacheAnonymousInnerClass();
    int get(std::shared_ptr<FacetLabel> categoryPath) override;
    bool put(std::shared_ptr<FacetLabel> categoryPath, int ordinal) override;
    bool isFull() override;
    void clear() override;
    int size() override;
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCommit() throws Exception
  virtual void testCommit() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCommitUserData() throws Exception
  virtual void testCommitUserData() ;

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestDirectoryTaxonomyWriter> outerInstance;

  public:
    HashMapAnonymousInnerClass(
        std::shared_ptr<TestDirectoryTaxonomyWriter> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRollback() throws Exception
  virtual void testRollback() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRecreateRollback() throws Exception
  virtual void testRecreateRollback() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEnsureOpen() throws Exception
  virtual void testEnsureOpen() ;

private:
  void touchTaxo(std::shared_ptr<DirectoryTaxonomyWriter> taxoWriter,
                 std::shared_ptr<FacetLabel> cp) ;

private:
  class HashMapAnonymousInnerClass2
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass2)
  private:
    std::shared_ptr<TestDirectoryTaxonomyWriter> outerInstance;

  public:
    HashMapAnonymousInnerClass2(
        std::shared_ptr<TestDirectoryTaxonomyWriter> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass2>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRecreateAndRefresh() throws Exception
  virtual void testRecreateAndRefresh() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBackwardsCompatibility() throws
  // Exception
  virtual void testBackwardsCompatibility() ;

  virtual void testConcurrency() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestDirectoryTaxonomyWriter> outerInstance;

    int range = 0;
    std::shared_ptr<AtomicInteger> numCats;
    std::shared_ptr<ConcurrentHashMap<std::wstring, std::wstring>> values;
    std::shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                        DirectoryTaxonomyWriter>
        tw;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestDirectoryTaxonomyWriter> outerInstance, int range,
        std::shared_ptr<AtomicInteger> numCats,
        std::shared_ptr<ConcurrentHashMap<std::wstring, std::wstring>> values,
        std::shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                            DirectoryTaxonomyWriter>
            tw);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

private:
  int64_t getEpoch(std::shared_ptr<Directory> taxoDir) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testReplaceTaxonomy() throws Exception
  virtual void testReplaceTaxonomy() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testReaderFreshness() throws Exception
  virtual void testReaderFreshness() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCommitNoEmptyCommits() throws
  // Exception
  virtual void testCommitNoEmptyCommits() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCloseNoEmptyCommits() throws Exception
  virtual void testCloseNoEmptyCommits() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPrepareCommitNoEmptyCommits() throws
  // Exception
  virtual void testPrepareCommitNoEmptyCommits() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testHugeLabel() throws Exception
  virtual void testHugeLabel() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testReplaceTaxoWithLargeTaxonomy() throws
  // Exception
  virtual void testReplaceTaxoWithLargeTaxonomy() ;

protected:
  std::shared_ptr<TestDirectoryTaxonomyWriter> shared_from_this()
  {
    return std::static_pointer_cast<TestDirectoryTaxonomyWriter>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/directory/
