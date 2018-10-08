#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::document
{
class FieldType;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class MatchesIterator;
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

using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestMatchesIterator : public LuceneTestCase
{
  GET_CLASS_NAME(TestMatchesIterator)

protected:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<Directory> directory;
  std::shared_ptr<IndexReader> reader;

private:
  static const std::wstring FIELD_WITH_OFFSETS;
  static const std::wstring FIELD_NO_OFFSETS;
  static const std::wstring FIELD_DOCS_ONLY;
  static const std::wstring FIELD_FREQS;

  static const std::shared_ptr<FieldType> OFFSETS;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static TestMatchesIterator::StaticConstructor staticConstructor;

  static const std::shared_ptr<FieldType> DOCS;

  static const std::shared_ptr<FieldType> DOCS_AND_FREQS;

public:
  void tearDown()  override;

  void setUp()  override;

protected:
  std::deque<std::wstring> docFields = {
      L"w1 w2 w3 w4 w5",
      L"w1 w3 w2 w3 zz",
      L"w1 xx w2 yy w4",
      L"w1 w2 w1 w4 w2 w3",
      L"a phrase sentence with many phrase sentence iterations of a phrase "
      L"sentence",
      L"nothing matches this document"};

public:
  virtual void
  checkMatches(std::shared_ptr<Query> q, const std::wstring &field,
               std::deque<std::deque<int>> &expected) ;

  virtual void checkFieldMatches(std::shared_ptr<MatchesIterator> it,
                                 std::deque<int> &expected) ;

  virtual void
  checkNoPositionsMatches(std::shared_ptr<Query> q, const std::wstring &field,
                          std::deque<bool> &expected) ;

  virtual void testTermQuery() ;

  virtual void testTermQueryNoStoredOffsets() ;

  virtual void testTermQueryNoPositions() ;

  virtual void testDisjunction() ;

  virtual void testDisjunctionNoPositions() ;

  virtual void testReqOpt() ;

  virtual void testReqOptNoPositions() ;

  virtual void testMinShouldMatch() ;

  virtual void testMinShouldMatchNoPositions() ;

  virtual void testExclusion() ;

  virtual void testExclusionNoPositions() ;

  virtual void testConjunction() ;

  virtual void testConjunctionNoPositions() ;

  virtual void testWildcards() ;

  virtual void testNoMatchWildcards() ;

  virtual void testWildcardsNoPositions() ;

  virtual void testSynonymQuery() ;

  virtual void testSynonymQueryNoPositions() ;

  virtual void testMultipleFields() ;

  //  0         1         2         3         4         5         6         7
  // "a phrase sentence with many phrase sentence iterations of a phrase
  // sentence",

  virtual void testSloppyPhraseQuery() ;

  virtual void testExactPhraseQuery() ;

  //  0         1         2         3         4         5         6         7
  // "a phrase sentence with many phrase sentence iterations of a phrase
  // sentence",

  virtual void testSloppyMultiPhraseQuery() ;

  virtual void testExactMultiPhraseQuery() ;

protected:
  std::shared_ptr<TestMatchesIterator> shared_from_this()
  {
    return std::static_pointer_cast<TestMatchesIterator>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
