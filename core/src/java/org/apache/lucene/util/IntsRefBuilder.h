#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"

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
 * A builder for {@link IntsRef} instances.
 * @lucene.internal
 */
class IntsRefBuilder : public std::enable_shared_from_this<IntsRefBuilder>
{
  GET_CLASS_NAME(IntsRefBuilder)

private:
  const std::shared_ptr<IntsRef> ref;

  /** Sole constructor. */
public:
  IntsRefBuilder();

  /** Return a reference to the ints of this builder. */
  virtual std::deque<int> ints();

  /** Return the number of ints in this buffer. */
  virtual int length();

  /** Set the length. */
  virtual void setLength(int length);

  /** Empty this builder. */
  virtual void clear();

  /** Return the int at the given offset. */
  virtual int intAt(int offset);

  /** Set an int. */
  virtual void setIntAt(int offset, int b);

  /** Append the provided int to this buffer. */
  virtual void append(int i);

  /**
   * Used to grow the reference array.
   *
   * In general this should not be used as it does not take the offset into
   * account.
   * @lucene.internal */
  virtual void grow(int newLength);

  /**
   * Copies the given array into this instance.
   */
  virtual void copyInts(std::deque<int> &otherInts, int otherOffset,
                        int otherLength);

  /**
   * Copies the given array into this instance.
   */
  virtual void copyInts(std::shared_ptr<IntsRef> ints);

  /**
   * Copy the given UTF-8 bytes into this builder. Works as if the bytes were
   * first converted from UTF-8 to UTF-32 and then copied into this builder.
   */
  virtual void copyUTF8Bytes(std::shared_ptr<BytesRef> bytes);

  /**
   * Return a {@link IntsRef} that points to the internal content of this
   * builder. Any update to the content of this builder might invalidate
   * the provided <code>ref</code> and vice-versa.
   */
  virtual std::shared_ptr<IntsRef> get();

  /** Build a new {@link CharsRef} that has the same content as this builder. */
  virtual std::shared_ptr<IntsRef> toIntsRef();

  bool equals(std::any obj) override;

  virtual int hashCode();
};

} // #include  "core/src/java/org/apache/lucene/util/
