#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/AutomatonQuery.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"

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
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Automaton = org::apache::lucene::util::automaton::Automaton;

/**
 * Test the automaton query for several unicode corner cases,
 * specifically enumerating strings/indexes containing supplementary characters,
 * and the differences between UTF-8/UTF-32 and UTF-16 binary sort order.
 */
class TestAutomatonQueryUnicode : public LuceneTestCase
{
  GET_CLASS_NAME(TestAutomatonQueryUnicode)
private:
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<Directory> directory;

  static const std::wstring FN;

public:
  void setUp()  override;

  void tearDown()  override;

private:
  std::shared_ptr<Term> newTerm(const std::wstring &value);

  int64_t automatonQueryNrHits(std::shared_ptr<AutomatonQuery> query) throw(
      IOException);

  void
  assertAutomatonHits(int expected,
                      std::shared_ptr<Automaton> automaton) ;

  /**
   * Test that AutomatonQuery interacts with lucene's sort order correctly.
   *
   * This expression matches something either starting with the arabic
   * presentation forms block, or a supplementary character.
   */
public:
  virtual void testSortOrder() ;

protected:
  std::shared_ptr<TestAutomatonQueryUnicode> shared_from_this()
  {
    return std::static_pointer_cast<TestAutomatonQueryUnicode>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
