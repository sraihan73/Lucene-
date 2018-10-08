#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spell/SpellCheckerMock.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/spell/SpellChecker.h"
#include  "core/src/java/org/apache/lucene/search/spell/StringDistance.h"
#include  "core/src/java/org/apache/lucene/search/spell/SuggestWord.h"

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
namespace org::apache::lucene::search::spell
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Spell checker test case
 */
class TestSpellChecker : public LuceneTestCase
{
  GET_CLASS_NAME(TestSpellChecker)
private:
  std::shared_ptr<SpellCheckerMock> spellChecker;
  std::shared_ptr<Directory> userindex, spellindex;
  std::shared_ptr<Analyzer> analyzer;
  std::deque<std::shared_ptr<IndexSearcher>> searchers;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testBuild() ;

  virtual void testComparator() ;

  virtual void testBogusField() ;

  virtual void testSuggestModes() ;

private:
  void
  checkCommonSuggestions(std::shared_ptr<IndexReader> r) ;

  void checkLevenshteinSuggestions(std::shared_ptr<IndexReader> r) throw(
      IOException);

  void checkJaroWinklerSuggestions() ;

  void checkNGramSuggestions() ;

  void addwords(std::shared_ptr<IndexReader> r,
                std::shared_ptr<SpellChecker> sc,
                const std::wstring &field) ;

  int numdoc() ;

public:
  virtual void testClose() ;

  /*
   * tests if the internally shared indexsearcher is correctly closed
   * when the spellchecker is concurrently accessed and closed.
   */
  virtual void testConcurrentAccess() ;

private:
  void assertLastSearcherOpen(int numSearchers);

  void assertSearchersClosed();

  // For debug
  //  private void showSearchersOpen() {
  //    int count = 0;
  //    for (IndexSearcher searcher : searchers) {
  //      if(searcher.getIndexReader().getRefCount() > 0)
  //        ++count;
  //    }
  //    System.out.println(count);
  //  }

private:
  class SpellCheckWorker
      : public std::enable_shared_from_this<SpellCheckWorker>,
        public Runnable
  {
    GET_CLASS_NAME(SpellCheckWorker)
  private:
    std::shared_ptr<TestSpellChecker> outerInstance;

    const std::shared_ptr<IndexReader> reader;

  public:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool terminated = false;
    bool terminated = false;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool failed = false;
    bool failed = false;

    // C++ TODO: No base class can be determined:
    SpellCheckWorker(std::shared_ptr<TestSpellChecker> outerInstance,
                     std::shared_ptr<IndexReader> reader); // super();

    void run() override;
  };

public:
  class SpellCheckerMock : public SpellChecker
  {
    GET_CLASS_NAME(SpellCheckerMock)
  private:
    std::shared_ptr<TestSpellChecker> outerInstance;

  public:
    SpellCheckerMock(std::shared_ptr<TestSpellChecker> outerInstance,
                     std::shared_ptr<Directory> spellIndex) ;

    SpellCheckerMock(std::shared_ptr<TestSpellChecker> outerInstance,
                     std::shared_ptr<Directory> spellIndex,
                     std::shared_ptr<StringDistance> sd) ;

    SpellCheckerMock(std::shared_ptr<TestSpellChecker> outerInstance,
                     std::shared_ptr<Directory> spellIndex,
                     std::shared_ptr<StringDistance> sd,
                     std::shared_ptr<Comparator<std::shared_ptr<SuggestWord>>>
                         comparator) ;

    std::shared_ptr<IndexSearcher>
    createSearcher(std::shared_ptr<Directory> dir)  override;

  protected:
    std::shared_ptr<SpellCheckerMock> shared_from_this()
    {
      return std::static_pointer_cast<SpellCheckerMock>(
          SpellChecker::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSpellChecker> shared_from_this()
  {
    return std::static_pointer_cast<TestSpellChecker>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spell/
