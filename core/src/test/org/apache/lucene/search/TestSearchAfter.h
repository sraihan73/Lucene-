#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
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
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class SortField;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class Sort;
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

/**
 * Tests IndexSearcher's searchAfter() method
 */
class TestSearchAfter : public LuceneTestCase
{
  GET_CLASS_NAME(TestSearchAfter)
private:
  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<IndexSearcher> searcher;
  int iter = 0;
  std::deque<std::shared_ptr<SortField>> allSortFields;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testQueries() ;

  virtual void
  assertQuery(std::shared_ptr<Query> query) ;

  virtual std::shared_ptr<Sort> getRandomSort();

  virtual void
  assertQuery(std::shared_ptr<Query> query,
              std::shared_ptr<Sort> sort) ;

  virtual void assertPage(int pageStart, std::shared_ptr<TopDocs> all,
                          std::shared_ptr<TopDocs> paged) ;

protected:
  std::shared_ptr<TestSearchAfter> shared_from_this()
  {
    return std::static_pointer_cast<TestSearchAfter>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
