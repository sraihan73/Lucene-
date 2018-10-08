#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/TermAutomatonQuery.h"

#include  "core/src/java/org/apache/lucene/search/EnumAndScorer.h"
#include  "core/src/java/org/apache/lucene/util/automaton/RunAutomaton.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/PosState.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
#include  "core/src/java/org/apache/lucene/search/TermAutomatonWeight.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
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

using EnumAndScorer =
    org::apache::lucene::search::TermAutomatonQuery::EnumAndScorer;
using TermAutomatonWeight =
    org::apache::lucene::search::TermAutomatonQuery::TermAutomatonWeight;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using BytesRef = org::apache::lucene::util::BytesRef;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using RunAutomaton = org::apache::lucene::util::automaton::RunAutomaton;

// TODO: add two-phase and needsScores support. maybe use conjunctionDISI
// internally?
class TermAutomatonScorer : public Scorer
{
  GET_CLASS_NAME(TermAutomatonScorer)
private:
  std::deque<std::shared_ptr<EnumAndScorer>> const subs;
  std::deque<std::shared_ptr<EnumAndScorer>> const subsOnDoc;
  const std::shared_ptr<PriorityQueue<std::shared_ptr<EnumAndScorer>>>
      docIDQueue;
  const std::shared_ptr<PriorityQueue<std::shared_ptr<EnumAndScorer>>> posQueue;
  const std::shared_ptr<RunAutomaton> runAutomaton;
  const std::unordered_map<int, std::shared_ptr<BytesRef>> idToTerm;

  // We reuse this array to check for matches starting from an initial
  // position; we increase posShift every time we move to a new possible
  // start:
  std::deque<std::shared_ptr<PosState>> positions;

public:
  int posShift = 0;

  // This is -1 if wildcard (null) terms were not used, else it's the id
  // of the wildcard term:
private:
  const int anyTermID;
  const std::shared_ptr<Similarity::SimScorer> docScorer;

  int numSubsOnDoc = 0;

  const int64_t cost;

  // C++ NOTE: Fields cannot have the same name as methods:
  int docID_ = -1;
  // C++ NOTE: Fields cannot have the same name as methods:
  int freq_ = 0;

public:
  TermAutomatonScorer(
      std::shared_ptr<TermAutomatonWeight> weight,
      std::deque<std::shared_ptr<EnumAndScorer>> &subs, int anyTermID,
      std::unordered_map<int, std::shared_ptr<BytesRef>> &idToTerm,
      std::shared_ptr<Similarity::SimScorer> docScorer) ;

  /** Sorts by docID so we can quickly pull out all scorers that are on
   *  the same (lowest) docID. */
private:
  class DocIDQueue : public PriorityQueue<std::shared_ptr<EnumAndScorer>>
  {
    GET_CLASS_NAME(DocIDQueue)
  public:
    DocIDQueue(int maxSize);

  protected:
    bool lessThan(std::shared_ptr<EnumAndScorer> a,
                  std::shared_ptr<EnumAndScorer> b) override;

  protected:
    std::shared_ptr<DocIDQueue> shared_from_this()
    {
      return std::static_pointer_cast<DocIDQueue>(
          org.apache.lucene.util
              .PriorityQueue<org.apache.lucene.search.TermAutomatonQuery
                                 .EnumAndScorer>::shared_from_this());
    }
  };

  /** Sorts by position so we can visit all scorers on one doc, by
   *  position. */
private:
  class PositionQueue : public PriorityQueue<std::shared_ptr<EnumAndScorer>>
  {
    GET_CLASS_NAME(PositionQueue)
  public:
    PositionQueue(int maxSize);

  protected:
    bool lessThan(std::shared_ptr<EnumAndScorer> a,
                  std::shared_ptr<EnumAndScorer> b) override;

  protected:
    std::shared_ptr<PositionQueue> shared_from_this()
    {
      return std::static_pointer_cast<PositionQueue>(
          org.apache.lucene.util
              .PriorityQueue<org.apache.lucene.search.TermAutomatonQuery
                                 .EnumAndScorer>::shared_from_this());
    }
  };

  /** Pops all enums positioned on the current (minimum) doc */
private:
  void popCurrentDoc();

  /** Pushes all previously pop'd enums back into the docIDQueue */
  void pushCurrentDoc();

public:
  std::shared_ptr<DocIdSetIterator> iterator() override;

private:
  class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
  {
    GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<TermAutomatonScorer> outerInstance;

  public:
    DocIdSetIteratorAnonymousInnerClass(
        std::shared_ptr<TermAutomatonScorer> outerInstance);

    int docID() override;

    int64_t cost() override;

    int nextDoc()  override;

    int advance(int target)  override;

  private:
    int doNext() ;

  protected:
    std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
          DocIdSetIterator::shared_from_this());
    }
  };

private:
  std::shared_ptr<PosState> getPosition(int pos);

  void shift(int pos);

  void countMatches() ;

public:
  virtual std::wstring toString();

  int docID() override;

  float score()  override;

  // for tests
  int freq();

public:
  class TermRunAutomaton : public RunAutomaton
  {
    GET_CLASS_NAME(TermRunAutomaton)
  public:
    TermRunAutomaton(std::shared_ptr<Automaton> a, int termCount);

  protected:
    std::shared_ptr<TermRunAutomaton> shared_from_this()
    {
      return std::static_pointer_cast<TermRunAutomaton>(
          org.apache.lucene.util.automaton.RunAutomaton::shared_from_this());
    }
  };

private:
  class PosState : public std::enable_shared_from_this<PosState>
  {
    GET_CLASS_NAME(PosState)
    // Which automaton states we are in at this position
  public:
    std::deque<int> states = std::deque<int>(2);

    // How many states
    int count = 0;

    virtual void add(int state);
  };

protected:
  std::shared_ptr<TermAutomatonScorer> shared_from_this()
  {
    return std::static_pointer_cast<TermAutomatonScorer>(
        Scorer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
