#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSet;
}

namespace org::apache::lucene::util
{
class Bits;
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
 * This {@link DocIdSet} encodes the negation of another {@link DocIdSet}.
 * It is cacheable and supports random-access if the underlying set is
 * cacheable and supports random-access.
 * @lucene.internal
 */
class NotDocIdSet final : public DocIdSet
{
  GET_CLASS_NAME(NotDocIdSet)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(NotDocIdSet::typeid);

  const int maxDoc;
  const std::shared_ptr<DocIdSet> in_;

  /** Sole constructor. */
public:
  NotDocIdSet(int maxDoc, std::shared_ptr<DocIdSet> in_);

  std::shared_ptr<Bits> bits()  override;

private:
  class BitsAnonymousInnerClass
      : public std::enable_shared_from_this<BitsAnonymousInnerClass>,
        public Bits
  {
    GET_CLASS_NAME(BitsAnonymousInnerClass)
  private:
    std::shared_ptr<NotDocIdSet> outerInstance;

    std::shared_ptr<org::apache::lucene::util::Bits> inBits;

  public:
    BitsAnonymousInnerClass(
        std::shared_ptr<NotDocIdSet> outerInstance,
        std::shared_ptr<org::apache::lucene::util::Bits> inBits);

    bool get(int index) override;

    int length() override;
  };

public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<DocIdSetIterator> iterator()  override;

private:
  class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
  {
    GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<NotDocIdSet> outerInstance;

    std::shared_ptr<DocIdSetIterator> inIterator;

  public:
    DocIdSetIteratorAnonymousInnerClass(
        std::shared_ptr<NotDocIdSet> outerInstance,
        std::shared_ptr<DocIdSetIterator> inIterator);

    int doc = 0;
    int nextSkippedDoc = 0;

    int nextDoc()  override;

    int advance(int target)  override;

    int docID() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
          org.apache.lucene.search.DocIdSetIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<NotDocIdSet> shared_from_this()
  {
    return std::static_pointer_cast<NotDocIdSet>(
        org.apache.lucene.search.DocIdSet::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
