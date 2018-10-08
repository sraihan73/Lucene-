#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}

namespace org::apache::lucene::util
{
class Counter;
}
namespace org::apache::lucene::util
{
class BytesRefHash;
}
namespace org::apache::lucene::facet::taxonomy
{
class FacetLabel;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::facet::taxonomy::writercache
{

using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache;
using Accountable = org::apache::lucene::util::Accountable;
using DirectTrackingAllocator =
    org::apache::lucene::util::ByteBlockPool::DirectTrackingAllocator;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using Counter = org::apache::lucene::util::Counter;

/** A "cache" that never frees memory, and stores labels in a BytesRefHash
 * (utf-8 encoding). */
class UTF8TaxonomyWriterCache final
    : public std::enable_shared_from_this<UTF8TaxonomyWriterCache>,
      public TaxonomyWriterCache,
      public Accountable
{
  GET_CLASS_NAME(UTF8TaxonomyWriterCache)
private:
  const std::shared_ptr<ThreadLocal<std::shared_ptr<BytesRefBuilder>>> bytes =
      std::make_shared<ThreadLocalAnonymousInnerClass>();

private:
  class ThreadLocalAnonymousInnerClass
      : public ThreadLocal<std::shared_ptr<BytesRefBuilder>>
  {
    GET_CLASS_NAME(ThreadLocalAnonymousInnerClass)
  public:
    ThreadLocalAnonymousInnerClass();

  protected:
    std::shared_ptr<BytesRefBuilder> initialValue() override;

  protected:
    std::shared_ptr<ThreadLocalAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadLocalAnonymousInnerClass>(
          ThreadLocal<
              org.apache.lucene.util.BytesRefBuilder>::shared_from_this());
    }
  };

private:
  const std::shared_ptr<Counter> bytesUsed = Counter::newCounter();
  const std::shared_ptr<BytesRefHash> map_obj =
      std::make_shared<BytesRefHash>(std::make_shared<ByteBlockPool>(
          std::make_shared<DirectTrackingAllocator>(bytesUsed)));

  static constexpr int ORDINALS_PAGE_SIZE = 65536;
  static const int ORDINALS_PAGE_MASK = ORDINALS_PAGE_SIZE - 1;

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile int[][] ordinals;
  std::deque<std::deque<int>> ordinals;

  // How many labels we are storing:
  int count = 0;

  // How many pages in ordinals we've allocated:
  int pageCount = 0;

  /** Sole constructor. */
public:
  UTF8TaxonomyWriterCache();

  int get(std::shared_ptr<FacetLabel> label) override;

  // Called only from assert
private:
  bool assertSameOrdinal(std::shared_ptr<FacetLabel> label, int id, int ord);

public:
  bool put(std::shared_ptr<FacetLabel> label, int ord) override;

  bool isFull() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void clear() override;

  /** How many labels are currently stored in the cache. */
  int size() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t ramBytesUsed() override;

  virtual ~UTF8TaxonomyWriterCache();

private:
  static const char DELIM_CHAR = static_cast<char>(0x1F);

  std::shared_ptr<BytesRef> toBytes(std::shared_ptr<FacetLabel> label);
};

} // namespace org::apache::lucene::facet::taxonomy::writercache
