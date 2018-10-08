#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::document
{
class Field;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class RandomIndexWriter;
}
namespace org::apache::lucene::search
{
class SearcherManager;
}
namespace org::apache::lucene::search
{
template <typename Stypename T>
class LiveFieldValues;
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
namespace org::apache::lucene::codecs::idversion
{

using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using PerThreadPKLookup = org::apache::lucene::index::PerThreadPKLookup;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Basic tests for IDVersionPostingsFormat
 */
// Cannot extend BasePostingsFormatTestCase because this PF is not
// general (it requires payloads, only allows 1 doc per term, etc.)
class TestIDVersionPostingsFormat : public LuceneTestCase
{
  GET_CLASS_NAME(TestIDVersionPostingsFormat)

public:
  virtual void testBasic() ;

private:
  class IDSource
  {
    GET_CLASS_NAME(IDSource)
  public:
    virtual std::wstring next() = 0;
  };

private:
  std::shared_ptr<IDSource> getRandomIDs();

private:
  class IDSourceAnonymousInnerClass
      : public std::enable_shared_from_this<IDSourceAnonymousInnerClass>,
        public IDSource
  {
    GET_CLASS_NAME(IDSourceAnonymousInnerClass)
  private:
    std::shared_ptr<TestIDVersionPostingsFormat> outerInstance;

  public:
    IDSourceAnonymousInnerClass(
        std::shared_ptr<TestIDVersionPostingsFormat> outerInstance);

    std::wstring next() override;
  };

private:
  class IDSourceAnonymousInnerClass2
      : public std::enable_shared_from_this<IDSourceAnonymousInnerClass2>,
        public IDSource
  {
    GET_CLASS_NAME(IDSourceAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIDVersionPostingsFormat> outerInstance;

  public:
    IDSourceAnonymousInnerClass2(
        std::shared_ptr<TestIDVersionPostingsFormat> outerInstance);

    std::wstring next() override;
  };

private:
  class IDSourceAnonymousInnerClass3
      : public std::enable_shared_from_this<IDSourceAnonymousInnerClass3>,
        public IDSource
  {
    GET_CLASS_NAME(IDSourceAnonymousInnerClass3)
  private:
    std::shared_ptr<TestIDVersionPostingsFormat> outerInstance;

  public:
    IDSourceAnonymousInnerClass3(
        std::shared_ptr<TestIDVersionPostingsFormat> outerInstance);

    int upto = 0;
    std::wstring next() override;
  };

private:
  class IDSourceAnonymousInnerClass4
      : public std::enable_shared_from_this<IDSourceAnonymousInnerClass4>,
        public IDSource
  {
    GET_CLASS_NAME(IDSourceAnonymousInnerClass4)
  private:
    std::shared_ptr<TestIDVersionPostingsFormat> outerInstance;

  public:
    IDSourceAnonymousInnerClass4(
        std::shared_ptr<TestIDVersionPostingsFormat> outerInstance);

    const int radix;
    const std::wstring zeroPad;
    int upto = 0;
    std::wstring next() override;
  };

private:
  class IDSourceAnonymousInnerClass5
      : public std::enable_shared_from_this<IDSourceAnonymousInnerClass5>,
        public IDSource
  {
    GET_CLASS_NAME(IDSourceAnonymousInnerClass5)
  private:
    std::shared_ptr<TestIDVersionPostingsFormat> outerInstance;

  public:
    IDSourceAnonymousInnerClass5(
        std::shared_ptr<TestIDVersionPostingsFormat> outerInstance);

    const int radix;
    int upto = 0;
    std::wstring next() override;
  };

private:
  class IDSourceAnonymousInnerClass6
      : public std::enable_shared_from_this<IDSourceAnonymousInnerClass6>,
        public IDSource
  {
    GET_CLASS_NAME(IDSourceAnonymousInnerClass6)
  private:
    std::shared_ptr<TestIDVersionPostingsFormat> outerInstance;

  public:
    IDSourceAnonymousInnerClass6(
        std::shared_ptr<TestIDVersionPostingsFormat> outerInstance);

    const int radix;
    const std::wstring zeroPad;
    int upto = 0;
    std::wstring next() override;
  };

  // TODO make a similar test for BT, w/ varied IDs:

public:
  virtual void testRandom() ;

private:
  class PerThreadVersionPKLookup : public PerThreadPKLookup
  {
    GET_CLASS_NAME(PerThreadVersionPKLookup)
  public:
    PerThreadVersionPKLookup(std::shared_ptr<IndexReader> r,
                             const std::wstring &field) ;

    int64_t lastVersion = 0;

    /** Returns docID if found, else -1. */
    virtual int lookup(std::shared_ptr<BytesRef> id,
                       int64_t version) ;

    /** Only valid if lookup returned a valid docID. */
    virtual int64_t getVersion();

  protected:
    std::shared_ptr<PerThreadVersionPKLookup> shared_from_this()
    {
      return std::static_pointer_cast<PerThreadVersionPKLookup>(
          org.apache.lucene.index.PerThreadPKLookup::shared_from_this());
    }
  };

private:
  static std::shared_ptr<Field> makeIDField(const std::wstring &id,
                                            int64_t version);

public:
  virtual void testMoreThanOneDocPerIDOneSegment() ;

  virtual void testMoreThanOneDocPerIDTwoSegments() ;

private:
  class ConcurrentMergeSchedulerAnonymousInnerClass
      : public ConcurrentMergeScheduler
  {
    GET_CLASS_NAME(ConcurrentMergeSchedulerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIDVersionPostingsFormat> outerInstance;

    std::shared_ptr<Directory> dir;

  public:
    ConcurrentMergeSchedulerAnonymousInnerClass(
        std::shared_ptr<TestIDVersionPostingsFormat> outerInstance,
        std::shared_ptr<Directory> dir);

  protected:
    void handleMergeException(std::shared_ptr<Directory> dir,
                              std::runtime_error exc) override;

  protected:
    std::shared_ptr<ConcurrentMergeSchedulerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          ConcurrentMergeSchedulerAnonymousInnerClass>(
          org.apache.lucene.index.ConcurrentMergeScheduler::shared_from_this());
    }
  };

public:
  virtual void testMoreThanOneDocPerIDWithUpdates() ;

  virtual void testMoreThanOneDocPerIDWithDeletes() ;

  virtual void testMissingPayload() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIDVersionPostingsFormat> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestIDVersionPostingsFormat> outerInstance);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testMissingPositions() ;

  virtual void testInvalidPayload() ;

  virtual void
  testMoreThanOneDocPerIDWithDeletesAcrossSegments() ;

  // LUCENE-5693: because CheckIndex cross-checks term vectors with postings
  // even for deleted docs, and because our PF only indexes the non-deleted
  // documents on flush, CheckIndex will see this as corruption:
  virtual void testCannotIndexTermVectors() ;

  virtual void testMoreThanOnceInSingleDoc() ;

  virtual void testInvalidVersions() ;

  virtual void testInvalidVersions2() ;

  // Simulates optimistic concurrency in a distributed indexing app and confirms
  // the latest version always wins:
  virtual void testGlobalVersions() ;

private:
  class LiveFieldValuesAnonymousInnerClass
      : public LiveFieldValues<std::shared_ptr<IndexSearcher>, int64_t>
  {
    GET_CLASS_NAME(LiveFieldValuesAnonymousInnerClass)
  private:
    std::shared_ptr<TestIDVersionPostingsFormat> outerInstance;

    std::optional<int64_t> missingValue;

  public:
    LiveFieldValuesAnonymousInnerClass(
        std::shared_ptr<TestIDVersionPostingsFormat> outerInstance,
        std::optional<int64_t> &missingValue);

  protected:
    std::optional<int64_t>
    lookupFromSearcher(std::shared_ptr<IndexSearcher> s,
                       const std::wstring &id) override;

  protected:
    std::shared_ptr<LiveFieldValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LiveFieldValuesAnonymousInnerClass>(
          org.apache.lucene.search
              .LiveFieldValues<org.apache.lucene.search.IndexSearcher,
                               long>::shared_from_this());
    }
  };

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIDVersionPostingsFormat> outerInstance;

    std::shared_ptr<RandomIndexWriter> w;
    std::deque<std::wstring> ids;
    std::deque<std::any> locks;
    std::shared_ptr<AtomicLong> nextVersion;
    std::shared_ptr<SearcherManager> mgr;
    std::optional<int64_t> missingValue;
    std::shared_ptr<LiveFieldValues<std::shared_ptr<IndexSearcher>, int64_t>>
        versionValues;
    std::unordered_map<std::wstring, int64_t> truth;
    std::shared_ptr<CountDownLatch> startingGun;
    int versionType = 0;
    int64_t stopTime = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIDVersionPostingsFormat> outerInstance,
        std::shared_ptr<RandomIndexWriter> w, std::deque<std::wstring> &ids,
        std::deque<std::any> &locks, std::shared_ptr<AtomicLong> nextVersion,
        std::shared_ptr<SearcherManager> mgr,
        std::optional<int64_t> &missingValue,
        std::shared_ptr<
            LiveFieldValues<std::shared_ptr<IndexSearcher>, int64_t>>
            versionValues,
        std::unordered_map<std::wstring, int64_t> &truth,
        std::shared_ptr<CountDownLatch> startingGun, int versionType,
        int64_t stopTime);

    void run() override;

  private:
    void runForReal() ;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestIDVersionPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<TestIDVersionPostingsFormat>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::idversion
