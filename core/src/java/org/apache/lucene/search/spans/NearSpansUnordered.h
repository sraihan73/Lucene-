#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanTotalLengthEndPositionWindow.h"

#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanCollector.h"

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

using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Similar to {@link NearSpansOrdered}, but for the unordered case.
 *
 * Expert:
 * Only public for subclassing.  Most implementations should not need this class
 */
class NearSpansUnordered : public ConjunctionSpans
{
  GET_CLASS_NAME(NearSpansUnordered)

private:
  const int allowedSlop;
  std::shared_ptr<SpanTotalLengthEndPositionWindow> spanWindow;

public:
  NearSpansUnordered(
      int allowedSlop,
      std::deque<std::shared_ptr<Spans>> &subSpans) ;

  /** Maintain totalSpanLength and maxEndPosition */
private:
  class SpanTotalLengthEndPositionWindow
      : public PriorityQueue<std::shared_ptr<Spans>>
  {
    GET_CLASS_NAME(SpanTotalLengthEndPositionWindow)
  private:
    std::shared_ptr<NearSpansUnordered> outerInstance;

  public:
    int totalSpanLength = 0;
    int maxEndPosition = 0;

    SpanTotalLengthEndPositionWindow(
        std::shared_ptr<NearSpansUnordered> outerInstance);

  protected:
    bool lessThan(std::shared_ptr<Spans> spans1,
                  std::shared_ptr<Spans> spans2) override;

  public:
    virtual void startDocument() ;

    virtual bool nextPosition() ;

    virtual bool atMatch();

  protected:
    std::shared_ptr<SpanTotalLengthEndPositionWindow> shared_from_this()
    {
      return std::static_pointer_cast<SpanTotalLengthEndPositionWindow>(
          org.apache.lucene.util.PriorityQueue<Spans>::shared_from_this());
    }
  };

  /** Check whether two Spans in the same document are ordered with possible
   * overlap.
   * @return true iff spans1 starts before spans2
   *              or the spans start at the same position,
   *              and spans1 ends before spans2.
   */
public:
  static bool positionsOrdered(std::shared_ptr<Spans> spans1,
                               std::shared_ptr<Spans> spans2);

  bool twoPhaseCurrentDocMatches()  override;

  int nextStartPosition()  override;

  int startPosition() override;

  int endPosition() override;

  int width() override;

  void
  collect(std::shared_ptr<SpanCollector> collector)  override;

protected:
  std::shared_ptr<NearSpansUnordered> shared_from_this()
  {
    return std::static_pointer_cast<NearSpansUnordered>(
        ConjunctionSpans::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
