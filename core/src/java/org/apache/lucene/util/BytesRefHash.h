#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/ByteBlockPool.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/BytesStartArray.h"
#include  "core/src/java/org/apache/lucene/util/Counter.h"

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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_MASK;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_SHIFT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_SIZE;

/**
 * {@link BytesRefHash} is a special purpose hash-map_obj like data-structure
 * optimized for {@link BytesRef} instances. BytesRefHash maintains mappings of
 * byte arrays to ids (Map&lt;BytesRef,int&gt;) storing the hashed bytes
 * efficiently in continuous storage. The mapping to the id is
 * encapsulated inside {@link BytesRefHash} and is guaranteed to be increased
 * for each added {@link BytesRef}.
 *
 * <p>
 * Note: The maximum capacity {@link BytesRef} instance passed to
 * {@link #add(BytesRef)} must not be longer than {@link
 * ByteBlockPool#BYTE_BLOCK_SIZE}-2. The internal storage is limited to 2GB
 * total byte storage.
 * </p>
 *
 * @lucene.internal
 */
class BytesRefHash final : public std::enable_shared_from_this<BytesRefHash>
{
  GET_CLASS_NAME(BytesRefHash)

public:
  static constexpr int DEFAULT_CAPACITY = 16;

  // the following fields are needed by comparator,
  // so package private to prevent access$-methods:
  const std::shared_ptr<ByteBlockPool> pool;
  std::deque<int> bytesStart;

private:
  const std::shared_ptr<BytesRef> scratch1 = std::make_shared<BytesRef>();
  int hashSize = 0;
  int hashHalfSize = 0;
  int hashMask = 0;
  int count = 0;
  int lastCount = -1;
  std::deque<int> ids;
  const std::shared_ptr<BytesStartArray> bytesStartArray;
  std::shared_ptr<Counter> bytesUsed;

  /**
   * Creates a new {@link BytesRefHash} with a {@link ByteBlockPool} using a
   * {@link DirectAllocator}.
   */
public:
  BytesRefHash();

  /**
   * Creates a new {@link BytesRefHash}
   */
  BytesRefHash(std::shared_ptr<ByteBlockPool> pool);

  /**
   * Creates a new {@link BytesRefHash}
   */
  BytesRefHash(std::shared_ptr<ByteBlockPool> pool, int capacity,
               std::shared_ptr<BytesStartArray> bytesStartArray);

  /**
   * Returns the number of {@link BytesRef} values in this {@link BytesRefHash}.
   *
   * @return the number of {@link BytesRef} values in this {@link BytesRefHash}.
   */
  int size();

  /**
   * Populates and returns a {@link BytesRef} with the bytes for the given
   * bytesID.
   * <p>
   * Note: the given bytesID must be a positive integer less than the current
   * size ({@link #size()})
   *
   * @param bytesID
   *          the id
   * @param ref
   *          the {@link BytesRef} to populate
   *
   * @return the given BytesRef instance populated with the bytes for the given
   *         bytesID
   */
  std::shared_ptr<BytesRef> get(int bytesID, std::shared_ptr<BytesRef> ref);

  /**
   * Returns the ids array in arbitrary order. Valid ids start at offset of 0
   * and end at a limit of {@link #size()} - 1
   * <p>
   * Note: This is a destructive operation. {@link #clear()} must be called in
   * order to reuse this {@link BytesRefHash} instance.
   * </p>
   */
  std::deque<int> compact();

  /**
   * Returns the values array sorted by the referenced byte values.
   * <p>
   * Note: This is a destructive operation. {@link #clear()} must be called in
   * order to reuse this {@link BytesRefHash} instance.
   * </p>
   */
  std::deque<int> sort();

private:
  class StringMSBRadixSorterAnonymousInnerClass : public StringMSBRadixSorter
  {
    GET_CLASS_NAME(StringMSBRadixSorterAnonymousInnerClass)
  private:
    std::shared_ptr<BytesRefHash> outerInstance;

    std::deque<int> compact;

  public:
    StringMSBRadixSorterAnonymousInnerClass(
        std::shared_ptr<BytesRefHash> outerInstance, std::deque<int> &compact);

    std::shared_ptr<BytesRef> scratch;

  protected:
    void swap(int i, int j) override;

    std::shared_ptr<BytesRef> get(int i) override;

  protected:
    std::shared_ptr<StringMSBRadixSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StringMSBRadixSorterAnonymousInnerClass>(
          StringMSBRadixSorter::shared_from_this());
    }
  };

private:
  bool equals(int id, std::shared_ptr<BytesRef> b);

  bool shrink(int targetSize);

  /**
   * Clears the {@link BytesRef} which maps to the given {@link BytesRef}
   */
public:
  void clear(bool resetPool);

  void clear();

  /**
   * Closes the BytesRefHash and releases all internally used memory
   */
  virtual ~BytesRefHash();

  /**
   * Adds a new {@link BytesRef}
   *
   * @param bytes
   *          the bytes to hash
   * @return the id the given bytes are hashed if there was no mapping for the
   *         given bytes, otherwise <code>(-(id)-1)</code>. This guarantees
   *         that the return value will always be &gt;= 0 if the given bytes
   *         haven't been hashed before.
   *
   * @throws MaxBytesLengthExceededException
   *           if the given bytes are {@code > 2 +}
   *           {@link ByteBlockPool#BYTE_BLOCK_SIZE}
   */
  int add(std::shared_ptr<BytesRef> bytes);

  /**
   * Returns the id of the given {@link BytesRef}.
   *
   * @param bytes
   *          the bytes to look for
   *
   * @return the id of the given bytes, or {@code -1} if there is no mapping for
   * the given bytes.
   */
  int find(std::shared_ptr<BytesRef> bytes);

private:
  int findHash(std::shared_ptr<BytesRef> bytes);

  /** Adds a "arbitrary" int offset instead of a BytesRef
   *  term.  This is used in the indexer to hold the hash for term
   *  vectors, because they do not redundantly store the byte[] term
   *  directly and instead reference the byte[] term
   *  already stored by the postings BytesRefHash.  See
   *  add(int textStart) in TermsHashPerField. */
public:
  int addByPoolOffset(int offset);

  /**
   * Called when hash is too small ({@code > 50%} occupied) or too large ({@code
   * < 20%} occupied).
   */
private:
  void rehash(int const newSize, bool hashOnData);

  // TODO: maybe use long?  But our keys are typically short...
  int doHash(std::deque<char> &bytes, int offset, int length);

  /**
   * reinitializes the {@link BytesRefHash} after a previous {@link #clear()}
   * call. If {@link #clear()} has not been called previously this method has no
   * effect.
   */
public:
  void reinit();

  /**
   * Returns the bytesStart offset into the internally used
   * {@link ByteBlockPool} for the given bytesID
   *
   * @param bytesID
   *          the id to look up
   * @return the bytesStart offset into the internally used
   *         {@link ByteBlockPool} for the given id
   */
  int byteStart(int bytesID);

  /**
   * Thrown if a {@link BytesRef} exceeds the {@link BytesRefHash} limit of
   * {@link ByteBlockPool#BYTE_BLOCK_SIZE}-2.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("serial") public static class
  // MaxBytesLengthExceededException extends RuntimeException
  class MaxBytesLengthExceededException : public std::runtime_error
  {
  public:
    MaxBytesLengthExceededException(const std::wstring &message);

  protected:
    std::shared_ptr<MaxBytesLengthExceededException> shared_from_this()
    {
      return std::static_pointer_cast<MaxBytesLengthExceededException>(
          RuntimeException::shared_from_this());
    }
  };

  /** Manages allocation of the per-term addresses. */
public:
  class BytesStartArray : public std::enable_shared_from_this<BytesStartArray>
  {
    GET_CLASS_NAME(BytesStartArray)
    /**
     * Initializes the BytesStartArray. This call will allocate memory
     *
     * @return the initialized bytes start array
     */
  public:
    virtual std::deque<int> init() = 0;

    /**
     * Grows the {@link BytesStartArray}
     *
     * @return the grown array
     */
    virtual std::deque<int> grow() = 0;

    /**
     * clears the {@link BytesStartArray} and returns the cleared instance.
     *
     * @return the cleared instance, this might be <code>null</code>
     */
    virtual std::deque<int> clear() = 0;

    /**
     * A {@link Counter} reference holding the number of bytes used by this
     * {@link BytesStartArray}. The {@link BytesRefHash} uses this reference to
     * track it memory usage
     *
     * @return a {@link AtomicLong} reference holding the number of bytes used
     *         by this {@link BytesStartArray}.
     */
    virtual std::shared_ptr<Counter> bytesUsed() = 0;
  };

  /** A simple {@link BytesStartArray} that tracks
   *  memory allocation using a private {@link Counter}
   *  instance.  */
public:
  class DirectBytesStartArray : public BytesStartArray
  {
    GET_CLASS_NAME(DirectBytesStartArray)
    // TODO: can't we just merge this w/
    // TrackingDirectBytesStartArray...?  Just add a ctor
    // that makes a private bytesUsed?

  protected:
    const int initSize;

  private:
    std::deque<int> bytesStart;
    // C++ NOTE: Fields cannot have the same name as methods:
    const std::shared_ptr<Counter> bytesUsed_;

  public:
    DirectBytesStartArray(int initSize, std::shared_ptr<Counter> counter);

    DirectBytesStartArray(int initSize);

    std::deque<int> clear() override;

    std::deque<int> grow() override;

    std::deque<int> init() override;

    std::shared_ptr<Counter> bytesUsed() override;

  protected:
    std::shared_ptr<DirectBytesStartArray> shared_from_this()
    {
      return std::static_pointer_cast<DirectBytesStartArray>(
          BytesStartArray::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/util/
