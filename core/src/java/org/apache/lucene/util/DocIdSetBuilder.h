#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class FixedBitSet;
}

namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search
{
class DocIdSet;
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

using PointValues = org::apache::lucene::index::PointValues;
using Terms = org::apache::lucene::index::Terms;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

/**
 * A builder of {@link DocIdSet}s.  At first it uses a sparse structure to
 * gather documents, and then upgrades to a non-sparse bit set once enough hits
 * match.
 *
 * To add documents, you first need to call {@link #grow} in order to reserve
 * space, and then call {@link BulkAdder#add(int)} on the returned
 * {@link BulkAdder}.
 *
 * @lucene.internal
 */
class DocIdSetBuilder final
    : public std::enable_shared_from_this<DocIdSetBuilder>
{
  GET_CLASS_NAME(DocIdSetBuilder)

  /** Utility class to efficiently add many docs in one go.
   *  @see DocIdSetBuilder#grow */
public:
  class BulkAdder : public std::enable_shared_from_this<BulkAdder>
  {
    GET_CLASS_NAME(BulkAdder)
  public:
    virtual void add(int doc) = 0;
  };

private:
  class FixedBitSetAdder : public BulkAdder
  {
    GET_CLASS_NAME(FixedBitSetAdder)
  public:
    const std::shared_ptr<FixedBitSet> bitSet;

    FixedBitSetAdder(std::shared_ptr<FixedBitSet> bitSet);

    void add(int doc) override;

  protected:
    std::shared_ptr<FixedBitSetAdder> shared_from_this()
    {
      return std::static_pointer_cast<FixedBitSetAdder>(
          BulkAdder::shared_from_this());
    }
  };

private:
  class Buffer : public std::enable_shared_from_this<Buffer>
  {
    GET_CLASS_NAME(Buffer)
  public:
    std::deque<int> array_;
    int length = 0;

    Buffer(int length);

    Buffer(std::deque<int> &array_, int length);
  };

private:
  class BufferAdder : public BulkAdder
  {
    GET_CLASS_NAME(BufferAdder)
  public:
    const std::shared_ptr<Buffer> buffer;

    BufferAdder(std::shared_ptr<Buffer> buffer);

    void add(int doc) override;

  protected:
    std::shared_ptr<BufferAdder> shared_from_this()
    {
      return std::static_pointer_cast<BufferAdder>(
          BulkAdder::shared_from_this());
    }
  };

private:
  const int maxDoc;
  const int threshold;
  // pkg-private for testing
public:
  const bool multivalued;
  const double numValuesPerDoc;

private:
  std::deque<std::shared_ptr<Buffer>> buffers =
      std::deque<std::shared_ptr<Buffer>>();
  int totalAllocated = 0; // accumulated size of the allocated buffers

  std::shared_ptr<FixedBitSet> bitSet;

  int64_t counter = -1;
  std::shared_ptr<BulkAdder> adder;

  /**
   * Create a builder that can contain doc IDs between {@code 0} and {@code
   * maxDoc}.
   */
public:
  DocIdSetBuilder(int maxDoc);

  /** Create a {@link DocIdSetBuilder} instance that is optimized for
   *  accumulating docs that match the given {@link Terms}. */
  DocIdSetBuilder(int maxDoc, std::shared_ptr<Terms> terms) ;

  /** Create a {@link DocIdSetBuilder} instance that is optimized for
   *  accumulating docs that match the given {@link PointValues}. */
  DocIdSetBuilder(int maxDoc, std::shared_ptr<PointValues> values,
                  const std::wstring &field) ;

  DocIdSetBuilder(int maxDoc, int docCount, int64_t valueCount);

  /**
   * Add the content of the provided {@link DocIdSetIterator} to this builder.
   * NOTE: if you need to build a {@link DocIdSet} out of a single
   * {@link DocIdSetIterator}, you should rather use {@link
   * RoaringDocIdSet.Builder}.
   */
  void add(std::shared_ptr<DocIdSetIterator> iter) ;

  /**
   * Reserve space and return a {@link BulkAdder} object that can be used to
   * add up to {@code numDocs} documents.
   */
  std::shared_ptr<BulkAdder> grow(int numDocs);

private:
  void ensureBufferCapacity(int numDocs);

  int additionalCapacity(int numDocs);

  std::shared_ptr<Buffer> addBuffer(int len);

  void growBuffer(std::shared_ptr<Buffer> buffer, int additionalCapacity);

  void upgradeToBitSet();

  /**
   * Build a {@link DocIdSet} from the accumulated doc IDs.
   */
public:
  std::shared_ptr<DocIdSet> build();

  /**
   * Concatenate the buffers in any order, leaving at least one empty slot in
   * the end
   * NOTE: this method might reuse one of the arrays
   */
private:
  static std::shared_ptr<Buffer>
  concat(std::deque<std::shared_ptr<Buffer>> &buffers);

  static int dedup(std::deque<int> &arr, int length);

  static bool noDups(std::deque<int> &a, int len);
};

} // namespace org::apache::lucene::util
