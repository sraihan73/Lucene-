#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BitSet;
}

namespace org::apache::lucene::search
{
class DocIdSetIterator;
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
namespace org::apache::lucene::util
{

using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

/**
 * Implementation of the {@link DocIdSet} interface on top of a {@link BitSet}.
 * @lucene.internal
 */
class BitDocIdSet : public DocIdSet
{
  GET_CLASS_NAME(BitDocIdSet)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(BitDocIdSet::typeid);

  const std::shared_ptr<BitSet> set;
  const int64_t cost;

  /**
   * Wrap the given {@link BitSet} as a {@link DocIdSet}. The provided
   * {@link BitSet} must not be modified afterwards.
   */
public:
  BitDocIdSet(std::shared_ptr<BitSet> set, int64_t cost);

  /**
   * Same as {@link #BitDocIdSet(BitSet, long)} but uses the set's
   * {@link BitSet#approximateCardinality() approximate cardinality} as a cost.
   */
  BitDocIdSet(std::shared_ptr<BitSet> set);

  std::shared_ptr<DocIdSetIterator> iterator() override;

  std::shared_ptr<BitSet> bits() override;

  int64_t ramBytesUsed() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<BitDocIdSet> shared_from_this()
  {
    return std::static_pointer_cast<BitDocIdSet>(
        org.apache.lucene.search.DocIdSet::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
