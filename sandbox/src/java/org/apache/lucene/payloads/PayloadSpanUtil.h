#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReaderContext;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search::spans
{
class SpanQuery;
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
namespace org::apache::lucene::payloads
{

using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;

/**
 * Experimental class to get set of payloads for most standard Lucene queries.
 * Operates like Highlighter - IndexReader should only contain doc of interest,
 * best to use MemoryIndex.
 *
 * @lucene.experimental
 *
 */
class PayloadSpanUtil : public std::enable_shared_from_this<PayloadSpanUtil>
{
  GET_CLASS_NAME(PayloadSpanUtil)
private:
  std::shared_ptr<IndexReaderContext> context;

  /**
   * @param context
   *          that contains doc with payloads to extract
   *
   * @see IndexReader#getContext()
   */
public:
  PayloadSpanUtil(std::shared_ptr<IndexReaderContext> context);

  /**
   * Query should be rewritten for wild/fuzzy support.
   *
   * @param query rewritten query
   * @return payloads std::deque
   * @throws IOException if there is a low-level I/O error
   */
  virtual std::shared_ptr<std::deque<std::deque<char>>>
  getPayloadsForQuery(std::shared_ptr<Query> query) ;

private:
  void queryToSpanQuery(std::shared_ptr<Query> query,
                        std::shared_ptr<std::deque<std::deque<char>>>
                            payloads) ;

  void getPayloads(std::shared_ptr<std::deque<std::deque<char>>> payloads,
                   std::shared_ptr<SpanQuery> query) ;
};

} // namespace org::apache::lucene::payloads
