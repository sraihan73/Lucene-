#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"

#include  "core/src/java/org/apache/lucene/search/spans/SpanCollector.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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
 * Wraps a Spans with additional asserts
 */
class AssertingSpans : public Spans
{
  GET_CLASS_NAME(AssertingSpans)
public:
  const std::shared_ptr<Spans> in_;
  int doc = -1;

  /**
   * tracks current state of this spans
   */
public:
  enum class State {
    GET_CLASS_NAME(State)
    /**
     * document iteration has not yet begun ({@link #docID()} = -1)
     */
    DOC_START,

    /**
     * two-phase iterator has moved to a new docid, but {@link
     * TwoPhaseIterator#matches()} has not been called or it returned false (so
     * you should not do things with the enum)
     */
    DOC_UNVERIFIED,

    /**
     * iterator set to a valid docID, but position iteration has not yet begun
     * ({@link #startPosition() == -1})
     */
    POS_START,

    /**
     * iterator set to a valid docID, and positioned (-1 < {@link
     * #startPosition()} < {@link #NO_MORE_POSITIONS})
     */
    ITERATING,

    /**
     * positions exhausted ({@link #startPosition()} = {@link
     * #NO_MORE_POSITIONS})
     */
    POS_FINISHED,

    /**
     * documents exhausted ({@link #docID()} = {@link #NO_MORE_DOCS})
     */
    DOC_FINISHED
  };

public:
  State state = State::DOC_START;

  AssertingSpans(std::shared_ptr<Spans> in_);

  int nextStartPosition()  override;

private:
  void checkCurrentPositions();

public:
  int startPosition() override;

  int endPosition() override;

  int width() override;

  void
  collect(std::shared_ptr<SpanCollector> collector)  override;

  int docID() override;

  int nextDoc()  override;

  int advance(int target)  override;

  virtual std::wstring toString();

  int64_t cost() override;

  float positionsCost() override;

  std::shared_ptr<TwoPhaseIterator> asTwoPhaseIterator() override;

public:
  class AssertingTwoPhaseView : public TwoPhaseIterator
  {
    GET_CLASS_NAME(AssertingTwoPhaseView)
  private:
    std::shared_ptr<AssertingSpans> outerInstance;

  public:
    const std::shared_ptr<TwoPhaseIterator> in_;
    int lastDoc = -1;

    AssertingTwoPhaseView(std::shared_ptr<AssertingSpans> outerInstance,
                          std::shared_ptr<TwoPhaseIterator> iterator);

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<AssertingTwoPhaseView> shared_from_this()
    {
      return std::static_pointer_cast<AssertingTwoPhaseView>(
          org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
    }
  };

public:
  class AssertingDISI : public DocIdSetIterator
  {
    GET_CLASS_NAME(AssertingDISI)
  private:
    std::shared_ptr<AssertingSpans> outerInstance;

  public:
    const std::shared_ptr<DocIdSetIterator> in_;

    AssertingDISI(std::shared_ptr<AssertingSpans> outerInstance,
                  std::shared_ptr<DocIdSetIterator> in_);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<AssertingDISI> shared_from_this()
    {
      return std::static_pointer_cast<AssertingDISI>(
          org.apache.lucene.search.DocIdSetIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<AssertingSpans> shared_from_this()
  {
    return std::static_pointer_cast<AssertingSpans>(Spans::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
