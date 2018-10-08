#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"

#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"

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
namespace org::apache::lucene::search::spans
{

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;

/**
 * Common super class for multiple sub spans required in a document.
 */
class ConjunctionSpans : public Spans
{
  GET_CLASS_NAME(ConjunctionSpans)
public:
  std::deque<std::shared_ptr<Spans>> const subSpans; // in query order
  const std::shared_ptr<DocIdSetIterator>
      conjunction;                  // use to move to next doc with all clauses
  bool atFirstInCurrentDoc = false; // a first start position is available in
                                    // current doc for nextStartPosition
  bool oneExhaustedInCurrentDoc =
      false; // one subspans exhausted in current doc

  ConjunctionSpans(std::deque<std::shared_ptr<Spans>> &subSpans);

  int docID() override;

  int64_t cost() override;

  int nextDoc()  override;

  int advance(int target)  override;

  virtual int toMatchDoc() ;

  virtual bool twoPhaseCurrentDocMatches() = 0;

  /**
   * Return a {@link TwoPhaseIterator} view of this ConjunctionSpans.
   */
  std::shared_ptr<TwoPhaseIterator> asTwoPhaseIterator() override;

private:
  class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<ConjunctionSpans> outerInstance;

    float matchCost = 0;

  public:
    TwoPhaseIteratorAnonymousInnerClass(
        std::shared_ptr<ConjunctionSpans> outerInstance,
        std::shared_ptr<DocIdSetIterator> conjunction, float matchCost);

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
          org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
    }
  };

public:
  float positionsCost() override;

  virtual std::deque<std::shared_ptr<Spans>> getSubSpans();

protected:
  std::shared_ptr<ConjunctionSpans> shared_from_this()
  {
    return std::static_pointer_cast<ConjunctionSpans>(
        Spans::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
