#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class QueryCachingPolicy;
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

/**
 * A cache for queries.
 *
 * @see LRUQueryCache
 * @lucene.experimental
 */
class QueryCache
{
  GET_CLASS_NAME(QueryCache)

  /**
   * Return a wrapper around the provided <code>weight</code> that will cache
   * matching docs per-segment accordingly to the given <code>policy</code>.
   * NOTE: The returned weight will only be equivalent if scores are not needed.
   * @see Collector#needsScores()
   */
public:
  virtual std::shared_ptr<Weight>
  doCache(std::shared_ptr<Weight> weight,
          std::shared_ptr<QueryCachingPolicy> policy) = 0;
};

} // namespace org::apache::lucene::search
