#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
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
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::index
{
class RandomIndexWriter;
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
namespace org::apache::lucene::sandbox::queries
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class FuzzyLikeThisQueryTest : public LuceneTestCase
{
  GET_CLASS_NAME(FuzzyLikeThisQueryTest)
private:
  std::shared_ptr<Directory> directory;
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<Analyzer> analyzer;

public:
  void setUp()  override;

  void tearDown()  override;

private:
  void addDoc(std::shared_ptr<RandomIndexWriter> writer,
              const std::wstring &name,
              const std::wstring &id) ;

  // Tests that idf ranking is not favouring rare mis-spellings over a strong
  // edit-distance match
public:
  virtual void
  testClosestEditDistanceMatchComesFirst() ;

  // Test multiple input words are having variants produced
  virtual void testMultiWord() ;

  // LUCENE-4809
  virtual void testNonExistingField() ;

  // Test bug found when first query word does not match anything
  virtual void testNoMatchFirstWordBug() ;

  virtual void testFuzzyLikeThisQueryEquals();

protected:
  std::shared_ptr<FuzzyLikeThisQueryTest> shared_from_this()
  {
    return std::static_pointer_cast<FuzzyLikeThisQueryTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::sandbox::queries
