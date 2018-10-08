#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

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
namespace org::apache::lucene::store
{

/**
 * Abstract base class for performing read operations of Lucene's low-level
 * data types.
 *
 * <p>{@code DataInput} may only be used from one thread, because it is not
 * thread safe (it keeps internal state like file position). To allow
 * multithreaded use, every {@code DataInput} instance must be cloned before
 * used in another thread. Subclasses must therefore implement {@link #clone()},
 * returning a new {@code DataInput} which operates on the same underlying
GET_CLASS_NAME(es)
 * resource, but positioned independently.
 */
class DataInput : public std::enable_shared_from_this<DataInput>,
                  public Cloneable
{
  GET_CLASS_NAME(DataInput)

private:
  static constexpr int SKIP_BUFFER_SIZE = 1024;

  /* This buffer is used to skip over bytes with the default implementation of
   * skipBytes. The reason why we need to use an instance member instead of
   * sharing a single instance across threads is that some delegating
   * implementations of DataInput might want to reuse the provided buffer in
   * order to eg. update the checksum. If we shared the same buffer across
   * threads, then another thread might update the buffer while the checksum is
   * being computed, making it invalid. See LUCENE-5583 for more information.
   */
  std::deque<char> skipBuffer;

  /** Reads and returns a single byte.
   * @see DataOutput#writeByte(byte)
   */
public:
  virtual char readByte() = 0;

  /** Reads a specified number of bytes into an array at the specified offset.
   * @param b the array to read bytes into
   * @param offset the offset in the array to start storing bytes
   * @param len the number of bytes to read
   * @see DataOutput#writeBytes(byte[],int)
   */
  virtual void readBytes(std::deque<char> &b, int offset, int len) = 0;

  /** Reads a specified number of bytes into an array at the
   * specified offset with control over whether the read
   * should be buffered (callers who have their own buffer
   * should pass in "false" for useBuffer).  Currently only
   * {@link BufferedIndexInput} respects this parameter.
   * @param b the array to read bytes into
   * @param offset the offset in the array to start storing bytes
   * @param len the number of bytes to read
   * @param useBuffer set to false if the caller will handle
   * buffering.
   * @see DataOutput#writeBytes(byte[],int)
   */
  virtual void readBytes(std::deque<char> &b, int offset, int len,
                         bool useBuffer) ;

  /** Reads two bytes and returns a short.
   * @see DataOutput#writeByte(byte)
   */
  virtual short readShort() ;

  /** Reads four bytes and returns an int.
   * @see DataOutput#writeInt(int)
   */
  virtual int readInt() ;

  /** Reads an int stored in variable-length format.  Reads between one and
   * five bytes.  Smaller values take fewer bytes.  Negative numbers are not
   * supported.
   * <p>
   * The format is described further in {@link DataOutput#writeVInt(int)}.
   *
   * @see DataOutput#writeVInt(int)
   */
  virtual int readVInt() ;

  /**
   * Read a {@link BitUtil#zigZagDecode(int) zig-zag}-encoded
   * {@link #readVInt() variable-length} integer.
   * @see DataOutput#writeZInt(int)
   */
  virtual int readZInt() ;

  /** Reads eight bytes and returns a long.
   * @see DataOutput#writeLong(long)
   */
  virtual int64_t readLong() ;

  /** Reads a long stored in variable-length format.  Reads between one and
   * nine bytes.  Smaller values take fewer bytes.  Negative numbers are not
   * supported.
   * <p>
   * The format is described further in {@link DataOutput#writeVInt(int)}.
   *
   * @see DataOutput#writeVLong(long)
   */
  virtual int64_t readVLong() ;

private:
  int64_t readVLong(bool allowNegative) ;

  /**
   * Read a {@link BitUtil#zigZagDecode(long) zig-zag}-encoded
   * {@link #readVLong() variable-length} integer. Reads between one and ten
   * bytes.
   * @see DataOutput#writeZLong(long)
   */
public:
  virtual int64_t readZLong() ;

  /** Reads a string.
   * @see DataOutput#writeString(std::wstring)
   */
  virtual std::wstring readString() ;

  /** Returns a clone of this stream.
   *
   * <p>Clones of a stream access the same data, and are positioned at the same
   * point as the stream they were cloned from.
   *
   * <p>Expert: Subclasses must ensure that clones may be positioned at
   * different points in the input from each other and from the stream they
   * were cloned from.
   */
  std::shared_ptr<DataInput> clone() override;

  /**
   * Reads a Map&lt;std::wstring,std::wstring&gt; previously written
   * with {@link DataOutput#writeMapOfStrings(Map)}.
   * @return An immutable map_obj containing the written contents.
   */
  virtual std::unordered_map<std::wstring, std::wstring>
  readMapOfStrings() ;

  /**
   * Reads a Set&lt;std::wstring&gt; previously written
   * with {@link DataOutput#writeSetOfStrings(Set)}.
   * @return An immutable set containing the written contents.
   */
  virtual std::shared_ptr<Set<std::wstring>>
  readSetOfStrings() ;

  /**
   * Skip over <code>numBytes</code> bytes. The contract on this method is that
   * it should have the same behavior as reading the same number of bytes into a
   * buffer and discarding its content. Negative values of <code>numBytes</code>
   * are not supported.
   */
  virtual void skipBytes(int64_t const numBytes) ;
};

} // namespace org::apache::lucene::store
