#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/DocIdSet.h"

#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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
 * {@link DocIdSet} implementation inspired from http://roaringbitmap.org/
 *
 * The space is divided into blocks of 2^16 bits and each block is encoded
 * independently. In each block, if less than 2^12 bits are set, then
 * documents are simply stored in a short[]. If more than 2^16-2^12 bits are
 * set, then the inverse of the set is encoded in a simple short[]. Otherwise
 * a {@link FixedBitSet} is used.
 *
 * @lucene.internal
 */
class RoaringDocIdSet : public DocIdSet
{
  GET_CLASS_NAME(RoaringDocIdSet)

  // Number of documents in a block
private:
  static const int BLOCK_SIZE = 1 << 16;
  // The maximum length for an array, beyond that point we switch to a bitset
  static const int MAX_ARRAY_LENGTH = 1 << 12;
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(RoaringDocIdSet::typeid);

  /** A builder of {@link RoaringDocIdSet}s. */
public:
  class Builder : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)

  private:
    const int maxDoc;
    std::deque<std::shared_ptr<DocIdSet>> const sets;

    int cardinality = 0;
    int lastDocId = 0;
    int currentBlock = 0;
    int currentBlockCardinality = 0;

    // We start by filling the buffer and when it's full we copy the content of
    // the buffer to the FixedBitSet and put further documents in that bitset
    std::deque<short> const buffer;
    std::shared_ptr<FixedBitSet> denseBuffer;

    /** Sole constructor. */
  public:
    Builder(int maxDoc);

  private:
    void flush();

    /**
     * Add a new doc-id to this builder.
     * NOTE: doc ids must be added in order.
     */
  public:
    virtual std::shared_ptr<Builder> add(int docId);

    /** Add the content of the provided {@link DocIdSetIterator}. */
    virtual std::shared_ptr<Builder>
    add(std::shared_ptr<DocIdSetIterator> disi) ;

    /** Build an instance. */
    virtual std::shared_ptr<RoaringDocIdSet> build();
  };

  /**
   * {@link DocIdSet} implementation that can store documents up to 2^16-1 in a
   * short[].
   */
private:
  class ShortArrayDocIdSet : public DocIdSet
  {
    GET_CLASS_NAME(ShortArrayDocIdSet)

  private:
    static const int64_t BASE_RAM_BYTES_USED =
        RamUsageEstimator::shallowSizeOfInstance(ShortArrayDocIdSet::typeid);

    std::deque<short> const docIDs;

    ShortArrayDocIdSet(std::deque<short> &docIDs);

  public:
    int64_t ramBytesUsed() override;

    std::shared_ptr<DocIdSetIterator> iterator()  override;

  private:
    class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
    {
      GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<ShortArrayDocIdSet> outerInstance;

    public:
      DocIdSetIteratorAnonymousInnerClass(
          std::shared_ptr<ShortArrayDocIdSet> outerInstance);

      int i = 0;
      int doc = 0;

    private:
      int docId(int i);

    public:
      int nextDoc()  override;

      int docID() override;

      int64_t cost() override;

      int advance(int target)  override;

    protected:
      std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
            org.apache.lucene.search.DocIdSetIterator::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<ShortArrayDocIdSet> shared_from_this()
    {
      return std::static_pointer_cast<ShortArrayDocIdSet>(
          org.apache.lucene.search.DocIdSet::shared_from_this());
    }
  };

private:
  std::deque<std::shared_ptr<DocIdSet>> const docIdSets;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int cardinality_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t ramBytesUsed_;

  RoaringDocIdSet(std::deque<std::shared_ptr<DocIdSet>> &docIdSets,
                  int cardinality);

public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<DocIdSetIterator> iterator()  override;

private:
  class Iterator : public DocIdSetIterator
  {
    GET_CLASS_NAME(Iterator)
  private:
    std::shared_ptr<RoaringDocIdSet> outerInstance;

  public:
    int block = 0;
    std::shared_ptr<DocIdSetIterator> sub = nullptr;
    int doc = 0;

    Iterator(std::shared_ptr<RoaringDocIdSet> outerInstance) ;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

  private:
    int firstDocFromNextBlock() ;

  public:
    int64_t cost() override;

  protected:
    std::shared_ptr<Iterator> shared_from_this()
    {
      return std::static_pointer_cast<Iterator>(
          org.apache.lucene.search.DocIdSetIterator::shared_from_this());
    }
  };

  /** Return the exact number of documents that are contained in this set. */
public:
  virtual int cardinality();

  virtual std::wstring toString();

protected:
  std::shared_ptr<RoaringDocIdSet> shared_from_this()
  {
    return std::static_pointer_cast<RoaringDocIdSet>(
        org.apache.lucene.search.DocIdSet::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
