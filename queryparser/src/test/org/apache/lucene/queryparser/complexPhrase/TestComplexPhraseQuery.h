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

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::queryparser::complexPhrase
{
class DocData;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::queryparser::complexPhrase
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestComplexPhraseQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestComplexPhraseQuery)
public:
  std::shared_ptr<Directory> rd;
  std::shared_ptr<Analyzer> analyzer;
  std::deque<std::shared_ptr<DocData>> docsContent = {
      std::make_shared<DocData>(L"john smith", L"1", L"developer"),
      std::make_shared<DocData>(L"johathon smith", L"2", L"developer"),
      std::make_shared<DocData>(L"john percival smith", L"3", L"designer"),
      std::make_shared<DocData>(L"jackson waits tom", L"4", L"project manager"),
      std::make_shared<DocData>(L"johny perkins", L"5", L"orders pizza"),
      std::make_shared<DocData>(L"hapax neverson", L"6", L"never matches"),
      std::make_shared<DocData>(L"dog cigar", L"7", L"just for synonyms"),
      std::make_shared<DocData>(L"dogs don't smoke cigarettes", L"8",
                                L"just for synonyms")};

private:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<IndexReader> reader;

public:
  std::wstring defaultFieldName = L"name";

  bool inOrder = true;

  virtual void testComplexPhrases() ;

  virtual void testSingleTermPhrase() ;

  virtual void testSynonyms() ;

  virtual void testUnOrderedProximitySearches() ;

private:
  void checkBadQuery(const std::wstring &qString);

  void checkMatches(const std::wstring &qString,
                    const std::wstring &expectedVals) ;

  void
  checkMatches(const std::wstring &qString, const std::wstring &expectedVals,
               std::shared_ptr<Analyzer> anAnalyzer) ;

public:
  virtual void testFieldedQuery() ;

  virtual void testToStringContainsSlop() ;

  virtual void testHashcodeEquals() ;

  void setUp()  override;

  void tearDown()  override;

public:
  class DocData : public std::enable_shared_from_this<DocData>
  {
    GET_CLASS_NAME(DocData)
  public:
    std::wstring name;

    std::wstring id;

    std::wstring role;

    // C++ TODO: No base class can be determined:
    DocData(const std::wstring &name, const std::wstring &id,
            const std::wstring &role); // super();
  };

protected:
  std::shared_ptr<TestComplexPhraseQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestComplexPhraseQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::complexPhrase
