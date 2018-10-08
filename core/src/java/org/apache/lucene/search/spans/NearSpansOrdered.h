#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
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

/**
 * A Spans that is formed from the ordered subspans of a SpanNearQuery
 * where the subspans do not overlap and have a maximum slop between them.
 * <p>
 * The formed spans only contains minimum slop matches.<br>
 * The matching slop is computed from the distance(s) between
 * the non overlapping matching Spans.<br>
 * Successive matches are always formed from the successive Spans
 * of the SpanNearQuery.
 * <p>
 * The formed spans may contain overlaps when the slop is at least 1.
 * For example, when querying using
 * <pre>t1 t2 t3</pre>
 * with slop at least 1, the fragment:
 * <pre>t1 t2 t1 t3 t2 t3</pre>
 * matches twice:
 * <pre>t1 t2 .. t3      </pre>
 * <pre>      t1 .. t2 t3</pre>
 *
 * Expert:
 * Only public for subclassing.  Most implementations should not need this class
 */
class NearSpansOrdered : public ConjunctionSpans
{
  GET_CLASS_NAME(NearSpansOrdered)

protected:
  int matchStart = -1;
  int matchEnd = -1;
  int matchWidth = -1;

private:
  const int allowedSlop;

public:
  NearSpansOrdered(
      int allowedSlop,
      std::deque<std::shared_ptr<Spans>> &subSpans) ;

  bool twoPhaseCurrentDocMatches()  override;

private:
  bool unpositioned();

public:
  int nextStartPosition()  override;

  /**
   * Order the subSpans within the same document by using nextStartPosition on
   * all subSpans after the first as little as necessary. Return true when the
   * subSpans could be ordered in this way, otherwise at least one is exhausted
   * in the current doc.
   */
private:
  bool stretchToOrder() ;

  static int advancePosition(std::shared_ptr<Spans> spans,
                             int position) ;

public:
  int startPosition() override;

  int endPosition() override;

  int width() override;

  void
  collect(std::shared_ptr<SpanCollector> collector)  override;

protected:
  std::shared_ptr<NearSpansOrdered> shared_from_this()
  {
    return std::static_pointer_cast<NearSpansOrdered>(
        ConjunctionSpans::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
