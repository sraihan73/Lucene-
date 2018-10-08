#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
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

using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * https://issues.apache.org/jira/browse/LUCENE-1974
 *
 * represent the bug of
 *
 *    BooleanScorer.score(Collector collector, int max, int firstDocID)
 *
 * Line 273, end=8192, subScorerDocID=11378, then more got false?
 */
class TestPrefixInBooleanQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestPrefixInBooleanQuery)

private:
  static const std::wstring FIELD;
  static std::shared_ptr<Directory> directory;
  static std::shared_ptr<IndexReader> reader;
  static std::shared_ptr<IndexSearcher> searcher;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  virtual void testPrefixQuery() ;
  virtual void testTermQuery() ;
  virtual void testTermBooleanQuery() ;
  virtual void testPrefixBooleanQuery() ;

protected:
  std::shared_ptr<TestPrefixInBooleanQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestPrefixInBooleanQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
