#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/CacheKey.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSet.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/util/BitSet.h"

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
namespace org::apache::lucene::search::join
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using Query = org::apache::lucene::search::Query;
using BitSet = org::apache::lucene::util::BitSet;

/**
 * A {@link BitSetProducer} that wraps a query and caches matching
 * {@link BitSet}s per segment.
 */
class QueryBitSetProducer
    : public std::enable_shared_from_this<QueryBitSetProducer>,
      public BitSetProducer
{
  GET_CLASS_NAME(QueryBitSetProducer)
private:
  const std::shared_ptr<Query> query;

public:
  const std::unordered_map<std::shared_ptr<IndexReader::CacheKey>,
                           std::shared_ptr<DocIdSet>>
      cache = Collections::synchronizedMap(
          std::make_shared<WeakHashMap<std::shared_ptr<IndexReader::CacheKey>,
                                       std::shared_ptr<DocIdSet>>>());

  /** Wraps another query's result and caches it into bitsets.
   * @param query Query to cache results of
   */
  QueryBitSetProducer(std::shared_ptr<Query> query);

  /**
   * Gets the contained query.
   * @return the contained query.
   */
  virtual std::shared_ptr<Query> getQuery();

  std::shared_ptr<BitSet> getBitSet(
      std::shared_ptr<LeafReaderContext> context)  override;

  virtual std::wstring toString();

  virtual bool equals(std::any o);

  virtual int hashCode();
};

} // #include  "core/src/java/org/apache/lucene/search/join/
