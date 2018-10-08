#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"

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

using PriorityQueue = org::apache::lucene::util::PriorityQueue;

class HitQueue final : public PriorityQueue<std::shared_ptr<ScoreDoc>>
{
  GET_CLASS_NAME(HitQueue)

  /**
   * Creates a new instance with <code>size</code> elements. If
   * <code>prePopulate</code> is set to true, the queue will pre-populate itself
   * with sentinel objects and set its {@link #size()} to <code>size</code>. In
   * that case, you should not rely on {@link #size()} to get the number of
   * actual elements that were added to the queue, but keep track yourself.<br>
   * <b>NOTE:</b> in case <code>prePopulate</code> is true, you should pop
   * elements from the queue using the following code example:
   *
   * <pre class="prettyprint">
   * PriorityQueue&lt;ScoreDoc&gt; pq = new HitQueue(10, true); // pre-populate.
   * ScoreDoc top = pq.top();
   *
   * // Add/Update one element.
   * top.score = 1.0f;
   * top.doc = 0;
   * top = (ScoreDoc) pq.updateTop();
   * int totalHits = 1;
   *
   * // Now pop only the elements that were *truly* inserted.
   * // First, pop all the sentinel elements (there are pq.size() - totalHits).
   * for (int i = pq.size() - totalHits; i &gt; 0; i--) pq.pop();
   *
   * // Now pop the truly added elements.
   * ScoreDoc[] results = new ScoreDoc[totalHits];
   * for (int i = totalHits - 1; i &gt;= 0; i--) {
   *   results[i] = (ScoreDoc) pq.pop();
   * }
   * </pre>
   *
   * <p><b>NOTE</b>: This class pre-allocate a full array of
   * length <code>size</code>.
   *
   * @param size
   *          the requested size of this queue.
   * @param prePopulate
   *          specifies whether to pre-populate the queue with sentinel values.
   * @see #getSentinelObject()
   */
public:
  HitQueue(int size, bool prePopulate);

protected:
  std::shared_ptr<ScoreDoc> getSentinelObject() override;

  bool lessThan(std::shared_ptr<ScoreDoc> hitA,
                std::shared_ptr<ScoreDoc> hitB) override;

protected:
  std::shared_ptr<HitQueue> shared_from_this()
  {
    return std::static_pointer_cast<HitQueue>(
        org.apache.lucene.util.PriorityQueue<ScoreDoc>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
