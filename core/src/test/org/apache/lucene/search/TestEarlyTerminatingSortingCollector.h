#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"

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
namespace org::apache::lucene::search
{

using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using QueryTimeout = org::apache::lucene::index::QueryTimeout;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestEarlyTerminatingSortingCollector : public LuceneTestCase
{
  GET_CLASS_NAME(TestEarlyTerminatingSortingCollector)

private:
  int numDocs = 0;
  std::deque<std::wstring> terms;
  std::shared_ptr<Directory> dir;
  const std::shared_ptr<Sort> sort = std::make_shared<Sort>(
      std::make_shared<SortField>(L"ndv1", SortField::Type::LONG));
  std::shared_ptr<RandomIndexWriter> iw;
  std::shared_ptr<IndexReader> reader;
  static constexpr int FORCE_MERGE_MAX_SEGMENT_COUNT = 5;

  std::shared_ptr<Document> randomDocument();

  void createRandomIndex(bool singleSortedSegment) ;

  void closeIndex() ;

public:
  virtual void testEarlyTermination() ;

  virtual void testCanEarlyTerminate();

  virtual void testEarlyTerminationDifferentSorter() ;

private:
  static void
  assertTopDocsEquals(std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs1,
                      std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs2);

private:
  class TestTerminatedEarlySimpleCollector : public SimpleCollector
  {
    GET_CLASS_NAME(TestTerminatedEarlySimpleCollector)
  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    bool collectedSomething_ = false;

  public:
    virtual bool collectedSomething();
    void collect(int doc)  override;
    bool needsScores() override;

  protected:
    std::shared_ptr<TestTerminatedEarlySimpleCollector> shared_from_this()
    {
      return std::static_pointer_cast<TestTerminatedEarlySimpleCollector>(
          SimpleCollector::shared_from_this());
    }
  };

private:
  class TestEarlyTerminatingSortingcollectorQueryTimeout
      : public std::enable_shared_from_this<
            TestEarlyTerminatingSortingcollectorQueryTimeout>,
        public QueryTimeout
  {
    GET_CLASS_NAME(TestEarlyTerminatingSortingcollectorQueryTimeout)
  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    const bool shouldExit_;

  public:
    TestEarlyTerminatingSortingcollectorQueryTimeout(bool shouldExit);
    bool shouldExit() override;
  };

public:
  virtual void testTerminatedEarly() ;

protected:
  std::shared_ptr<TestEarlyTerminatingSortingCollector> shared_from_this()
  {
    return std::static_pointer_cast<TestEarlyTerminatingSortingCollector>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
