#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::search
{
class Sort;
}
namespace org::apache::lucene::index
{
class RandomIndexWriter;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::search
{
class ScoreDoc;
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
namespace org::apache::lucene::search
{

using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestTopFieldCollectorEarlyTermination : public LuceneTestCase
{
  GET_CLASS_NAME(TestTopFieldCollectorEarlyTermination)

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

  virtual void testEarlyTerminationWhenPaging() ;

private:
  void doTestEarlyTermination(bool paging) ;

public:
  virtual void testCanEarlyTerminate();

private:
  static void
  assertTopDocsEquals(std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs1,
                      std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs2);

protected:
  std::shared_ptr<TestTopFieldCollectorEarlyTermination> shared_from_this()
  {
    return std::static_pointer_cast<TestTopFieldCollectorEarlyTermination>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
