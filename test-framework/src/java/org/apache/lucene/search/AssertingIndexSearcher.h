#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::index
{
class IndexReaderContext;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Collector;
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
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

/**
 * Helper class that adds some extra checks to ensure correct
 * usage of {@code IndexSearcher} and {@code Weight}.
GET_CLASS_NAME(that)
 */
class AssertingIndexSearcher : public IndexSearcher
{
  GET_CLASS_NAME(AssertingIndexSearcher)
public:
  const std::shared_ptr<Random> random;
  AssertingIndexSearcher(std::shared_ptr<Random> random,
                         std::shared_ptr<IndexReader> r);

  AssertingIndexSearcher(std::shared_ptr<Random> random,
                         std::shared_ptr<IndexReaderContext> context);

  AssertingIndexSearcher(std::shared_ptr<Random> random,
                         std::shared_ptr<IndexReader> r,
                         std::shared_ptr<ExecutorService> ex);

  AssertingIndexSearcher(std::shared_ptr<Random> random,
                         std::shared_ptr<IndexReaderContext> context,
                         std::shared_ptr<ExecutorService> ex);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<Query> query,
                                       bool needsScores,
                                       float boost)  override;

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<Query> original)  override;

protected:
  void search(std::deque<std::shared_ptr<LeafReaderContext>> &leaves,
              std::shared_ptr<Weight> weight,
              std::shared_ptr<Collector> collector)  override;

public:
  virtual std::wstring toString();

protected:
  std::shared_ptr<AssertingIndexSearcher> shared_from_this()
  {
    return std::static_pointer_cast<AssertingIndexSearcher>(
        IndexSearcher::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
