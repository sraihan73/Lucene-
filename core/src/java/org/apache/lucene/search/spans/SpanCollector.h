#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class PostingsEnum;
}

namespace org::apache::lucene::index
{
class Term;
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

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;

/**
 * An interface defining the collection of postings information from the leaves
 * of a {@link org.apache.lucene.search.spans.Spans}
 *
 * @lucene.experimental
 */
class SpanCollector
{
  GET_CLASS_NAME(SpanCollector)

  /**
   * Collect information from postings
   * @param postings a {@link PostingsEnum}
   * @param position the position of the PostingsEnum
   * @param term     the {@link Term} for this postings deque
   * @throws IOException on error
   */
public:
  virtual void collectLeaf(std::shared_ptr<PostingsEnum> postings, int position,
                           std::shared_ptr<Term> term) = 0;

  /**
   * Call to indicate that the driving Spans has moved to a new position
   */
  virtual void reset() = 0;
};

} // namespace org::apache::lucene::search::spans
