#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class ScoreDoc;
}

namespace org::apache::lucene::search
{
class Sort;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class ShardIndexSearcher;
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

// TODO
//   - other queries besides PrefixQuery & TermQuery (but:
//     FuzzyQ will be problematic... the top N terms it
//     takes means results will differ)
//   - NRQ/F
//   - BQ, negated clauses, negated prefix clauses
//   - test pulling docs in 2nd round trip...
//   - filter too

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({ "SimpleText", "Memory", "Direct" }) public
// class TestShardSearching extends ShardSearchingTestBase
class TestShardSearching : public ShardSearchingTestBase
{

private:
  class PreviousSearchState
      : public std::enable_shared_from_this<PreviousSearchState>
  {
    GET_CLASS_NAME(PreviousSearchState)
  public:
    const int64_t searchTimeNanos;
    std::deque<int64_t> const versions;
    const std::shared_ptr<ScoreDoc> searchAfterLocal;
    const std::shared_ptr<ScoreDoc> searchAfterShard;
    const std::shared_ptr<Sort> sort;
    const std::shared_ptr<Query> query;
    const int numHitsPaged;

    PreviousSearchState(std::shared_ptr<Query> query,
                        std::shared_ptr<Sort> sort,
                        std::shared_ptr<ScoreDoc> searchAfterLocal,
                        std::shared_ptr<ScoreDoc> searchAfterShard,
                        std::deque<int64_t> &versions, int numHitsPaged);
  };

public:
  virtual void testSimple() ;

private:
  std::shared_ptr<PreviousSearchState>
  assertSame(std::shared_ptr<IndexSearcher> mockSearcher,
             std::shared_ptr<NodeState::ShardIndexSearcher> shardSearcher,
             std::shared_ptr<Query> q, std::shared_ptr<Sort> sort,
             std::shared_ptr<PreviousSearchState> state) ;

protected:
  std::shared_ptr<TestShardSearching> shared_from_this()
  {
    return std::static_pointer_cast<TestShardSearching>(
        ShardSearchingTestBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
