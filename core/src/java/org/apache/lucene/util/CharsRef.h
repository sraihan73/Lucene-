#pragma once
#include "exceptionhelper.h"
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

/**
 * Represents char[], as a slice (offset + length) into an existing char[].
 * The {@link #chars} member should never be null; use
 * {@link #EMPTY_CHARS} if necessary.
 * @lucene.internal
 */
class CharsRef final : public std::enable_shared_from_this<CharsRef>,
                       public Comparable<std::shared_ptr<CharsRef>>,
                       public std::wstring,
                       public Cloneable
{
  GET_CLASS_NAME(CharsRef)
  /** An empty character array for convenience */
public:
  static std::deque<wchar_t> const EMPTY_CHARS;
  /** The contents of the CharsRef. Should never be {@code null}. */
  std::deque<wchar_t> chars;
  /** Offset of first valid character. */
  int offset = 0;
  /** Length of used characters. */
  // C++ NOTE: Fields cannot have the same name as methods:
  int length_ = 0;

  /**
   * Creates a new {@link CharsRef} initialized an empty array zero-length
   */
  CharsRef();

  /**
   * Creates a new {@link CharsRef} initialized with an array of the given
   * capacity
   */
  CharsRef(int capacity);

  /**
   * Creates a new {@link CharsRef} initialized with the given array, offset and
   * length
   */
  CharsRef(std::deque<wchar_t> &chars, int offset, int length);

  /**
   * Creates a new {@link CharsRef} initialized with the given Strings character
   * array
   */
  CharsRef(const std::wstring &string);

  /**
   * Returns a shallow clone of this instance (the underlying characters are
   * <b>not</b> copied and will be shared by both the returned object and this
   * object.
   *
   * @see #deepCopyOf
   */
  std::shared_ptr<CharsRef> clone() override;

  virtual int hashCode();

  bool equals(std::any other) override;

  bool charsEquals(std::shared_ptr<CharsRef> other);

  /** Signed int order comparison */
  int compareTo(std::shared_ptr<CharsRef> other) override;

  virtual std::wstring toString();

  int length() override;

  wchar_t charAt(int index) override;

  std::shared_ptr<std::wstring> subSequence(int start, int end) override;

  /** @deprecated This comparator is only a transition mechanism */
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated private final static
  // java.util.Comparator<CharsRef> utf16SortedAsUTF8SortOrder = new
  // UTF16SortedAsUTF8Comparator();
  static const std::shared_ptr<Comparator<std::shared_ptr<CharsRef>>>
      utf16SortedAsUTF8SortOrder;

  /** @deprecated This comparator is only a transition mechanism */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static java.util.Comparator<CharsRef>
  // getUTF16SortedAsUTF8Comparator()
  static std::shared_ptr<Comparator<std::shared_ptr<CharsRef>>>
  getUTF16SortedAsUTF8Comparator();

  /** @deprecated This comparator is only a transition mechanism */
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated private static class UTF16SortedAsUTF8Comparator
  // implements java.util.Comparator<CharsRef>
  class UTF16SortedAsUTF8Comparator
      : public std::enable_shared_from_this<UTF16SortedAsUTF8Comparator>,
        public Comparator<std::shared_ptr<CharsRef>>
  {
    // Only singleton
  private:
    UTF16SortedAsUTF8Comparator();

  public:
    int compare(std::shared_ptr<CharsRef> a,
                std::shared_ptr<CharsRef> b) override;
  };

  /**
   * Creates a new CharsRef that points to a copy of the chars from
   * <code>other</code>
   * <p>
   * The returned CharsRef will have a length of other.length
   * and an offset of zero.
   */
public:
  static std::shared_ptr<CharsRef> deepCopyOf(std::shared_ptr<CharsRef> other);

  /**
   * Performs internal consistency checks.
   * Always returns true (or throws IllegalStateException)
   */
  bool isValid();
};
} // namespace org::apache::lucene::util
