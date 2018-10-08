#pragma once
#include "../../search/DocIdSetIterator.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexOutput;
}

namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::store
{
class IndexInput;
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
namespace org::apache::lucene::codecs::lucene70
{

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

/**
 * Disk-based implementation of a {@link DocIdSetIterator} which can return
 * the index of the current document, i.e. the ordinal of the current document
 * among the deque of documents that this iterator can return. This is useful
 * to implement sparse doc values by only having to encode values for documents
 * that actually have a value.
 * <p>Implementation-wise, this {@link DocIdSetIterator} is inspired of
 * {@link RoaringDocIdSet roaring bitmaps} and encodes ranges of {@code 65536}
 * documents independently and picks between 3 encodings depending on the
 * density of the range:<ul>
 *   <li>{@code ALL} if the range contains 65536 documents exactly,
 *   <li>{@code DENSE} if the range contains 4096 documents or more; in that
 *       case documents are stored in a bit set,
 *   <li>{@code SPARSE} otherwise, and the lower 16 bits of the doc IDs are
 *       stored in a {@link DataInput#readShort() short}.
 * </ul>
 * <p>Only ranges that contain at least one value are encoded.
 * <p>This implementation uses 6 bytes per document in the worst-case, which
 * happens in the case that all ranges contain exactly one document.
 * @lucene.internal
 */
class IndexedDISI final : public DocIdSetIterator
{
  GET_CLASS_NAME(IndexedDISI)

public:
  static const int MAX_ARRAY_LENGTH = (1 << 12) - 1;

private:
  static void flush(int block, std::shared_ptr<FixedBitSet> buffer,
                    int cardinality,
                    std::shared_ptr<IndexOutput> out) ;

public:
  static void writeBitSet(std::shared_ptr<DocIdSetIterator> it,
                          std::shared_ptr<IndexOutput> out) ;

  /** The slice that stores the {@link DocIdSetIterator}. */
private:
  const std::shared_ptr<IndexInput> slice;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t cost_;

public:
  IndexedDISI(std::shared_ptr<IndexInput> in_, int64_t offset,
              int64_t length, int64_t cost) ;

private:
  int block = -1;
  int64_t blockEnd = 0;
  int nextBlockIndex = -1;

public:
  Method method = static_cast<Method>(0);

private:
  int doc = -1;
  // C++ NOTE: Fields cannot have the same name as methods:
  int index_ = -1;

  // SPARSE variables
public:
  bool exists = false;

  // DENSE variables
private:
  int64_t word = 0;
  int wordIndex = -1;
  // number of one bits encountered so far, including those of `word`
  int numberOfOnes = 0;

  // ALL variables
  int gap = 0;

public:
  int docID() override;

  int advance(int target)  override;

  bool advanceExact(int target) ;

private:
  void advanceBlock(int targetBlock) ;

  void readBlockHeader() ;

public:
  int nextDoc()  override;

  int index();

  int64_t cost() override;

public:
  class Method final
  {
    GET_CLASS_NAME(Method)
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        SPARSE
    //        {
    //            bool advanceWithinBlock(IndexedDISI disi, int target)
    //            throws java.io.IOException
    //            {
    //                final int targetInBlock = target & 0xFFFF;
    //                for(; disi.index < disi.nextBlockIndex;)
    //                {
    //                    int doc = Short.toUnsignedInt(disi.slice.readShort());
    //                    disi.index++;
    //                    if(doc >= targetInBlock)
    //                    {
    //                        disi.doc = disi.block | doc;
    //                        disi.exists = true;
    //                        return true;
    //                    }
    //                }
    //                return false;
    //            }
    //            bool advanceExactWithinBlock(IndexedDISI disi, int target)
    //            throws java.io.IOException
    //            {
    //                final int targetInBlock = target & 0xFFFF;
    //                if(target == disi.doc)
    //                {
    //                    return disi.exists;
    //                }
    //                for(; disi.index < disi.nextBlockIndex;)
    //                {
    //                    int doc = Short.toUnsignedInt(disi.slice.readShort());
    //                    disi.index++;
    //                    if(doc >= targetInBlock)
    //                    {
    //                        if(doc != targetInBlock)
    //                        {
    //                            disi.index--;
    //                            disi.slice.seek(disi.slice.getFilePointer() -
    //                            Short.BYTES); break;
    //                        }
    //                        disi.exists = true;
    //                        return true;
    //                    }
    //                }
    //                disi.exists = false;
    //                return false;
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        DENSE
    //        {
    //            bool advanceWithinBlock(IndexedDISI disi, int target)
    //            throws java.io.IOException
    //            {
    //                final int targetInBlock = target & 0xFFFF;
    //                final int targetWordIndex = targetInBlock >>> 6;
    //                for(int i = disi.wordIndex + 1; i <= targetWordIndex; ++i)
    //                {
    //                    disi.word = disi.slice.readLong();
    //                    disi.numberOfOnes += Long.bitCount(disi.word);
    //                }
    //                disi.wordIndex = targetWordIndex;
    //                long leftBits = disi.word >>> target;
    //                if(leftBits != 0L)
    //                {
    //                    disi.doc = target +
    //                    Long.numberOfTrailingZeros(leftBits); disi.index =
    //                    disi.numberOfOnes - Long.bitCount(leftBits); return
    //                    true;
    //                } while (++disi.wordIndex < 1024)
    //                {
    //                    disi.word = disi.slice.readLong();
    //                    if(disi.word != 0)
    //                    {
    //                        disi.index = disi.numberOfOnes;
    //                        disi.numberOfOnes += Long.bitCount(disi.word);
    //                        disi.doc = disi.block | (disi.wordIndex << 6) |
    //                        Long.numberOfTrailingZeros(disi.word); return
    //                        true;
    //                    }
    //                }
    //                return false;
    //            }
    //            bool advanceExactWithinBlock(IndexedDISI disi, int target)
    //            throws java.io.IOException
    //            {
    //                final int targetInBlock = target & 0xFFFF;
    //                final int targetWordIndex = targetInBlock >>> 6;
    //                for(int i = disi.wordIndex + 1; i <= targetWordIndex; ++i)
    //                {
    //                    disi.word = disi.slice.readLong();
    //                    disi.numberOfOnes += Long.bitCount(disi.word);
    //                }
    //                disi.wordIndex = targetWordIndex;
    //                long leftBits = disi.word >>> target;
    //                disi.index = disi.numberOfOnes - Long.bitCount(leftBits);
    //                return (leftBits & 1L) != 0;
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        ALL
    //        {
    //            bool advanceWithinBlock(IndexedDISI disi, int target)
    //            throws java.io.IOException
    //            {
    //                disi.doc = target;
    //                disi.index = target - disi.gap;
    //                return true;
    //            }
    //            bool advanceExactWithinBlock(IndexedDISI disi, int target)
    //            throws java.io.IOException
    //            {
    //                disi.index = target - disi.gap;
    //                return true;
    //            }
    //        };

  private:
    static std::deque<Method> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum { GET_CLASS_NAME(InnerEnum) };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

    Method(const std::wstring &name, InnerEnum innerEnum);

    /** Advance to the first doc from the block that is equal to or greater than
     * {@code target}. Return true if there is such a doc and false otherwise.
     */
  public:
    virtual bool advanceWithinBlock(std::shared_ptr<IndexedDISI> disi,
                                    int target) = 0;

    /** Advance the iterator exactly to the position corresponding to the given
     * {@code target} and return whether this document exists. */
    virtual bool advanceExactWithinBlock(std::shared_ptr<IndexedDISI> disi,
                                         int target) = 0;

  public:
    bool operator==(const Method &other);

    bool operator!=(const Method &other);

    static std::deque<Method> values();

    int ordinal();

    std::wstring toString();

    static Method valueOf(const std::wstring &name);
  };

protected:
  std::shared_ptr<IndexedDISI> shared_from_this()
  {
    return std::static_pointer_cast<IndexedDISI>(
        org.apache.lucene.search.DocIdSetIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene70
