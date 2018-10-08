#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::search::intervals
{
class IntervalsSource;
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

namespace org::apache::lucene::search::intervals
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIntervals : public LuceneTestCase
{
  GET_CLASS_NAME(TestIntervals)

private:
  static std::deque<std::wstring> field1_docs;

  static std::deque<std::wstring> field2_docs;

  static std::shared_ptr<Directory> directory;
  static std::shared_ptr<IndexSearcher> searcher;
  static std::shared_ptr<Analyzer> analyzer;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void setupIndex() throws
  // java.io.IOException
  static void setupIndex() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void teardownIndex() throws
  // java.io.IOException
  static void teardownIndex() ;

private:
  void
  checkIntervals(std::shared_ptr<IntervalsSource> source,
                 const std::wstring &field, int expectedMatchCount,
                 std::deque<std::deque<int>> &expected) ;

public:
  virtual void testIntervalsOnFieldWithNoPositions() ;

  virtual void testTermQueryIntervals() ;

  virtual void testOrderedNearIntervals() ;

  virtual void testPhraseIntervals() ;

  virtual void testUnorderedNearIntervals() ;

  virtual void testIntervalDisjunction() ;

  virtual void testNesting() ;

  virtual void testNesting2() ;

  virtual void testUnorderedDistinct() ;

protected:
  std::shared_ptr<TestIntervals> shared_from_this()
  {
    return std::static_pointer_cast<TestIntervals>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::intervals
