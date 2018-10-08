#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <map_obj>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

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
class Query;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Sort;
}
namespace org::apache::lucene::search
{
class ScoreDoc;
}
namespace org::apache::lucene::search
{
class TopFieldDocs;
}
namespace org::apache::lucene::search
{
class TopDocs;
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

/** Unit test for sorting code. */
class TestCustomSearcherSort : public LuceneTestCase
{
  GET_CLASS_NAME(TestCustomSearcherSort)

private:
  std::shared_ptr<Directory> index = nullptr;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<Query> query = nullptr;
  // reduced from 20000 to 2000 to speed up test...
  int INDEX_SIZE = 0;

  /**
   * Create index and query for test cases.
   */
public:
  void setUp()  override;

  void tearDown()  override;

  /**
   * Run the test using two CustomSearcher instances.
   */
  virtual void testFieldSortCustomSearcher() ;

  /**
   * Run the test using one CustomSearcher wrapped by a MultiSearcher.
   */
  virtual void testFieldSortSingleSearcher() ;

  // make sure the documents returned by the search match the expected deque
private:
  void matchHits(std::shared_ptr<IndexSearcher> searcher,
                 std::shared_ptr<Sort> sort) ;

  /**
   * Check the hits for duplicates.
   */
  void checkHits(std::deque<std::shared_ptr<ScoreDoc>> &hits,
                 const std::wstring &prefix);

  // Simply write to console - choosen to be independant of log4j etc
  void log(const std::wstring &message);

public:
  class CustomSearcher : public IndexSearcher
  {
    GET_CLASS_NAME(CustomSearcher)
  private:
    int switcher = 0;

  public:
    CustomSearcher(std::shared_ptr<IndexReader> r, int switcher);

    std::shared_ptr<TopFieldDocs>
    search(std::shared_ptr<Query> query, int nDocs,
           std::shared_ptr<Sort> sort)  override;

    std::shared_ptr<TopDocs> search(std::shared_ptr<Query> query,
                                    int nDocs)  override;

  protected:
    std::shared_ptr<CustomSearcher> shared_from_this()
    {
      return std::static_pointer_cast<CustomSearcher>(
          IndexSearcher::shared_from_this());
    }
  };

private:
  class RandomGen : public std::enable_shared_from_this<RandomGen>
  {
    GET_CLASS_NAME(RandomGen)
  public:
    RandomGen(std::shared_ptr<Random> random);

  private:
    std::shared_ptr<Random> random;
    // we use the default Locale/TZ since LuceneTestCase randomizes it
    std::shared_ptr<Calendar> base = std::make_shared<GregorianCalendar>(
        TimeZone::getDefault(), Locale::getDefault());

    // Just to generate some different Lucene Date strings
    std::wstring getLuceneDate();
  };

protected:
  std::shared_ptr<TestCustomSearcherSort> shared_from_this()
  {
    return std::static_pointer_cast<TestCustomSearcherSort>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
