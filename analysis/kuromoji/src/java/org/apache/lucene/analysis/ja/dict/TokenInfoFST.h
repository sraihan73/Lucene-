#pragma once
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}

namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
}
#include  "core/src/java/org/apache/lucene/util/fst/BytesReader.h"

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
namespace org::apache::lucene::analysis::ja::dict
{

using org::apache::lucene::util::fst::FST::Arc;
using FST = org::apache::lucene::util::fst::FST;

/**
 * Thin wrapper around an FST with root-arc caching for Japanese.
 * <p>
 * Depending upon fasterButMoreRam, either just kana (191 arcs),
 * or kana and han (28,607 arcs) are cached. The latter offers
 * additional performance at the cost of more RAM.
 */
class TokenInfoFST final : public std::enable_shared_from_this<TokenInfoFST>
{
  GET_CLASS_NAME(TokenInfoFST)
private:
  const std::shared_ptr<FST<int64_t>> fst;

  // depending upon fasterButMoreRam, we cache root arcs for either
  // kana (0x3040-0x30FF) or kana + han (0x3040-0x9FFF)
  // false: 191 arcs
  // true:  28,607 arcs (costs ~1.5MB)
  const int cacheCeiling;
  std::deque<Arc<int64_t>> const rootCache;

public:
  const std::optional<int64_t> NO_OUTPUT;

  TokenInfoFST(std::shared_ptr<FST<int64_t>> fst,
               bool fasterButMoreRam) ;

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) private
  // org.apache.lucene.util.fst.FST.Arc<long>[] cacheRootArcs() throws
  // java.io.IOException
  std::deque<Arc<int64_t>> cacheRootArcs() ;

public:
  std::shared_ptr<Arc<int64_t>>
  findTargetArc(int ch, std::shared_ptr<Arc<int64_t>> follow,
                std::shared_ptr<Arc<int64_t>> arc, bool useCache,
                std::shared_ptr<FST::BytesReader> fstReader) ;

  std::shared_ptr<Arc<int64_t>>
  getFirstArc(std::shared_ptr<Arc<int64_t>> arc);

  std::shared_ptr<FST::BytesReader> getBytesReader();

  /** @lucene.internal for testing only */
  std::shared_ptr<FST<int64_t>> getInternalFST();
};

} // #include  "core/src/java/org/apache/lucene/analysis/ja/dict/
