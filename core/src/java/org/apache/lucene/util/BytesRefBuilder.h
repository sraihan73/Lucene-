#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::util
{

/**
 * A builder for {@link BytesRef} instances.
 * @lucene.internal
 */
class BytesRefBuilder : public std::enable_shared_from_this<BytesRefBuilder>
{
  GET_CLASS_NAME(BytesRefBuilder)

private:
  const std::shared_ptr<BytesRef> ref;

  /** Sole constructor. */
public:
  BytesRefBuilder();

  /** Return a reference to the bytes of this builder. */
  virtual std::deque<char> bytes();

  /** Return the number of bytes in this buffer. */
  virtual int length();

  /** Set the length. */
  virtual void setLength(int length);

  /** Return the byte at the given offset. */
  virtual char byteAt(int offset);

  /** Set a byte. */
  virtual void setByteAt(int offset, char b);

  /**
   * Ensure that this builder can hold at least <code>capacity</code> bytes
   * without resizing.
   */
  virtual void grow(int capacity);

  /**
   * Append a single byte to this builder.
   */
  virtual void append(char b);

  /**
   * Append the provided bytes to this builder.
   */
  virtual void append(std::deque<char> &b, int off, int len);

  /**
   * Append the provided bytes to this builder.
   */
  virtual void append(std::shared_ptr<BytesRef> ref);

  /**
   * Append the provided bytes to this builder.
   */
  virtual void append(std::shared_ptr<BytesRefBuilder> builder);

  /**
   * Reset this builder to the empty state.
   */
  virtual void clear();

  /**
   * Replace the content of this builder with the provided bytes. Equivalent to
   * calling {@link #clear()} and then {@link #append(byte[], int, int)}.
   */
  virtual void copyBytes(std::deque<char> &b, int off, int len);

  /**
   * Replace the content of this builder with the provided bytes. Equivalent to
   * calling {@link #clear()} and then {@link #append(BytesRef)}.
   */
  virtual void copyBytes(std::shared_ptr<BytesRef> ref);

  /**
   * Replace the content of this builder with the provided bytes. Equivalent to
   * calling {@link #clear()} and then {@link #append(BytesRefBuilder)}.
   */
  virtual void copyBytes(std::shared_ptr<BytesRefBuilder> builder);

  /**
   * Replace the content of this buffer with UTF-8 encoded bytes that would
   * represent the provided text.
   */
  virtual void copyChars(std::shared_ptr<std::wstring> text);

  /**
   * Replace the content of this buffer with UTF-8 encoded bytes that would
   * represent the provided text.
   */
  virtual void copyChars(std::shared_ptr<std::wstring> text, int off, int len);

  /**
   * Replace the content of this buffer with UTF-8 encoded bytes that would
   * represent the provided text.
   */
  virtual void copyChars(std::deque<wchar_t> &text, int off, int len);

  /**
   * Return a {@link BytesRef} that points to the internal content of this
   * builder. Any update to the content of this builder might invalidate
   * the provided <code>ref</code> and vice-versa.
   */
  virtual std::shared_ptr<BytesRef> get();

  /**
   * Build a new {@link BytesRef} that has the same content as this buffer.
   */
  virtual std::shared_ptr<BytesRef> toBytesRef();

  virtual bool equals(std::any obj);

  virtual int hashCode();
};

} // namespace org::apache::lucene::util
