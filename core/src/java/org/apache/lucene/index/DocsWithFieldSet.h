#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class FixedBitSet;
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
namespace org::apache::lucene::index
{

using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/** Accumulator for documents that have a value for a field. This is optimized
 *  for the case that all documents have a value. */
class DocsWithFieldSet final : public DocIdSet
{
  GET_CLASS_NAME(DocsWithFieldSet)

private:
  static int64_t BASE_RAM_BYTES_USED;

  std::shared_ptr<FixedBitSet> set;
  int cost = 0;
  int lastDocId = -1;

public:
  void add(int docID);

  int64_t ramBytesUsed() override;

  std::shared_ptr<DocIdSetIterator> iterator() override;

protected:
  std::shared_ptr<DocsWithFieldSet> shared_from_this()
  {
    return std::static_pointer_cast<DocsWithFieldSet>(
        org.apache.lucene.search.DocIdSet::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
