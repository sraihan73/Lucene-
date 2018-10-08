#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class LongsRef;
}

namespace org::apache::lucene::store
{
class DataInput;
}
namespace org::apache::lucene::store
{
class DataOutput;
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
namespace org::apache::lucene::util::packed
{

using LegacyNumericDocValues =
    org::apache::lucene::index::LegacyNumericDocValues;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using LongsRef = org::apache::lucene::util::LongsRef;

/**
 * Simplistic compression for array of unsigned long values.
 * Each value is {@code >= 0} and {@code <=} a specified maximum value.  The
 * values are stored as packed ints, with each value
 * consuming a fixed number of bits.
 *
 * @lucene.internal
 */
class PackedInts : public std::enable_shared_from_this<PackedInts>
{
  GET_CLASS_NAME(PackedInts)

  /**
   * At most 700% memory overhead, always select a direct implementation.
   */
public:
  static constexpr float FASTEST = 7.0f;

  /**
   * At most 50% memory overhead, always select a reasonably fast
   * implementation.
   */
  static constexpr float FAST = 0.5f;

  /**
   * At most 25% memory overhead.
   */
  static constexpr float DEFAULT = 0.25f;

  /**
   * No memory overhead at all, but the returned implementation may be slow.
   */
  static constexpr float COMPACT = 0.0f;

  /**
   * Default amount of memory to use for bulk operations.
   */
  static constexpr int DEFAULT_BUFFER_SIZE = 1024; // 1K

  static const std::wstring CODEC_NAME;
  static constexpr int VERSION_MONOTONIC_WITHOUT_ZIGZAG = 2;
  static constexpr int VERSION_START = VERSION_MONOTONIC_WITHOUT_ZIGZAG;
  static constexpr int VERSION_CURRENT = VERSION_MONOTONIC_WITHOUT_ZIGZAG;

  /**
   * Check the validity of a version number.
   */
  static void checkVersion(int version);

  /**
   * A format to write packed ints.
   *
   * @lucene.internal
   */
public:
  enum class Format {
    GET_CLASS_NAME(Format)
    /**
     * Compact format, all bits are written contiguously.
     */
    public :
        // C++ TODO: Most Java annotations will not have direct C++ equivalents:
        // ORIGINAL LINE: PACKED(0) { @Override public long byteCount(int
        // packedIntsVersion, int valueCount, int bitsPerValue) { return(long)
        // Math.ceil((double) valueCount * bitsPerValue / 8); } },
            PACKED(0){int64_t byteCount(int packedIntsVersion, int valueCount,
                                          int bitsPerValue){
                returnsafe_cast<int64_t>(std::ceil(
                    static_cast<double>(valueCount) * bitsPerValue / 8));}
},

    /**
     * A format that may insert padding bits to improve encoding and decoding
     * speed. Since this format doesn't support all possible bits per value, you
     * should never use it directly, but rather use
     * {@link PackedInts#fastestFormatAndBits(int, int, float)} to find the
     * format that best suits your needs.
     */
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: PACKED_SINGLE_BLOCK(1) { @Override public int
    // longCount(int packedIntsVersion, int valueCount, int bitsPerValue) { final
    // int valuesPerBlock = 64 / bitsPerValue; return(int) Math.ceil((double)
    // valueCount / valuesPerBlock); } @Override public bool isSupported(int
    // bitsPerValue) { return Packed64SingleBlock.isSupported(bitsPerValue); }
    // @Override public float overheadPerValue(int bitsPerValue) { assert
    // isSupported(bitsPerValue); final int valuesPerBlock = 64 / bitsPerValue;
    // final int overhead = 64%bitsPerValue; return(float) overhead /
    // valuesPerBlock; } };
    PACKED_SINGLE_BLOCK(1){
        int longCount(int packedIntsVersion, int valueCount, int bitsPerValue){
            final int valuesPerBlock = 64 / bitsPerValue;
returnsafe_cast<int>(std::ceil(static_cast<double>(valueCount) /
                               valuesPerBlock));
} // namespace org::apache::lucene::util::packed
public
bool isSupported(int bitsPerValue)
{
  return Packed64SingleBlock::isSupported(bitsPerValue);
}
public
float overheadPerValue(int bitsPerValue)
{
  assert isSupported(bitsPerValue);
  final int valuesPerBlock = 64 / bitsPerValue;
  final int overhead = 64 % bitsPerValue;
  returnsafe_cast<float>(overhead) / valuesPerBlock;
}
}
;

/**
 * Get a format according to its ID.
 */
static Format byId(int id)
{
  for (Format format : Format::values()) {
    if (format::getId() == id) {
      return format;
    }
  }
  throw std::invalid_argument("Unknown format id: " + id);
}
private
Format(int id) { this->id = id; }
public
int id;

/**
 * Returns the ID of the format.
 */
int getId() { return id; }
public
int64_t byteCount(int packedIntsVersion, int valueCount, int bitsPerValue)
{
  assert bitsPerValue >= 0 && bitsPerValue <= 64 : bitsPerValue;
  return 8LL * longCount(packedIntsVersion, valueCount, bitsPerValue);
}
public
int longCount(int packedIntsVersion, int valueCount, int bitsPerValue)
{
  assert bitsPerValue >= 0 && bitsPerValue <= 64 : bitsPerValue;
  final int64_t byteCount =
      byteCount(packedIntsVersion, valueCount, bitsPerValue);
  assert byteCount < 8LL * std::numeric_limits<int>::max();
  if ((byteCount % 8) == 0) {
    returnsafe_cast<int>(byteCount / 8);
  } else {
    returnsafe_cast<int>(byteCount / 8 + 1);
  }
}
public
bool isSupported(int bitsPerValue)
{
  return bitsPerValue >= 1 && bitsPerValue <= 64;
}
public
float overheadPerValue(int bitsPerValue)
{
  assert isSupported(bitsPerValue);
  return 0.0f;
}
public
final float overheadRatio(int bitsPerValue)
{
  assert isSupported(bitsPerValue);
  return overheadPerValue(bitsPerValue) / bitsPerValue;
}
}
;

/**
 * Simple class that holds a format and a number of bits per value.
 */
public:
class FormatAndBits : public std::enable_shared_from_this<FormatAndBits>
{
  GET_CLASS_NAME(FormatAndBits)
public:
  const Format format;
  const int bitsPerValue;
  FormatAndBits(Format format, int bitsPerValue);

  virtual std::wstring toString();
};

/**
 * Try to find the {@link Format} and number of bits per value that would
 * restore from disk the fastest reader whose overhead is less than
 * <code>acceptableOverheadRatio</code>.
 * <p>
 * The <code>acceptableOverheadRatio</code> parameter makes sense for
 * random-access {@link Reader}s. In case you only plan to perform
 * sequential access on this stream later on, you should probably use
 * {@link PackedInts#COMPACT}.
 * <p>
 * If you don't know how many values you are going to write, use
 * <code>valueCount = -1</code>.
 */
public:
static std::shared_ptr<FormatAndBits>
fastestFormatAndBits(int valueCount, int bitsPerValue,
                     float acceptableOverheadRatio);

/**
 * A decoder for packed integers.
 */
public:
class Decoder
{
  GET_CLASS_NAME(Decoder)

  /**
   * The minimum number of long blocks to encode in a single iteration, when
   * using long encoding.
   */
public:
  virtual int longBlockCount() = 0;

  /**
   * The number of values that can be stored in {@link #longBlockCount()} long
   * blocks.
   */
  virtual int longValueCount() = 0;

  /**
   * The minimum number of byte blocks to encode in a single iteration, when
   * using byte encoding.
   */
  virtual int byteBlockCount() = 0;

  /**
   * The number of values that can be stored in {@link #byteBlockCount()} byte
   * blocks.
   */
  virtual int byteValueCount() = 0;

  /**
   * Read <code>iterations * blockCount()</code> blocks from
   * <code>blocks</code>, decode them and write <code>iterations *
   * valueCount()</code> values into <code>values</code>.
   *
   * @param blocks       the long blocks that hold packed integer values
   * @param blocksOffset the offset where to start reading blocks
   * @param values       the values buffer
   * @param valuesOffset the offset where to start writing values
   * @param iterations   controls how much data to decode
   */
  virtual void decode(std::deque<int64_t> &blocks, int blocksOffset,
                      std::deque<int64_t> &values, int valuesOffset,
                      int iterations) = 0;

  /**
   * Read <code>8 * iterations * blockCount()</code> blocks from
   * <code>blocks</code>, decode them and write <code>iterations *
   * valueCount()</code> values into <code>values</code>.
   *
   * @param blocks       the long blocks that hold packed integer values
   * @param blocksOffset the offset where to start reading blocks
   * @param values       the values buffer
   * @param valuesOffset the offset where to start writing values
   * @param iterations   controls how much data to decode
   */
  virtual void decode(std::deque<char> &blocks, int blocksOffset,
                      std::deque<int64_t> &values, int valuesOffset,
                      int iterations) = 0;

  /**
   * Read <code>iterations * blockCount()</code> blocks from
   * <code>blocks</code>, decode them and write <code>iterations *
   * valueCount()</code> values into <code>values</code>.
   *
   * @param blocks       the long blocks that hold packed integer values
   * @param blocksOffset the offset where to start reading blocks
   * @param values       the values buffer
   * @param valuesOffset the offset where to start writing values
   * @param iterations   controls how much data to decode
   */
  virtual void decode(std::deque<int64_t> &blocks, int blocksOffset,
                      std::deque<int> &values, int valuesOffset,
                      int iterations) = 0;

  /**
   * Read <code>8 * iterations * blockCount()</code> blocks from
   * <code>blocks</code>, decode them and write <code>iterations *
   * valueCount()</code> values into <code>values</code>.
   *
   * @param blocks       the long blocks that hold packed integer values
   * @param blocksOffset the offset where to start reading blocks
   * @param values       the values buffer
   * @param valuesOffset the offset where to start writing values
   * @param iterations   controls how much data to decode
   */
  virtual void decode(std::deque<char> &blocks, int blocksOffset,
                      std::deque<int> &values, int valuesOffset,
                      int iterations) = 0;
};

/**
 * An encoder for packed integers.
 */
public:
class Encoder
{
  GET_CLASS_NAME(Encoder)

  /**
   * The minimum number of long blocks to encode in a single iteration, when
   * using long encoding.
   */
public:
  virtual int longBlockCount() = 0;

  /**
   * The number of values that can be stored in {@link #longBlockCount()} long
   * blocks.
   */
  virtual int longValueCount() = 0;

  /**
   * The minimum number of byte blocks to encode in a single iteration, when
   * using byte encoding.
   */
  virtual int byteBlockCount() = 0;

  /**
   * The number of values that can be stored in {@link #byteBlockCount()} byte
   * blocks.
   */
  virtual int byteValueCount() = 0;

  /**
   * Read <code>iterations * valueCount()</code> values from
   * <code>values</code>, encode them and write <code>iterations *
   * blockCount()</code> blocks into <code>blocks</code>.
   *
   * @param blocks       the long blocks that hold packed integer values
   * @param blocksOffset the offset where to start writing blocks
   * @param values       the values buffer
   * @param valuesOffset the offset where to start reading values
   * @param iterations   controls how much data to encode
   */
  virtual void encode(std::deque<int64_t> &values, int valuesOffset,
                      std::deque<int64_t> &blocks, int blocksOffset,
                      int iterations) = 0;

  /**
   * Read <code>iterations * valueCount()</code> values from
   * <code>values</code>, encode them and write <code>8 * iterations *
   * blockCount()</code> blocks into <code>blocks</code>.
   *
   * @param blocks       the long blocks that hold packed integer values
   * @param blocksOffset the offset where to start writing blocks
   * @param values       the values buffer
   * @param valuesOffset the offset where to start reading values
   * @param iterations   controls how much data to encode
   */
  virtual void encode(std::deque<int64_t> &values, int valuesOffset,
                      std::deque<char> &blocks, int blocksOffset,
                      int iterations) = 0;

  /**
   * Read <code>iterations * valueCount()</code> values from
   * <code>values</code>, encode them and write <code>iterations *
   * blockCount()</code> blocks into <code>blocks</code>.
   *
   * @param blocks       the long blocks that hold packed integer values
   * @param blocksOffset the offset where to start writing blocks
   * @param values       the values buffer
   * @param valuesOffset the offset where to start reading values
   * @param iterations   controls how much data to encode
   */
  virtual void encode(std::deque<int> &values, int valuesOffset,
                      std::deque<int64_t> &blocks, int blocksOffset,
                      int iterations) = 0;

  /**
   * Read <code>iterations * valueCount()</code> values from
   * <code>values</code>, encode them and write <code>8 * iterations *
   * blockCount()</code> blocks into <code>blocks</code>.
   *
   * @param blocks       the long blocks that hold packed integer values
   * @param blocksOffset the offset where to start writing blocks
   * @param values       the values buffer
   * @param valuesOffset the offset where to start reading values
   * @param iterations   controls how much data to encode
   */
  virtual void encode(std::deque<int> &values, int valuesOffset,
                      std::deque<char> &blocks, int blocksOffset,
                      int iterations) = 0;
};

/**
 * A read-only random access array of positive integers.
 * @lucene.internal
 */
public:
class Reader : public LegacyNumericDocValues, public Accountable
{
  GET_CLASS_NAME(Reader)

  /**
   * Bulk get: read at least one and at most <code>len</code> longs starting
   * from <code>index</code> into <code>arr[off:off+len]</code> and return
   * the actual number of values that have been read.
   */
public:
  virtual int get(int index, std::deque<int64_t> &arr, int off, int len);

  /**
   * @return the number of values.
   */
  virtual int size() = 0;

protected:
  std::shared_ptr<Reader> shared_from_this()
  {
    return std::static_pointer_cast<Reader>(
        org.apache.lucene.index.LegacyNumericDocValues::shared_from_this());
  }
};

/**
 * Run-once iterator interface, to decode previously saved PackedInts.
 */
public:
class ReaderIterator
{
  GET_CLASS_NAME(ReaderIterator)
  /** Returns next value */
public:
  virtual int64_t next() = 0;
  /** Returns at least 1 and at most <code>count</code> next values,
   * the returned ref MUST NOT be modified */
  virtual std::shared_ptr<LongsRef> next(int count) = 0;
  /** Returns number of bits per value */
  virtual int getBitsPerValue() = 0;
  /** Returns number of values */
  virtual int size() = 0;
  /** Returns the current position */
  virtual int ord() = 0;
};

public:
class ReaderIteratorImpl
    : public std::enable_shared_from_this<ReaderIteratorImpl>,
      public ReaderIterator
{
  GET_CLASS_NAME(ReaderIteratorImpl)

protected:
  const std::shared_ptr<DataInput> in_;
  const int bitsPerValue;
  const int valueCount;

  ReaderIteratorImpl(int valueCount, int bitsPerValue,
                     std::shared_ptr<DataInput> in_);

public:
  int64_t next()  override;

  int getBitsPerValue() override;

  int size() override;
};

/**
 * A packed integer array that can be modified.
 * @lucene.internal
 */
public:
class Mutable : public Reader
{
  GET_CLASS_NAME(Mutable)

  /**
   * @return the number of bits used to store any given value.
   *         Note: This does not imply that memory usage is
   *         {@code bitsPerValue * #values} as implementations are free to
   *         use non-space-optimal packing of bits.
   */
public:
  virtual int getBitsPerValue() = 0;

  /**
   * Set the value at the given index in the array.
   * @param index where the value should be positioned.
   * @param value a value conforming to the constraints set by the array.
   */
  virtual void set(int index, int64_t value) = 0;

  /**
   * Bulk set: set at least one and at most <code>len</code> longs starting
   * at <code>off</code> in <code>arr</code> into this mutable, starting at
   * <code>index</code>. Returns the actual number of values that have been
   * set.
   */
  virtual int set(int index, std::deque<int64_t> &arr, int off, int len);

  /**
   * Fill the mutable from <code>fromIndex</code> (inclusive) to
   * <code>toIndex</code> (exclusive) with <code>val</code>.
   */
  virtual void fill(int fromIndex, int toIndex, int64_t val);

  /**
   * Sets all values to 0.
   */
  virtual void clear();

  /**
   * Save this mutable into <code>out</code>. Instantiating a reader from
   * the generated data will return a reader with the same number of bits
   * per value.
   */
  virtual void save(std::shared_ptr<DataOutput> out) ;

  /** The underlying format. */
  virtual Format getFormat();

protected:
  std::shared_ptr<Mutable> shared_from_this()
  {
    return std::static_pointer_cast<Mutable>(Reader::shared_from_this());
  }
};

/**
 * A simple base for Readers that keeps track of valueCount and bitsPerValue.
 * @lucene.internal
 */
public:
class ReaderImpl : public Reader
{
  GET_CLASS_NAME(ReaderImpl)
protected:
  const int valueCount;

  ReaderImpl(int valueCount);

public:
  int64_t get(int index) = 0;
  override override;

  int size() override final;

protected:
  std::shared_ptr<ReaderImpl> shared_from_this()
  {
    return std::static_pointer_cast<ReaderImpl>(Reader::shared_from_this());
  }
};

public:
class MutableImpl : public Mutable
{
  GET_CLASS_NAME(MutableImpl)

protected:
  const int valueCount;
  const int bitsPerValue;

  MutableImpl(int valueCount, int bitsPerValue);

public:
  int getBitsPerValue() override final;

  int size() override final;

  virtual std::wstring toString();

protected:
  std::shared_ptr<MutableImpl> shared_from_this()
  {
    return std::static_pointer_cast<MutableImpl>(Mutable::shared_from_this());
  }
};

/** A {@link Reader} which has all its values equal to 0 (bitsPerValue = 0). */
public:
class NullReader final : public Reader
{
  GET_CLASS_NAME(NullReader)

private:
  const int valueCount;

  /** Sole constructor. */
public:
  NullReader(int valueCount);

  int64_t get(int index) override;

  int get(int index, std::deque<int64_t> &arr, int off, int len) override;

  int size() override;

  int64_t ramBytesUsed() override;

protected:
  std::shared_ptr<NullReader> shared_from_this()
  {
    return std::static_pointer_cast<NullReader>(Reader::shared_from_this());
  }
};

/** A write-once Writer.
 * @lucene.internal
 */
public:
class Writer : public std::enable_shared_from_this<Writer>
{
  GET_CLASS_NAME(Writer)
protected:
  const std::shared_ptr<DataOutput> out;
  const int valueCount;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int bitsPerValue_;

  Writer(std::shared_ptr<DataOutput> out, int valueCount, int bitsPerValue);

public:
  virtual void writeHeader() ;

  /** The format used to serialize values. */
protected:
  virtual PackedInts::Format getFormat() = 0;

  /** Add a value to the stream. */
public:
  virtual void add(int64_t v) = 0;

  /** The number of bits per value. */
  int bitsPerValue();

  /** Perform end-of-stream operations. */
  virtual void finish() = 0;

  /**
   * Returns the current ord in the stream (number of values that have been
   * written so far minus one).
   */
  virtual int ord() = 0;
};

/**
 * Get a {@link Decoder}.
 *
 * @param format         the format used to store packed ints
 * @param version        the compatibility version
 * @param bitsPerValue   the number of bits per value
 * @return a decoder
 */
public:
static std::shared_ptr<Decoder> getDecoder(Format format, int version,
                                           int bitsPerValue);

/**
 * Get an {@link Encoder}.
 *
 * @param format         the format used to store packed ints
 * @param version        the compatibility version
 * @param bitsPerValue   the number of bits per value
 * @return an encoder
 */
static std::shared_ptr<Encoder> getEncoder(Format format, int version,
                                           int bitsPerValue);

/**
 * Expert: Restore a {@link Reader} from a stream without reading metadata at
 * the beginning of the stream. This method is useful to restore data from
 * streams which have been created using
 * {@link PackedInts#getWriterNoHeader(DataOutput, Format, int, int, int)}.
 *
 * @param in           the stream to read data from, positioned at the beginning
 * of the packed values
 * @param format       the format used to serialize
 * @param version      the version used to serialize the data
 * @param valueCount   how many values the stream holds
 * @param bitsPerValue the number of bits per value
 * @return             a Reader
 * @throws IOException If there is a low-level I/O error
 * @see PackedInts#getWriterNoHeader(DataOutput, Format, int, int, int)
 * @lucene.internal
 */
static std::shared_ptr<Reader>
getReaderNoHeader(std::shared_ptr<DataInput> in_, Format format, int version,
                  int valueCount, int bitsPerValue) ;

/**
 * Restore a {@link Reader} from a stream.
 *
 * @param in           the stream to read data from
 * @return             a Reader
 * @throws IOException If there is a low-level I/O error
 * @lucene.internal
 */
static std::shared_ptr<Reader>
getReader(std::shared_ptr<DataInput> in_) ;

/**
 * Expert: Restore a {@link ReaderIterator} from a stream without reading
 * metadata at the beginning of the stream. This method is useful to restore
 * data from streams which have been created using
 * {@link PackedInts#getWriterNoHeader(DataOutput, Format, int, int, int)}.
 *
 * @param in           the stream to read data from, positioned at the beginning
 * of the packed values
 * @param format       the format used to serialize
 * @param version      the version used to serialize the data
 * @param valueCount   how many values the stream holds
 * @param bitsPerValue the number of bits per value
 * @param mem          how much memory the iterator is allowed to use to
 * read-ahead (likely to speed up iteration)
 * @return             a ReaderIterator
 * @see PackedInts#getWriterNoHeader(DataOutput, Format, int, int, int)
 * @lucene.internal
 */
static std::shared_ptr<ReaderIterator>
getReaderIteratorNoHeader(std::shared_ptr<DataInput> in_, Format format,
                          int version, int valueCount, int bitsPerValue,
                          int mem);

/**
 * Retrieve PackedInts as a {@link ReaderIterator}
 * @param in positioned at the beginning of a stored packed int structure.
 * @param mem how much memory the iterator is allowed to use to read-ahead
 * (likely to speed up iteration)
 * @return an iterator to access the values
 * @throws IOException if the structure could not be retrieved.
 * @lucene.internal
 */
static std::shared_ptr<ReaderIterator>
getReaderIterator(std::shared_ptr<DataInput> in_, int mem) ;

/**
 * Expert: Construct a direct {@link Reader} from a stream without reading
 * metadata at the beginning of the stream. This method is useful to restore
 * data from streams which have been created using
 * {@link PackedInts#getWriterNoHeader(DataOutput, Format, int, int, int)}.
 * <p>
 * The returned reader will have very little memory overhead, but every call
 * to {@link Reader#get(int)} is likely to perform a disk seek.
 *
 * @param in           the stream to read data from
 * @param format       the format used to serialize
 * @param version      the version used to serialize the data
 * @param valueCount   how many values the stream holds
 * @param bitsPerValue the number of bits per value
 * @return a direct Reader
 * @lucene.internal
 */
static std::shared_ptr<Reader>
getDirectReaderNoHeader(std::shared_ptr<IndexInput> in_, Format format,
                        int version, int valueCount, int bitsPerValue);

/**
 * Construct a direct {@link Reader} from an {@link IndexInput}. This method
 * is useful to restore data from streams which have been created using
 * {@link PackedInts#getWriter(DataOutput, int, int, float)}.
 * <p>
 * The returned reader will have very little memory overhead, but every call
 * to {@link Reader#get(int)} is likely to perform a disk seek.
 *
 * @param in           the stream to read data from
 * @return a direct Reader
 * @throws IOException If there is a low-level I/O error
 * @lucene.internal
 */
static std::shared_ptr<Reader>
getDirectReader(std::shared_ptr<IndexInput> in_) ;

/**
 * Create a packed integer array with the given amount of values initialized
 * to 0. the valueCount and the bitsPerValue cannot be changed after creation.
 * All Mutables known by this factory are kept fully in RAM.
 * <p>
 * Positive values of <code>acceptableOverheadRatio</code> will trade space
 * for speed by selecting a faster but potentially less memory-efficient
 * implementation. An <code>acceptableOverheadRatio</code> of
 * {@link PackedInts#COMPACT} will make sure that the most memory-efficient
 * implementation is selected whereas {@link PackedInts#FASTEST} will make sure
 * that the fastest implementation is selected.
 *
 * @param valueCount   the number of elements
 * @param bitsPerValue the number of bits available for any given value
 * @param acceptableOverheadRatio an acceptable overhead
 *        ratio per value
 * @return a mutable packed integer array
 * @lucene.internal
 */
static std::shared_ptr<Mutable> getMutable(int valueCount, int bitsPerValue,
                                           float acceptableOverheadRatio);

/** Same as {@link #getMutable(int, int, float)} with a pre-computed number
 *  of bits per value and format.
 *  @lucene.internal */
static std::shared_ptr<Mutable> getMutable(int valueCount, int bitsPerValue,
                                           PackedInts::Format format);

/**
 * Expert: Create a packed integer array writer for the given output, format,
 * value count, and number of bits per value.
 * <p>
 * The resulting stream will be long-aligned. This means that depending on
 * the format which is used, up to 63 bits will be wasted. An easy way to
 * make sure that no space is lost is to always use a <code>valueCount</code>
 * that is a multiple of 64.
 * <p>
 * This method does not write any metadata to the stream, meaning that it is
 * your responsibility to store it somewhere else in order to be able to
 * recover data from the stream later on:
 * <ul>
 *   <li><code>format</code> (using {@link Format#getId()}),</li>
 *   <li><code>valueCount</code>,</li>
 *   <li><code>bitsPerValue</code>,</li>
 *   <li>{@link #VERSION_CURRENT}.</li>
 * </ul>
 * <p>
 * It is possible to start writing values without knowing how many of them you
 * are actually going to write. To do this, just pass <code>-1</code> as
 * <code>valueCount</code>. On the other hand, for any positive value of
 * <code>valueCount</code>, the returned writer will make sure that you don't
 * write more values than expected and pad the end of stream with zeros in
 * case you have written less than <code>valueCount</code> when calling
 * {@link Writer#finish()}.
 * <p>
 * The <code>mem</code> parameter lets you control how much memory can be used
 * to buffer changes in memory before flushing to disk. High values of
 * <code>mem</code> are likely to improve throughput. On the other hand, if
 * speed is not that important to you, a value of <code>0</code> will use as
 * little memory as possible and should already offer reasonable throughput.
 *
 * @param out          the data output
 * @param format       the format to use to serialize the values
 * @param valueCount   the number of values
 * @param bitsPerValue the number of bits per value
 * @param mem          how much memory (in bytes) can be used to speed up
 * serialization
 * @return             a Writer
 * @see PackedInts#getReaderIteratorNoHeader(DataInput, Format, int, int, int,
 * int)
 * @see PackedInts#getReaderNoHeader(DataInput, Format, int, int, int)
 * @lucene.internal
 */
static std::shared_ptr<Writer>
getWriterNoHeader(std::shared_ptr<DataOutput> out, Format format,
                  int valueCount, int bitsPerValue, int mem);

/**
 * Create a packed integer array writer for the given output, format, value
 * count, and number of bits per value.
 * <p>
 * The resulting stream will be long-aligned. This means that depending on
 * the format which is used under the hoods, up to 63 bits will be wasted.
 * An easy way to make sure that no space is lost is to always use a
 * <code>valueCount</code> that is a multiple of 64.
 * <p>
 * This method writes metadata to the stream, so that the resulting stream is
 * sufficient to restore a {@link Reader} from it. You don't need to track
 * <code>valueCount</code> or <code>bitsPerValue</code> by yourself. In case
 * this is a problem, you should probably look at
 * {@link #getWriterNoHeader(DataOutput, Format, int, int, int)}.
 * <p>
 * The <code>acceptableOverheadRatio</code> parameter controls how
 * readers that will be restored from this stream trade space
 * for speed by selecting a faster but potentially less memory-efficient
 * implementation. An <code>acceptableOverheadRatio</code> of
 * {@link PackedInts#COMPACT} will make sure that the most memory-efficient
 * implementation is selected whereas {@link PackedInts#FASTEST} will make sure
 * that the fastest implementation is selected. In case you are only interested
 * in reading this stream sequentially later on, you should probably use
 * {@link PackedInts#COMPACT}.
 *
 * @param out          the data output
 * @param valueCount   the number of values
 * @param bitsPerValue the number of bits per value
 * @param acceptableOverheadRatio an acceptable overhead ratio per value
 * @return             a Writer
 * @throws IOException If there is a low-level I/O error
 * @lucene.internal
 */
static std::shared_ptr<Writer>
getWriter(std::shared_ptr<DataOutput> out, int valueCount, int bitsPerValue,
          float acceptableOverheadRatio) ;

/** Returns how many bits are required to hold values up
 *  to and including maxValue
 *  NOTE: This method returns at least 1.
 * @param maxValue the maximum value that should be representable.
 * @return the amount of bits needed to represent values from 0 to maxValue.
 * @lucene.internal
 */
static int bitsRequired(int64_t maxValue);

/** Returns how many bits are required to store <code>bits</code>,
 * interpreted as an unsigned value.
 * NOTE: This method returns at least 1.
 * @lucene.internal
 */
static int unsignedBitsRequired(int64_t bits);

/**
 * Calculates the maximum unsigned long that can be expressed with the given
 * number of bits.
 * @param bitsPerValue the number of bits available for any given value.
 * @return the maximum value for the given bits.
 * @lucene.internal
 */
static int64_t maxValue(int bitsPerValue);

/**
 * Copy <code>src[srcPos:srcPos+len]</code> into
 * <code>dest[destPos:destPos+len]</code> using at most <code>mem</code>
 * bytes.
 */
static void copy(std::shared_ptr<Reader> src, int srcPos,
                 std::shared_ptr<Mutable> dest, int destPos, int len, int mem);

/** Same as {@link #copy(Reader, int, Mutable, int, int, int)} but using a
 * pre-allocated buffer. */
static void copy(std::shared_ptr<Reader> src, int srcPos,
                 std::shared_ptr<Mutable> dest, int destPos, int len,
                 std::deque<int64_t> &buf);

/** Check that the block size is a power of 2, in the right bounds, and return
 *  its log in base 2. */
static int checkBlockSize(int blockSize, int minBlockSize, int maxBlockSize);

/** Return the number of blocks required to store <code>size</code> values on
 *  <code>blockSize</code>. */
static int numBlocks(int64_t size, int blockSize);
}
;
}
