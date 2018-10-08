#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

/*
 * Some of this code came from the excellent Unicode
 * conversion examples from:
 *
 *   http://www.unicode.org/Public/PROGRAMS/CVTUTF
 *
 * Full Copyright for that code follows:
 */

/*
 * Copyright 2001-2004 Unicode, Inc.
 *
 * Disclaimer
 *
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 *
 * Limitations on Rights to Redistribute This Code
 *
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/*
 * Additional code came from the IBM ICU library.
 *
 *  http://www.icu-project.org
 *
 * Full Copyright for that code follows.
 */

/*
 * Copyright (C) 1999-2010, International Business Machines
 * Corporation and others.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * provided that the above copyright notice(s) and this permission notice appear
 * in all copies of the Software and that both the above copyright notice(s) and
 * this permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE BE
 * LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder shall not
 * be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization of the
 * copyright holder.
 */

/**
 * Class to encode java's UTF16 char[] into UTF8 byte[]
 * without always allocating a new byte[] as
 * std::wstring.getBytes(StandardCharsets.UTF_8) does.
 *
 * @lucene.internal
 */

class UnicodeUtil final : public std::enable_shared_from_this<UnicodeUtil>
{
  GET_CLASS_NAME(UnicodeUtil)

  /** A binary term consisting of a number of 0xff bytes, likely to be bigger
   *than other terms (e.g. collation keys) one would normally encounter, and
   *definitely bigger than any UTF-8 terms. <p> WARNING: This is not a valid
   *UTF8 Term
   **/
public:
  static const std::shared_ptr<BytesRef>
      BIG_TERM; // TODO this is unrelated here find a better place for it

private:
  UnicodeUtil();

public:
  static constexpr int UNI_SUR_HIGH_START = 0xD800;
  static constexpr int UNI_SUR_HIGH_END = 0xDBFF;
  static constexpr int UNI_SUR_LOW_START = 0xDC00;
  static constexpr int UNI_SUR_LOW_END = 0xDFFF;
  static constexpr int UNI_REPLACEMENT_CHAR = 0xFFFD;

private:
  static constexpr int64_t UNI_MAX_BMP = 0x0000FFFF;

  static constexpr int64_t HALF_SHIFT = 10;
  static constexpr int64_t HALF_MASK = 0x3FFLL;

  static const int SURROGATE_OFFSET = Character::MIN_SUPPLEMENTARY_CODE_POINT -
                                      (UNI_SUR_HIGH_START << HALF_SHIFT) -
                                      UNI_SUR_LOW_START;

  /** Maximum number of UTF8 bytes per UTF16 character. */
public:
  static constexpr int MAX_UTF8_BYTES_PER_CHAR = 3;

  /** Encode characters from a char[] source, starting at
   *  offset for length chars. It is the responsibility of the
   *  caller to make sure that the destination array is large enough.
   */
  static int UTF16toUTF8(std::deque<wchar_t> &source, int const offset,
                         int const length, std::deque<char> &out);

  /** Encode characters from this std::wstring, starting at offset
   *  for length characters. It is the responsibility of the
   *  caller to make sure that the destination array is large enough.
   */
  static int UTF16toUTF8(std::shared_ptr<std::wstring> s, int const offset,
                         int const length, std::deque<char> &out);

  /** Encode characters from this std::wstring, starting at offset
   *  for length characters. Output to the destination array
   *  will begin at {@code outOffset}. It is the responsibility of the
   *  caller to make sure that the destination array is large enough.
   *  <p>
   *  note this method returns the final output offset (outOffset + number of
   * bytes written)
   */
  static int UTF16toUTF8(std::shared_ptr<std::wstring> s, int const offset,
                         int const length, std::deque<char> &out,
                         int outOffset);

  /**
   * Calculates the number of UTF8 bytes necessary to write a UTF16 string.
   *
   * @return the number of bytes written
   */
  static int calcUTF16toUTF8Length(std::shared_ptr<std::wstring> s,
                                   int const offset, int const len);

  // Only called from assert
  /*
  private static bool matches(char[] source, int offset, int length, byte[]
  result, int upto) { try { std::wstring s1 = new std::wstring(source, offset, length);
      std::wstring s2 = new std::wstring(result, 0, upto, StandardCharsets.UTF_8);
      if (!s1.equals(s2)) {
        //System.out.println("DIFF: s1 len=" + s1.length());
        //for(int i=0;i<s1.length();i++)
        //  System.out.println("    " + i + ": " + (int) s1.charAt(i));
        //System.out.println("s2 len=" + s2.length());
        //for(int i=0;i<s2.length();i++)
        //  System.out.println("    " + i + ": " + (int) s2.charAt(i));

        // If the input string was invalid, then the
        // difference is OK
        if (!validUTF16String(s1))
          return true;

        return false;
      }
      return s1.equals(s2);
    } catch (UnsupportedEncodingException uee) {
      return false;
    }
  }

  // Only called from assert
  private static bool matches(std::wstring source, int offset, int length, byte[]
  result, int upto) { try { std::wstring s1 = source.substring(offset, offset+length);
      std::wstring s2 = new std::wstring(result, 0, upto, StandardCharsets.UTF_8);
      if (!s1.equals(s2)) {
        // Allow a difference if s1 is not valid UTF-16

        //System.out.println("DIFF: s1 len=" + s1.length());
        //for(int i=0;i<s1.length();i++)
        //  System.out.println("    " + i + ": " + (int) s1.charAt(i));
        //System.out.println("  s2 len=" + s2.length());
        //for(int i=0;i<s2.length();i++)
        //  System.out.println("    " + i + ": " + (int) s2.charAt(i));

        // If the input string was invalid, then the
        // difference is OK
        if (!validUTF16String(s1))
          return true;

        return false;
      }
      return s1.equals(s2);
    } catch (UnsupportedEncodingException uee) {
      return false;
    }
  }
  */
  static bool validUTF16String(std::shared_ptr<std::wstring> s);

  static bool validUTF16String(std::deque<wchar_t> &s, int size);

  // Borrowed from Python's 3.1.2 sources,
  // Objects/unicodeobject.c, and modified (see commented
  // out section, and the -1s) to disallow the reserved for
  // future (RFC 3629) 5/6 byte sequence characters, and
  // invalid 0xFE and 0xFF bytes.

  /* Map UTF-8 encoded prefix byte to sequence length.  -1 (0xFF)
   * means illegal prefix.  see RFC 2279 for details */
  static std::deque<int> const utf8CodeLength;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static UnicodeUtil::StaticConstructor staticConstructor;

  /**
   * Returns the number of code points in this UTF8 sequence.
   *
   * <p>This method assumes valid UTF8 input. This method
   * <strong>does not perform</strong> full UTF8 validation, it will check only
   * the first byte of each codepoint (for multi-byte sequences any bytes after
   * the head are skipped).
   *
   * @throws IllegalArgumentException If invalid codepoint header byte occurs or
   * the content is prematurely truncated.
   */
public:
  static int codePointCount(std::shared_ptr<BytesRef> utf8);

  /**
   * <p>This method assumes valid UTF8 input. This method
   * <strong>does not perform</strong> full UTF8 validation, it will check only
   * the first byte of each codepoint (for multi-byte sequences any bytes after
   * the head are skipped). It is the responsibility of the caller to make sure
   * that the destination array is large enough.
   *
   * @throws IllegalArgumentException If invalid codepoint header byte occurs or
   * the content is prematurely truncated.
   */
  static int UTF8toUTF32(std::shared_ptr<BytesRef> utf8,
                         std::deque<int> &ints);

  /** Shift value for lead surrogate to form a supplementary character. */
private:
  static constexpr int LEAD_SURROGATE_SHIFT_ = 10;
  /** Mask to retrieve the significant value from a trail surrogate.*/
  static constexpr int TRAIL_SURROGATE_MASK_ = 0x3FF;
  /** Trail surrogate minimum value */
  static constexpr int TRAIL_SURROGATE_MIN_VALUE = 0xDC00;
  /** Lead surrogate minimum value */
  static constexpr int LEAD_SURROGATE_MIN_VALUE = 0xD800;
  /** The minimum value for Supplementary code points */
  static constexpr int SUPPLEMENTARY_MIN_VALUE = 0x10000;
  /** Value that all lead surrogate starts with */
  static const int LEAD_SURROGATE_OFFSET_ =
      LEAD_SURROGATE_MIN_VALUE -
      (SUPPLEMENTARY_MIN_VALUE >> LEAD_SURROGATE_SHIFT_);

  /**
   * Cover JDK 1.5 API. Create a std::wstring from an array of codePoints.
   *
   * @param codePoints The code array
   * @param offset The start of the text in the code point array
   * @param count The number of code points
   * @return a std::wstring representing the code points between offset and count
   * @throws IllegalArgumentException If an invalid code point is encountered
   * @throws IndexOutOfBoundsException If the offset or count are out of bounds.
   */
public:
  static std::wstring newString(std::deque<int> &codePoints, int offset,
                                int count);

  // for debugging
  static std::wstring toHexString(const std::wstring &s);

  /**
   * Interprets the given byte array as UTF-8 and converts to UTF-16. It is the
   * responsibility of the caller to make sure that the destination array is
   * large enough. <p> NOTE: Full characters are read, even if this reads past
   * the length passed (and can result in an ArrayOutOfBoundsException if
   * invalid UTF-8 is passed). Explicit checks for valid UTF-8 are not
   * performed.
   */
  // TODO: broken if chars.offset != 0
  static int UTF8toUTF16(std::deque<char> &utf8, int offset, int length,
                         std::deque<wchar_t> &out);

  /** Returns the maximum number of utf8 bytes required to encode a utf16 (e.g.,
   * java char[], std::wstring) */
  static int maxUTF8Length(int utf16Length);

  /**
   * Utility method for {@link #UTF8toUTF16(byte[], int, int, char[])}
   * @see #UTF8toUTF16(byte[], int, int, char[])
   */
  static int UTF8toUTF16(std::shared_ptr<BytesRef> bytesRef,
                         std::deque<wchar_t> &chars);
};

} // #include  "core/src/java/org/apache/lucene/util/
