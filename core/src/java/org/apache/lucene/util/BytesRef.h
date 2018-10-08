#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
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
namespace org::apache::lucene::util
{

/** Represents byte[], as a slice (offset + length) into an
 *  existing byte[].  The {@link #bytes} member should never be null;
 *  use {@link #EMPTY_BYTES} if necessary.
 *
 * <p><b>Important note:</b> Unless otherwise noted, Lucene uses this class to
 * represent terms that are encoded as <b>UTF8</b> bytes in the index. To
 * convert them to a Java {@link std::wstring} (which is UTF16), use {@link
 * #utf8ToString}. Using code like {@code new std::wstring(bytes, offset, length)} to
 * do this is <b>wrong</b>, as it does not respect the correct character set and
 * may return wrong results (depending on the platform's defaults)!
 *
 * <p>{@code BytesRef} implements {@link Comparable}. The underlying byte arrays
 * are sorted lexicographically, numerically treating elements as unsigned.
 * This is identical to Unicode codepoint order.
 */
class BytesRef final : public std::enable_shared_from_this<BytesRef>
{
  GET_CLASS_NAME(BytesRef)
  /** An empty byte array for convenience */
public:
  static std::deque<char> const EMPTY_BYTES;

  /** The contents of the BytesRef. Should never be {@code null}. */
  std::deque<char> bytes;

  /** Offset of first valid byte. */
  int offset = 0;

  /** Length of used bytes. */
  int length = 0;

  /** Create a BytesRef with {@link #EMPTY_BYTES} */
  BytesRef();

  /** This instance will directly reference bytes w/o making a copy.
   * bytes should not be null.
   */
  BytesRef(std::deque<char> &bytes, int offset, int length);

  /** This instance will directly reference bytes w/o making a copy.
   * bytes should not be null */
  BytesRef(std::deque<char> &bytes);

  /**
   * Create a BytesRef pointing to a new array of size <code>capacity</code>.
   * Offset and length will both be zero.
   */
  BytesRef(int capacity);

  /**
   * Initialize the byte[] from the UTF8 bytes
   * for the provided std::wstring.
   *
   * @param text This must be well-formed
   * unicode text, with no unpaired surrogates.
   */
  BytesRef(std::shared_ptr<std::wstring> text);

  /**
   * Expert: compares the bytes against another BytesRef,
   * returning true if the bytes are equal.
   *
   * @param other Another BytesRef, should not be null.
   * @lucene.internal
   */
  bool bytesEquals(std::shared_ptr<BytesRef> other);

  /**
   * Returns a shallow clone of this instance (the underlying bytes are
   * <b>not</b> copied and will be shared by both the returned object and this
   * object.
   *
   * @see #deepCopyOf
   */
  std::shared_ptr<BytesRef> clone();

  /** Calculates the hash code as required by TermsHash during indexing.
   *  <p> This is currently implemented as MurmurHash3 (32
   *  bit), using the seed from {@link
   *  StringHelper#GOOD_FAST_HASH_SEED}, but is subject to
   *  change from release to release. */
  int hashCode();

  bool equals(std::any other);

  /** Interprets stored bytes as UTF8 bytes, returning the
   *  resulting string */
  std::wstring utf8ToString();

  /** Returns hex encoded bytes, eg [0x6c 0x75 0x63 0x65 0x6e 0x65] */
  std::wstring toString();

  /** Unsigned byte order comparison */
  int compareTo(std::shared_ptr<BytesRef> other);

  /**
   * Creates a new BytesRef that points to a copy of the bytes from
   * <code>other</code>
   * <p>
   * The returned BytesRef will have a length of other.length
   * and an offset of zero.
   */
  static std::shared_ptr<BytesRef> deepCopyOf(std::shared_ptr<BytesRef> other);

  /**
   * Performs internal consistency checks.
   * Always returns true (or throws IllegalStateException)
   */
  bool isValid();
};

} // #include  "core/src/java/org/apache/lucene/util/
