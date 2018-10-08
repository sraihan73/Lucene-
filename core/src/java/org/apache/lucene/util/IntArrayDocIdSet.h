#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
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

class IntArrayDocIdSet final : public DocIdSet
{
  GET_CLASS_NAME(IntArrayDocIdSet)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(IntArrayDocIdSet::typeid);

  std::deque<int> const docs;
  const int length;

public:
  IntArrayDocIdSet(std::deque<int> &docs, int length);

  int64_t ramBytesUsed() override;

  std::shared_ptr<DocIdSetIterator> iterator()  override;

public:
  class IntArrayDocIdSetIterator : public DocIdSetIterator
  {
    GET_CLASS_NAME(IntArrayDocIdSetIterator)

  private:
    std::deque<int> const docs;
    const int length;
    int i = -1;
    int doc = -1;

  public:
    IntArrayDocIdSetIterator(std::deque<int> &docs, int length);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<IntArrayDocIdSetIterator> shared_from_this()
    {
      return std::static_pointer_cast<IntArrayDocIdSetIterator>(
          org.apache.lucene.search.DocIdSetIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<IntArrayDocIdSet> shared_from_this()
  {
    return std::static_pointer_cast<IntArrayDocIdSet>(
        org.apache.lucene.search.DocIdSet::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
