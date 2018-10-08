#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataInput;
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
namespace org::apache::lucene::store
{

/**
 * Abstract base class for performing write operations of Lucene's low-level
 * data types.

 * <p>{@code DataOutput} may only be used from one thread, because it is not
 * thread safe (it keeps internal state like file position).
 */
class DataOutput : public std::enable_shared_from_this<DataOutput>
{
  GET_CLASS_NAME(DataOutput)

  /** Writes a single byte.
   * <p>
   * The most primitive data type is an eight-bit byte. Files are
   * accessed as sequences of bytes. All other data types are defined
   * as sequences of bytes, so file formats are byte-order independent.
   *
   * @see IndexInput#readByte()
   */
public:
  virtual void writeByte(char b) = 0;

  /** Writes an array of bytes.
   * @param b the bytes to write
   * @param length the number of bytes to write
   * @see DataInput#readBytes(byte[],int,int)
   */
  virtual void writeBytes(std::deque<char> &b, int length) ;

  /** Writes an array of bytes.
   * @param b the bytes to write
   * @param offset the offset in the byte array
   * @param length the number of bytes to write
   * @see DataInput#readBytes(byte[],int,int)
   */
  virtual void writeBytes(std::deque<char> &b, int offset, int length) = 0;

  /** Writes an int as four bytes.
   * <p>
   * 32-bit unsigned integer written as four bytes, high-order bytes first.
   *
   * @see DataInput#readInt()
   */
  virtual void writeInt(int i) ;

  /** Writes a short as two bytes.
   * @see DataInput#readShort()
   */
  virtual void writeShort(short i) ;

  /** Writes an int in a variable-length format.  Writes between one and
   * five bytes.  Smaller values take fewer bytes.  Negative numbers are
   * supported, but should be avoided.
   * <p>VByte is a variable-length format for positive integers is defined where
   * the high-order bit of each byte indicates whether more bytes remain to be
   * read. The low-order seven bits are appended as increasingly more
   * significant bits in the resulting integer value. Thus values from zero to
   * 127 may be stored in a single byte, values from 128 to 16,383 may be stored
   * in two bytes, and so on.</p> <p>VByte Encoding Example</p> <table
   * cellspacing="0" cellpadding="2" border="0" summary="variable length
   * encoding examples"> <tr valign="top"> <th align="left">Value</th> <th
   * align="left">Byte 1</th> <th align="left">Byte 2</th> <th align="left">Byte
   * 3</th>
   * </tr>
   * <tr valign="bottom">
   *   <td>0</td>
   *   <td><code>00000000</code></td>
   *   <td></td>
   *   <td></td>
   * </tr>
   * <tr valign="bottom">
   *   <td>1</td>
   *   <td><code>00000001</code></td>
   *   <td></td>
   *   <td></td>
   * </tr>
   * <tr valign="bottom">
   *   <td>2</td>
   *   <td><code>00000010</code></td>
   *   <td></td>
   *   <td></td>
   * </tr>
   * <tr>
   *   <td valign="top">...</td>
   *   <td valign="bottom"></td>
   *   <td valign="bottom"></td>
   *   <td valign="bottom"></td>
   * </tr>
   * <tr valign="bottom">
   *   <td>127</td>
   *   <td><code>01111111</code></td>
   *   <td></td>
   *   <td></td>
   * </tr>
   * <tr valign="bottom">
   *   <td>128</td>
   *   <td><code>10000000</code></td>
   *   <td><code>00000001</code></td>
   *   <td></td>
   * </tr>
   * <tr valign="bottom">
   *   <td>129</td>
   *   <td><code>10000001</code></td>
   *   <td><code>00000001</code></td>
   *   <td></td>
   * </tr>
   * <tr valign="bottom">
   *   <td>130</td>
   *   <td><code>10000010</code></td>
   *   <td><code>00000001</code></td>
   *   <td></td>
   * </tr>
   * <tr>
   *   <td valign="top">...</td>
   *   <td></td>
   *   <td></td>
   *   <td></td>
   * </tr>
   * <tr valign="bottom">
   *   <td>16,383</td>
   *   <td><code>11111111</code></td>
   *   <td><code>01111111</code></td>
   *   <td></td>
   * </tr>
   * <tr valign="bottom">
   *   <td>16,384</td>
   *   <td><code>10000000</code></td>
   *   <td><code>10000000</code></td>
   *   <td><code>00000001</code></td>
   * </tr>
   * <tr valign="bottom">
   *   <td>16,385</td>
   *   <td><code>10000001</code></td>
   *   <td><code>10000000</code></td>
   *   <td><code>00000001</code></td>
   * </tr>
   * <tr>
   *   <td valign="top">...</td>
   *   <td valign="bottom"></td>
   *   <td valign="bottom"></td>
   *   <td valign="bottom"></td>
   * </tr>
   * </table>
   * <p>This provides compression while still being efficient to decode.</p>
   *
   * @param i Smaller values take fewer bytes.  Negative numbers are
   * supported, but should be avoided.
   * @throws IOException If there is an I/O error writing to the underlying
   * medium.
   * @see DataInput#readVInt()
   */
  void writeVInt(int i) ;

  /**
   * Write a {@link BitUtil#zigZagEncode(int) zig-zag}-encoded
   * {@link #writeVInt(int) variable-length} integer. This is typically useful
   * to write small signed ints and is equivalent to calling
   * <code>writeVInt(BitUtil.zigZagEncode(i))</code>.
   * @see DataInput#readZInt()
   */
  void writeZInt(int i) ;

  /** Writes a long as eight bytes.
   * <p>
   * 64-bit unsigned integer written as eight bytes, high-order bytes first.
   *
   * @see DataInput#readLong()
   */
  virtual void writeLong(int64_t i) ;

  /** Writes an long in a variable-length format.  Writes between one and nine
   * bytes.  Smaller values take fewer bytes.  Negative numbers are not
   * supported.
   * <p>
   * The format is described further in {@link DataOutput#writeVInt(int)}.
   * @see DataInput#readVLong()
   */
  void writeVLong(int64_t i) ;

  // write a potentially negative vLong
private:
  void writeSignedVLong(int64_t i) ;

  /**
   * Write a {@link BitUtil#zigZagEncode(long) zig-zag}-encoded
   * {@link #writeVLong(long) variable-length} long. Writes between one and ten
   * bytes. This is typically useful to write small signed ints.
   * @see DataInput#readZLong()
   */
public:
  void writeZLong(int64_t i) ;

  /** Writes a string.
   * <p>
   * Writes strings as UTF-8 encoded bytes. First the length, in bytes, is
   * written as a {@link #writeVInt VInt}, followed by the bytes.
   *
   * @see DataInput#readString()
   */
  virtual void writeString(const std::wstring &s) ;

private:
  static int COPY_BUFFER_SIZE;
  std::deque<char> copyBuffer;

  /** Copy numBytes bytes from input to ourself. */
public:
  virtual void copyBytes(std::shared_ptr<DataInput> input,
                         int64_t numBytes) ;

  /**
   * Writes a std::wstring map_obj.
   * <p>
   * First the size is written as an {@link #writeVInt(int) vInt},
   * followed by each key-value pair written as two consecutive
   * {@link #writeString(std::wstring) std::wstring}s.
   *
   * @param map_obj Input map_obj.
   * @throws NullPointerException if {@code map_obj} is null.
   */
  virtual void writeMapOfStrings(
      std::unordered_map<std::wstring, std::wstring> &map_obj) ;

  /**
   * Writes a std::wstring set.
   * <p>
   * First the size is written as an {@link #writeVInt(int) vInt},
   * followed by each value written as a
   * {@link #writeString(std::wstring) std::wstring}.
   *
   * @param set Input set.
   * @throws NullPointerException if {@code set} is null.
   */
  virtual void
  writeSetOfStrings(std::shared_ptr<Set<std::wstring>> set) ;
};

} // namespace org::apache::lucene::store
