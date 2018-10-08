#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::store
{
class Directory;
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
 * Create an index with terms from 000-999.
 * Generates random regexps according to simple patterns,
 * and validates the correct number of hits are returned.
 */
class TestRegexpRandom : public LuceneTestCase
{
  GET_CLASS_NAME(TestRegexpRandom)
private:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<Directory> dir;

public:
  void setUp()  override;

private:
  wchar_t N();

  std::wstring fillPattern(const std::wstring &wildcardPattern);

  void assertPatternHits(const std::wstring &pattern,
                         int numHits) ;

public:
  void tearDown()  override;

  virtual void testRegexps() ;

protected:
  std::shared_ptr<TestRegexpRandom> shared_from_this()
  {
    return std::static_pointer_cast<TestRegexpRandom>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
