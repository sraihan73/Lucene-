#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"

#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"

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
namespace org::apache::lucene::codecs
{

using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;

/**
 * Utility class for reading and writing versioned headers.
 * <p>
 * Writing codec headers is useful to ensure that a file is in
 * the format you think it is.
 *
 * @lucene.experimental
 */

class CodecUtil final : public std::enable_shared_from_this<CodecUtil>
{
  GET_CLASS_NAME(CodecUtil)
private:
  CodecUtil();

  /**
   * Constant to identify the start of a codec header.
   */
public:
  static constexpr int CODEC_MAGIC = 0x3fd76c17;
  /**
   * Constant to identify the start of a codec footer.
   */
  static const int FOOTER_MAGIC = ~CODEC_MAGIC;

  /**
   * Writes a codec header, which records both a string to
   * identify the file and a version number. This header can
   * be parsed and validated with
   * {@link #checkHeader(DataInput, std::wstring, int, int) checkHeader()}.
   * <p>
   * CodecHeader --&gt; Magic,CodecName,Version
   * <ul>
   *    <li>Magic --&gt; {@link DataOutput#writeInt Uint32}. This
   *        identifies the start of the header. It is always {@value
   * #CODEC_MAGIC}. <li>CodecName --&gt; {@link DataOutput#writeString std::wstring}.
   * This is a string to identify this file. <li>Version --&gt; {@link
   * DataOutput#writeInt Uint32}. Records the version of the file.
   * </ul>
   * <p>
   * Note that the length of a codec header depends only upon the
   * name of the codec, so this length can be computed at any time
   * with {@link #headerLength(std::wstring)}.
   *
   * @param out Output stream
   * @param codec std::wstring to identify this file. It should be simple ASCII,
   *              less than 128 characters in length.
   * @param version Version number
   * @throws IOException If there is an I/O error writing to the underlying
   * medium.
   * @throws IllegalArgumentException If the codec name is not simple ASCII, or
   * is more than 127 characters in length
   */
  static void writeHeader(std::shared_ptr<DataOutput> out,
                          const std::wstring &codec,
                          int version) ;

  /**
   * Writes a codec header for an index file, which records both a string to
   * identify the format of the file, a version number, and data to identify
   * the file instance (ID and auxiliary suffix such as generation).
   * <p>
   * This header can be parsed and validated with
   * {@link #checkIndexHeader(DataInput, std::wstring, int, int, byte[], std::wstring)
   * checkIndexHeader()}. <p> IndexHeader --&gt;
   * CodecHeader,ObjectID,ObjectSuffix <ul> <li>CodecHeader   --&gt; {@link
   * #writeHeader} <li>ObjectID     --&gt; {@link DataOutput#writeByte
   * byte}<sup>16</sup> <li>ObjectSuffix --&gt; SuffixLength,SuffixBytes
   *    <li>SuffixLength  --&gt; {@link DataOutput#writeByte byte}
   *    <li>SuffixBytes   --&gt; {@link DataOutput#writeByte
   * byte}<sup>SuffixLength</sup>
   * </ul>
   * <p>
   * Note that the length of an index header depends only upon the
   * name of the codec and suffix, so this length can be computed at any time
   * with {@link #indexHeaderLength(std::wstring,std::wstring)}.
   *
   * @param out Output stream
   * @param codec std::wstring to identify the format of this file. It should be
   * simple ASCII, less than 128 characters in length.
   * @param id Unique identifier for this particular file instance.
   * @param suffix auxiliary suffix information for the file. It should be
   * simple ASCII, less than 256 characters in length.
   * @param version Version number
   * @throws IOException If there is an I/O error writing to the underlying
   * medium.
   * @throws IllegalArgumentException If the codec name is not simple ASCII, or
   *         is more than 127 characters in length, or if id is invalid,
   *         or if the suffix is not simple ASCII, or more than 255 characters
   *         in length.
   */
  static void writeIndexHeader(std::shared_ptr<DataOutput> out,
                               const std::wstring &codec, int version,
                               std::deque<char> &id,
                               const std::wstring &suffix) ;

  /**
   * Computes the length of a codec header.
   *
   * @param codec Codec name.
   * @return length of the entire codec header.
   * @see #writeHeader(DataOutput, std::wstring, int)
   */
  static int headerLength(const std::wstring &codec);

  /**
   * Computes the length of an index header.
   *
   * @param codec Codec name.
   * @return length of the entire index header.
   * @see #writeIndexHeader(DataOutput, std::wstring, int, byte[], std::wstring)
   */
  static int indexHeaderLength(const std::wstring &codec,
                               const std::wstring &suffix);

  /**
   * Reads and validates a header previously written with
   * {@link #writeHeader(DataOutput, std::wstring, int)}.
   * <p>
   * When reading a file, supply the expected <code>codec</code> and
   * an expected version range (<code>minVersion to maxVersion</code>).
   *
   * @param in Input stream, positioned at the point where the
   *        header was previously written. Typically this is located
   *        at the beginning of the file.
   * @param codec The expected codec name.
   * @param minVersion The minimum supported expected version number.
   * @param maxVersion The maximum supported expected version number.
   * @return The actual version found, when a valid header is found
   *         that matches <code>codec</code>, with an actual version
   *         where {@code minVersion <= actual <= maxVersion}.
   *         Otherwise an exception is thrown.
   * @throws CorruptIndexException If the first four bytes are not
   *         {@link #CODEC_MAGIC}, or if the actual codec found is
   *         not <code>codec</code>.
   * @throws IndexFormatTooOldException If the actual version is less
   *         than <code>minVersion</code>.
   * @throws IndexFormatTooNewException If the actual version is greater
   *         than <code>maxVersion</code>.
   * @throws IOException If there is an I/O error reading from the underlying
   * medium.
   * @see #writeHeader(DataOutput, std::wstring, int)
   */
  static int checkHeader(std::shared_ptr<DataInput> in_,
                         const std::wstring &codec, int minVersion,
                         int maxVersion) ;

  /** Like {@link
   *  #checkHeader(DataInput,std::wstring,int,int)} except this
   *  version assumes the first int has already been read
   *  and validated from the input. */
  static int checkHeaderNoMagic(std::shared_ptr<DataInput> in_,
                                const std::wstring &codec, int minVersion,
                                int maxVersion) ;

  /**
   * Reads and validates a header previously written with
   * {@link #writeIndexHeader(DataOutput, std::wstring, int, byte[], std::wstring)}.
   * <p>
   * When reading a file, supply the expected <code>codec</code>,
   * expected version range (<code>minVersion to maxVersion</code>),
   * and object ID and suffix.
   *
   * @param in Input stream, positioned at the point where the
   *        header was previously written. Typically this is located
   *        at the beginning of the file.
   * @param codec The expected codec name.
   * @param minVersion The minimum supported expected version number.
   * @param maxVersion The maximum supported expected version number.
   * @param expectedID The expected object identifier for this file.
   * @param expectedSuffix The expected auxiliary suffix for this file.
   * @return The actual version found, when a valid header is found
   *         that matches <code>codec</code>, with an actual version
   *         where {@code minVersion <= actual <= maxVersion},
   *         and matching <code>expectedID</code> and
   * <code>expectedSuffix</code> Otherwise an exception is thrown.
   * @throws CorruptIndexException If the first four bytes are not
   *         {@link #CODEC_MAGIC}, or if the actual codec found is
   *         not <code>codec</code>, or if the <code>expectedID</code>
   *         or <code>expectedSuffix</code> do not match.
   * @throws IndexFormatTooOldException If the actual version is less
   *         than <code>minVersion</code>.
   * @throws IndexFormatTooNewException If the actual version is greater
   *         than <code>maxVersion</code>.
   * @throws IOException If there is an I/O error reading from the underlying
   * medium.
   * @see #writeIndexHeader(DataOutput, std::wstring, int, byte[],std::wstring)
   */
  static int
  checkIndexHeader(std::shared_ptr<DataInput> in_, const std::wstring &codec,
                   int minVersion, int maxVersion,
                   std::deque<char> &expectedID,
                   const std::wstring &expectedSuffix) ;

  /**
   * Expert: verifies the incoming {@link IndexInput} has an index header
   * and that its segment ID matches the expected one, and then copies
   * that index header into the provided {@link DataOutput}.  This is
   * useful when building compound files.
   *
   * @param in Input stream, positioned at the point where the
   *        index header was previously written. Typically this is located
   *        at the beginning of the file.
   * @param out Output stream, where the header will be copied to.
   * @param expectedID Expected segment ID
   * @throws CorruptIndexException If the first four bytes are not
   *         {@link #CODEC_MAGIC}, or if the <code>expectedID</code>
   *         does not match.
   * @throws IOException If there is an I/O error reading from the underlying
   * medium.
   *
   * @lucene.internal
   */
  static void
  verifyAndCopyIndexHeader(std::shared_ptr<IndexInput> in_,
                           std::shared_ptr<DataOutput> out,
                           std::deque<char> &expectedID) ;

  /** Retrieves the full index header from the provided {@link IndexInput}.
   *  This throws {@link CorruptIndexException} if this file does
   * not appear to be an index file. */
  static std::deque<char>
  readIndexHeader(std::shared_ptr<IndexInput> in_) ;

  /** Retrieves the full footer from the provided {@link IndexInput}.  This
   * throws
   *  {@link CorruptIndexException} if this file does not have a valid footer.
   */
  static std::deque<char>
  readFooter(std::shared_ptr<IndexInput> in_) ;

  /** Expert: just reads and verifies the object ID of an index header */
  static std::deque<char>
  checkIndexHeaderID(std::shared_ptr<DataInput> in_,
                     std::deque<char> &expectedID) ;

  /** Expert: just reads and verifies the suffix of an index header */
  static std::wstring
  checkIndexHeaderSuffix(std::shared_ptr<DataInput> in_,
                         const std::wstring &expectedSuffix) ;

  /**
   * Writes a codec footer, which records both a checksum
   * algorithm ID and a checksum. This footer can
   * be parsed and validated with
   * {@link #checkFooter(ChecksumIndexInput) checkFooter()}.
   * <p>
   * CodecFooter --&gt; Magic,AlgorithmID,Checksum
   * <ul>
   *    <li>Magic --&gt; {@link DataOutput#writeInt Uint32}. This
   *        identifies the start of the footer. It is always {@value
   * #FOOTER_MAGIC}. <li>AlgorithmID --&gt; {@link DataOutput#writeInt Uint32}.
   * This indicates the checksum algorithm used. Currently this is always 0, for
   * zlib-crc32. <li>Checksum --&gt; {@link DataOutput#writeLong Uint64}. The
   *        actual checksum value for all previous bytes in the stream,
   * including the bytes from Magic and AlgorithmID.
   * </ul>
   *
   * @param out Output stream
   * @throws IOException If there is an I/O error writing to the underlying
   * medium.
   */
  static void writeFooter(std::shared_ptr<IndexOutput> out) ;

  /**
   * Computes the length of a codec footer.
   *
   * @return length of the entire codec footer.
   * @see #writeFooter(IndexOutput)
   */
  static int footerLength();

  /**
   * Validates the codec footer previously written by {@link #writeFooter}.
   * @return actual checksum value
   * @throws IOException if the footer is invalid, if the checksum does not
   * match, or if {@code in} is not properly positioned before the footer at the
   * end of the stream.
   */
  static int64_t
  checkFooter(std::shared_ptr<ChecksumIndexInput> in_) ;

  /**
   * Validates the codec footer previously written by {@link #writeFooter},
optionally
   * passing an unexpected exception that has already occurred.
   * <p>
   * When a {@code priorException} is provided, this method will add a
suppressed exception
   * indicating whether the checksum for the stream passes, fails, or cannot be
computed, and
   * rethrow it. Otherwise it behaves the same as {@link
#checkFooter(ChecksumIndexInput)}.
   * <p>
   * Example usage:
   * <pre class="prettyprint">
   * try (ChecksumIndexInput input = ...) {
GET_CLASS_NAME(="prettyprint">)
   *   Throwable priorE = null;
   *   try {
   *     // ... read a bunch of stuff ...
   *   } catch (Throwable exception) {
   *     priorE = exception;
   *   } finally {
   *     CodecUtil.checkFooter(input, priorE);
   *   }
   * }
   * </pre>
   */
  static void checkFooter(std::shared_ptr<ChecksumIndexInput> in_,
                          std::runtime_error priorException) ;

  /**
   * Returns (but does not validate) the checksum previously written by {@link
   * #checkFooter}.
   * @return actual checksum value
   * @throws IOException if the footer is invalid
   */
  static int64_t
  retrieveChecksum(std::shared_ptr<IndexInput> in_) ;

private:
  static void
  validateFooter(std::shared_ptr<IndexInput> in_) ;

  /**
   * Clones the provided input, reads all bytes from the file, and calls {@link
   * #checkFooter} <p> Note that this method may be slow, as it must process the
   * entire file. If you just need to extract the checksum value, call {@link
   * #retrieveChecksum}.
   */
public:
  static int64_t
  checksumEntireFile(std::shared_ptr<IndexInput> input) ;

  /**
   * Reads CRC32 value as a 64-bit long from the input.
   * @throws CorruptIndexException if CRC is formatted incorrectly (wrong bits
   * set)
   * @throws IOException if an i/o error occurs
   */
  static int64_t
  readCRC(std::shared_ptr<IndexInput> input) ;

  /**
   * Writes CRC32 value as a 64-bit long to the output.
   * @throws IllegalStateException if CRC is formatted incorrectly (wrong bits
   * set)
   * @throws IOException if an i/o error occurs
   */
  static void writeCRC(std::shared_ptr<IndexOutput> output) ;
};

} // #include  "core/src/java/org/apache/lucene/codecs/
