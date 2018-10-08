#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"

#include  "core/src/java/org/apache/lucene/index/Term.h"

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
namespace org::apache::lucene::payloads
{

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;

/**
 * SpanCollector for collecting payloads
 */
class PayloadSpanCollector
    : public std::enable_shared_from_this<PayloadSpanCollector>,
      public SpanCollector
{
  GET_CLASS_NAME(PayloadSpanCollector)

private:
  const std::shared_ptr<std::deque<std::deque<char>>> payloads =
      std::deque<std::deque<char>>();

public:
  void collectLeaf(std::shared_ptr<PostingsEnum> postings, int position,
                   std::shared_ptr<Term> term)  override;

  void reset() override;

  /**
   * @return the collected payloads
   */
  virtual std::shared_ptr<std::deque<std::deque<char>>> getPayloads();
};

} // #include  "core/src/java/org/apache/lucene/payloads/
