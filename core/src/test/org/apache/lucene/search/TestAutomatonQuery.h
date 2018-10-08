#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

class TestAutomatonQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestAutomatonQuery)
private:
  std::shared_ptr<Directory> directory;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<IndexSearcher> searcher;

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
   * Test some very simple automata.
   */
public:
  virtual void testAutomata() ;

  /**
   * Test that a nondeterministic automaton works correctly. (It should will be
   * determinized)
   */
  virtual void testNFA() ;

  virtual void testEquals();

  /**
   * Test that rewriting to a single term works as expected, preserves
   * MultiTermQuery semantics.
   */
  virtual void testRewriteSingleTerm() ;

  /**
   * Test that rewriting to a prefix query works as expected, preserves
   * MultiTermQuery semantics.
   */
  virtual void testRewritePrefix() ;

  /**
   * Test handling of the empty language
   */
  virtual void testEmptyOptimization() ;

  virtual void testHashCodeWithThreads() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestAutomatonQuery> outerInstance;

    std::deque<std::shared_ptr<org::apache::lucene::search::AutomatonQuery>>
        queries;
    std::shared_ptr<CountDownLatch> startingGun;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestAutomatonQuery> outerInstance,
        std::deque<
            std::shared_ptr<org::apache::lucene::search::AutomatonQuery>>
            &queries,
        std::shared_ptr<CountDownLatch> startingGun);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testBiggishAutomaton();

protected:
  std::shared_ptr<TestAutomatonQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestAutomatonQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
