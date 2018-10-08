#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::util
{
class LineFileDocs;
}
namespace org::apache::lucene::index
{
class SegmentCoreReaders;
}
namespace org::apache::lucene::search
{
class Query;
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
namespace org::apache::lucene::index
{

using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// TODO
//   - mix in forceMerge, addIndexes
//   - randomly mix in non-congruent docs

/** Utility class that spawns multiple indexing and
 *  searching threads. */
class ThreadedIndexingAndSearchingTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(ThreadedIndexingAndSearchingTestCase)

protected:
  const std::shared_ptr<AtomicBoolean> failed =
      std::make_shared<AtomicBoolean>();
  const std::shared_ptr<AtomicInteger> addCount =
      std::make_shared<AtomicInteger>();
  const std::shared_ptr<AtomicInteger> delCount =
      std::make_shared<AtomicInteger>();
  const std::shared_ptr<AtomicInteger> packCount =
      std::make_shared<AtomicInteger>();

  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexWriter> writer;

private:
  class SubDocs : public std::enable_shared_from_this<SubDocs>
  {
    GET_CLASS_NAME(SubDocs)
  public:
    const std::wstring packID;
    const std::deque<std::wstring> subIDs;
    bool deleted = false;

    SubDocs(const std::wstring &packID, std::deque<std::wstring> &subIDs);
  };

  // Called per-search
protected:
  virtual std::shared_ptr<IndexSearcher> getCurrentSearcher() = 0;

  virtual std::shared_ptr<IndexSearcher> getFinalSearcher() = 0;

  virtual void
  releaseSearcher(std::shared_ptr<IndexSearcher> s) ;

  // Called once to run searching
  virtual void doSearching(std::shared_ptr<ExecutorService> es,
                           int64_t stopTime) = 0;

  virtual std::shared_ptr<Directory>
  getDirectory(std::shared_ptr<Directory> in_);

  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: protected void updateDocuments(Term id,
  // java.util.List<? extends Iterable<? extends IndexableField>> docs) throws
  // Exception
  void updateDocuments(std::shared_ptr<Term> id,
                       std::deque<T1> docs) ;

  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: protected void addDocuments(Term id,
  // java.util.List<? extends Iterable<? extends IndexableField>> docs) throws
  // Exception
  void addDocuments(std::shared_ptr<Term> id,
                    std::deque<T1> docs) ;

  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: protected void addDocument(Term id, Iterable<?
  // extends IndexableField> doc) throws Exception
  void addDocument(std::shared_ptr<Term> id,
                   std::deque<T1> doc) ;

  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: protected void updateDocument(Term term,
  // Iterable<? extends IndexableField> doc) throws Exception
  void updateDocument(std::shared_ptr<Term> term,
                      std::deque<T1> doc) ;

  virtual void
  deleteDocuments(std::shared_ptr<Term> term) ;

  virtual void doAfterIndexingThreadDone();

private:
  std::deque<std::shared_ptr<Thread>>
  launchIndexingThreads(std::shared_ptr<LineFileDocs> docs, int numThreads,
                        int64_t const stopTime,
                        std::shared_ptr<Set<std::wstring>> delIDs,
                        std::shared_ptr<Set<std::wstring>> delPackIDs,
                        std::deque<std::shared_ptr<SubDocs>> &allSubDocs);

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<ThreadedIndexingAndSearchingTestCase> outerInstance;

    std::shared_ptr<LineFileDocs> docs;
    int64_t stopTime = 0;
    std::shared_ptr<Set<std::wstring>> delIDs;
    std::shared_ptr<Set<std::wstring>> delPackIDs;
    std::deque<std::shared_ptr<SubDocs>> allSubDocs;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<ThreadedIndexingAndSearchingTestCase> outerInstance,
        std::shared_ptr<LineFileDocs> docs, int64_t stopTime,
        std::shared_ptr<Set<std::wstring>> delIDs,
        std::shared_ptr<Set<std::wstring>> delPackIDs,
        std::deque<std::shared_ptr<SubDocs>> &allSubDocs);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  virtual void
  runSearchThreads(int64_t const stopTimeMS) ;

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<ThreadedIndexingAndSearchingTestCase> outerInstance;

    int64_t stopTimeMS = 0;
    std::shared_ptr<AtomicLong> totHits;
    std::shared_ptr<AtomicInteger> totTermCount;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<ThreadedIndexingAndSearchingTestCase> outerInstance,
        int64_t stopTimeMS, std::shared_ptr<AtomicLong> totHits,
        std::shared_ptr<AtomicInteger> totTermCount);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

protected:
  virtual void
  doAfterWriter(std::shared_ptr<ExecutorService> es) ;

  virtual void doClose() ;

  bool assertMergedSegmentsWarmed = true;

private:
  const std::unordered_map<std::shared_ptr<SegmentCoreReaders>, bool> warmed =
      Collections::synchronizedMap(
          std::make_shared<
              WeakHashMap<std::shared_ptr<SegmentCoreReaders>, bool>>());

public:
  virtual void runTest(const std::wstring &testName) ;

private:
  class PrintStreamInfoStreamAnonymousInnerClass : public PrintStreamInfoStream
  {
    GET_CLASS_NAME(PrintStreamInfoStreamAnonymousInnerClass)
  private:
    std::shared_ptr<ThreadedIndexingAndSearchingTestCase> outerInstance;

  public:
    PrintStreamInfoStreamAnonymousInnerClass(
        std::shared_ptr<ThreadedIndexingAndSearchingTestCase> outerInstance,
        std::shared_ptr<UnknownType> out);

    void message(const std::wstring &component,
                 const std::wstring &message) override;

  protected:
    std::shared_ptr<PrintStreamInfoStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PrintStreamInfoStreamAnonymousInnerClass>(
          org.apache.lucene.util.PrintStreamInfoStream::shared_from_this());
    }
  };

private:
  int64_t runQuery(std::shared_ptr<IndexSearcher> s,
                     std::shared_ptr<Query> q) ;

protected:
  virtual void
  smokeTestSearcher(std::shared_ptr<IndexSearcher> s) ;

protected:
  std::shared_ptr<ThreadedIndexingAndSearchingTestCase> shared_from_this()
  {
    return std::static_pointer_cast<ThreadedIndexingAndSearchingTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
