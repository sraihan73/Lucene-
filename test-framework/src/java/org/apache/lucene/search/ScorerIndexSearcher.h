#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Collector;
}
namespace org::apache::lucene::search
{
class Weight;
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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

/**
 * An {@link IndexSearcher} that always uses the {@link Scorer} API, never
 * {@link BulkScorer}.
 */
class ScorerIndexSearcher : public IndexSearcher
{
  GET_CLASS_NAME(ScorerIndexSearcher)

  /** Creates a searcher searching the provided index. Search on individual
   *  segments will be run in the provided {@link ExecutorService}.
   * @see IndexSearcher#IndexSearcher(IndexReader, ExecutorService) */
public:
  ScorerIndexSearcher(std::shared_ptr<IndexReader> r,
                      std::shared_ptr<ExecutorService> executor);

  /** Creates a searcher searching the provided index.
   * @see IndexSearcher#IndexSearcher(IndexReader) */
  ScorerIndexSearcher(std::shared_ptr<IndexReader> r);

protected:
  void search(std::deque<std::shared_ptr<LeafReaderContext>> &leaves,
              std::shared_ptr<Weight> weight,
              std::shared_ptr<Collector> collector)  override;

protected:
  std::shared_ptr<ScorerIndexSearcher> shared_from_this()
  {
    return std::static_pointer_cast<ScorerIndexSearcher>(
        IndexSearcher::shared_from_this());
  }
};
} // namespace org::apache::lucene::search
