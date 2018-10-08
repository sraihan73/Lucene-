#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"

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
namespace org::apache::lucene::index
{

using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
  A Term represents a word from text.  This is the unit of search.  It is
  composed of two elements, the text of the word, as a string, and the name of
  the field that the text occurred in.

  Note that terms may represent more than words from text fields, but also
  things like dates, email addresses, urls, etc.  */

class Term final : public std::enable_shared_from_this<Term>                   
{
  GET_CLASS_NAME(Term)
public:
  // C++ NOTE: Fields cannot have the same name as methods:
  std::wstring field_;
  // C++ NOTE: Fields cannot have the same name as methods:
  std::shared_ptr<BytesRef> bytes_;

  /** Constructs a Term with the given field and bytes.
   * <p>Note that a null field or null bytes value results in undefined
   * behavior for most Lucene APIs that accept a Term parameter.
   *
   * <p>The provided BytesRef is copied when it is non null.
   */
  Term(const std::wstring &fld, std::shared_ptr<BytesRef> bytes);

  /** Constructs a Term with the given field and the bytes from a builder.
   * <p>Note that a null field value results in undefined
   * behavior for most Lucene APIs that accept a Term parameter.
   */
  Term(const std::wstring &fld, std::shared_ptr<BytesRefBuilder> bytesBuilder);

  /** Constructs a Term with the given field and text.
   * <p>Note that a null field or null text value results in undefined
   * behavior for most Lucene APIs that accept a Term parameter. */
  Term(const std::wstring &fld, const std::wstring &text);

  /** Constructs a Term with the given field and empty text.
   * This serves two purposes: 1) reuse of a Term with the same field.
   * 2) pattern for a query.
   *
   * @param fld field's name
   */
  Term(const std::wstring &fld);

  /** Returns the field of this term.   The field indicates
    the part of a document which this term came from. */
  std::wstring field();

  /** Returns the text of this term.  In the case of words, this is simply the
    text of the word.  In the case of dates and other types, this is an
    encoding of the object as a string.  */
  std::wstring text();

  /** Returns human-readable form of the term text. If the term is not unicode,
   * the raw bytes will be printed instead. */
  static std::wstring toString(std::shared_ptr<BytesRef> termText);

  /** Returns the bytes of this term, these should not be modified. */
  std::shared_ptr<BytesRef> bytes();

  virtual bool equals(std::any obj);

  virtual int hashCode();

  /** Compares two terms, returning a negative integer if this
    term belongs before the argument, zero if this term is equal to the
    argument, and a positive integer if this term belongs after the argument.

    The ordering of terms is first by field, then by text.*/
  virtual int compareTo(std::shared_ptr<Term> other);

  /**
   * Resets the field and text of a Term.
   * <p>WARNING: the provided BytesRef is not copied, but used directly.
   * Therefore the bytes should not be modified after construction, for
   * example, you should clone a copy rather than pass reused bytes from
   * a TermsEnum.
   */
  void set(const std::wstring &fld, std::shared_ptr<BytesRef> bytes);

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/index/
