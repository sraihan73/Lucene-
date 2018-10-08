#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/CharsRef.h"

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

/**
 * A builder for {@link CharsRef} instances.
 * @lucene.internal
 */
class CharsRefBuilder : public std::enable_shared_from_this<CharsRefBuilder>,
                        public Appendable
{
  GET_CLASS_NAME(CharsRefBuilder)

private:
  static const std::wstring NULL_STRING;

  const std::shared_ptr<CharsRef> ref;

  /** Sole constructor. */
public:
  CharsRefBuilder();

  /** Return a reference to the chars of this builder. */
  virtual std::deque<wchar_t> chars();

  /** Return the number of chars in this buffer. */
  virtual int length();

  /** Set the length. */
  virtual void setLength(int length);

  /** Return the char at the given offset. */
  virtual wchar_t charAt(int offset);

  /** Set a char. */
  virtual void setCharAt(int offset, wchar_t b);

  /**
   * Reset this builder to the empty state.
   */
  virtual void clear();

  std::shared_ptr<CharsRefBuilder>
  append(std::shared_ptr<std::wstring> csq) override;

  std::shared_ptr<CharsRefBuilder> append(std::shared_ptr<std::wstring> csq,
                                          int start, int end) override;

  std::shared_ptr<CharsRefBuilder> append(wchar_t c) override;

  /**
   * Copies the given {@link CharsRef} referenced content into this instance.
   */
  virtual void copyChars(std::shared_ptr<CharsRef> other);

  /**
   * Used to grow the reference array.
   */
  virtual void grow(int newLength);

  /**
   * Copy the provided bytes, interpreted as UTF-8 bytes.
   */
  virtual void copyUTF8Bytes(std::deque<char> &bytes, int offset, int length);

  /**
   * Copy the provided bytes, interpreted as UTF-8 bytes.
   */
  virtual void copyUTF8Bytes(std::shared_ptr<BytesRef> bytes);

  /**
   * Copies the given array into this instance.
   */
  virtual void copyChars(std::deque<wchar_t> &otherChars, int otherOffset,
                         int otherLength);

  /**
   * Appends the given array to this CharsRef
   */
  virtual void append(std::deque<wchar_t> &otherChars, int otherOffset,
                      int otherLength);

  /**
   * Return a {@link CharsRef} that points to the internal content of this
   * builder. Any update to the content of this builder might invalidate
   * the provided <code>ref</code> and vice-versa.
   */
  virtual std::shared_ptr<CharsRef> get();

  /** Build a new {@link CharsRef} that has the same content as this builder. */
  virtual std::shared_ptr<CharsRef> toCharsRef();

  virtual std::wstring toString();

  bool equals(std::any obj) override;

  virtual int hashCode();
};

} // #include  "core/src/java/org/apache/lucene/util/
