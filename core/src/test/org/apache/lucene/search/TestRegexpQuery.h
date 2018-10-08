#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

/**
 * Some simple regex tests, mostly converted from contrib's TestRegexQuery.
 */
class TestRegexpQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestRegexpQuery)
private:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<Directory> directory;
  static const std::wstring FN;

public:
  void setUp()  override;

  void tearDown()  override;

private:
  std::shared_ptr<Term> newTerm(const std::wstring &value);

  int64_t regexQueryNrHits(const std::wstring &regex) ;

public:
  virtual void testRegex1() ;

  virtual void testRegex2() ;

  virtual void testRegex3() ;

  virtual void testNumericRange() ;

  virtual void testRegexComplement() ;

  virtual void testCustomProvider() ;

private:
  class AutomatonProviderAnonymousInnerClass
      : public std::enable_shared_from_this<
            AutomatonProviderAnonymousInnerClass>,
        public AutomatonProvider
  {
    GET_CLASS_NAME(AutomatonProviderAnonymousInnerClass)
  private:
    std::shared_ptr<TestRegexpQuery> outerInstance;

  public:
    AutomatonProviderAnonymousInnerClass(
        std::shared_ptr<TestRegexpQuery> outerInstance);

    // automaton that matches quick or brown
  private:
    std::shared_ptr<Automaton> quickBrownAutomaton;

  public:
    std::shared_ptr<Automaton> getAutomaton(const std::wstring &name);
  };

  /**
   * Test a corner case for backtracking: In this case the term dictionary has
   * 493432 followed by 49344. When backtracking from 49343... to 4934, it's
   * necessary to test that 4934 itself is ok before trying to append more
   * characters.
   */
public:
  virtual void testBacktracking() ;

protected:
  std::shared_ptr<TestRegexpQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestRegexpQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
