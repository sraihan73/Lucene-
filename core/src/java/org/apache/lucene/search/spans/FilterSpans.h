#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::spans
{
class Spans;
}

namespace org::apache::lucene::search::spans
{
class SpanCollector;
}
namespace org::apache::lucene::search
{
class TwoPhaseIterator;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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
namespace org::apache::lucene::search::spans
{

using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;

/**
 * A {@link Spans} implementation wrapping another spans instance,
 * allowing to filter spans matches easily by implementing {@link #accept}
 */
class FilterSpans : public Spans
{
  GET_CLASS_NAME(FilterSpans)

  /** The wrapped spans instance. */
protected:
  const std::shared_ptr<Spans> in_;

private:
  bool atFirstInCurrentDoc = false;
  int startPos = -1;

  /** Wrap the given {@link Spans}. */
protected:
  FilterSpans(std::shared_ptr<Spans> in_);

  /**
   * Returns YES if the candidate should be an accepted match,
   * NO if it should not, and NO_MORE_IN_CURRENT_DOC if iteration
   * should move on to the next document.
   */
  virtual AcceptStatus accept(std::shared_ptr<Spans> candidate) = 0;

public:
  int nextDoc()  override final;

  int advance(int target)  override final;

  int docID() override final;

  int nextStartPosition()  override final;

  int startPosition() override final;

  int endPosition() override final;

  int width() override;

  void
  collect(std::shared_ptr<SpanCollector> collector)  override;

  int64_t cost() override final;

  virtual std::wstring toString();

  std::shared_ptr<TwoPhaseIterator> asTwoPhaseIterator() override final;

private:
  class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<FilterSpans> outerInstance;

    std::shared_ptr<TwoPhaseIterator> inner;

  public:
    TwoPhaseIteratorAnonymousInnerClass(
        std::shared_ptr<FilterSpans> outerInstance,
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            approximation,
        std::shared_ptr<TwoPhaseIterator> inner);

    bool matches()  override;

    float matchCost() override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
          org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
    }
  };

private:
  class TwoPhaseIteratorAnonymousInnerClass2 : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass2)
  private:
    std::shared_ptr<FilterSpans> outerInstance;

  public:
    TwoPhaseIteratorAnonymousInnerClass2(
        std::shared_ptr<FilterSpans> outerInstance,
        std::shared_ptr<org::apache::lucene::search::spans::Spans> in_);

    bool matches()  override;

    float matchCost() override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass2>(
          org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
    }
  };

public:
  float positionsCost() override;

  /**
   * Returns true if the current document matches.
   * <p>
   * This is called during two-phase processing.
   */
  // return true if the current document matches
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") private final bool
  // twoPhaseCurrentDocMatches() throws java.io.IOException
  bool twoPhaseCurrentDocMatches() ;

  /**
   * Status returned from {@link FilterSpans#accept(Spans)} that indicates
   * whether a candidate match should be accepted, rejected, or rejected
   * and move on to the next document.
   */
public:
  enum class AcceptStatus {
    GET_CLASS_NAME(AcceptStatus)
    /** Indicates the match should be accepted */
    YES,

    /** Indicates the match should be rejected */
    NO,

    /**
     * Indicates the match should be rejected, and the enumeration may continue
     * with the next document.
     */
    NO_MORE_IN_CURRENT_DOC
  };

protected:
  std::shared_ptr<FilterSpans> shared_from_this()
  {
    return std::static_pointer_cast<FilterSpans>(Spans::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spans
