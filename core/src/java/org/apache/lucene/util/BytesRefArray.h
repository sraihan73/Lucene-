#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/ByteBlockPool.h"

#include  "core/src/java/org/apache/lucene/util/Counter.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefIterator.h"

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
 * A simple append only random-access {@link BytesRef} array that stores full
 * copies of the appended bytes in a {@link ByteBlockPool}.
 *
 *
 * <b>Note: This class is not Thread-Safe!</b>
 *
 * @lucene.internal
 * @lucene.experimental
 */
class BytesRefArray final : public std::enable_shared_from_this<BytesRefArray>,
                            public SortableBytesRefArray
{
  GET_CLASS_NAME(BytesRefArray)
private:
  const std::shared_ptr<ByteBlockPool> pool;
  std::deque<int> offsets = std::deque<int>(1);
  int lastElement = 0;
  int currentOffset = 0;
  const std::shared_ptr<Counter> bytesUsed;

  /**
   * Creates a new {@link BytesRefArray} with a counter to track allocated bytes
   */
public:
  BytesRefArray(std::shared_ptr<Counter> bytesUsed);

  /**
   * Clears this {@link BytesRefArray}
   */
  void clear() override;

  /**
   * Appends a copy of the given {@link BytesRef} to this {@link BytesRefArray}.
   * @param bytes the bytes to append
   * @return the index of the appended bytes
   */
  int append(std::shared_ptr<BytesRef> bytes) override;

  /**
   * Returns the current size of this {@link BytesRefArray}
   * @return the current size of this {@link BytesRefArray}
   */
  int size() override;

  /**
   * Returns the <i>n'th</i> element of this {@link BytesRefArray}
   * @param spare a spare {@link BytesRef} instance
   * @param index the elements index to retrieve
   * @return the <i>n'th</i> element of this {@link BytesRefArray}
   */
  std::shared_ptr<BytesRef> get(std::shared_ptr<BytesRefBuilder> spare,
                                int index);

  /** Used only by sort below, to set a {@link BytesRef} with the specified
   * slice, avoiding copying bytes in the common case when the slice is
   * contained in a single block in the byte block pool. */
private:
  void setBytesRef(std::shared_ptr<BytesRefBuilder> spare,
                   std::shared_ptr<BytesRef> result, int index);

  std::deque<int>
  sort(std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp);

private:
  class IntroSorterAnonymousInnerClass : public IntroSorter
  {
    GET_CLASS_NAME(IntroSorterAnonymousInnerClass)
  private:
    std::shared_ptr<BytesRefArray> outerInstance;

    std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp;
    std::deque<int> orderedEntries;

  public:
    IntroSorterAnonymousInnerClass(
        std::shared_ptr<BytesRefArray> outerInstance,
        std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp,
        std::deque<int> &orderedEntries);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

    void setPivot(int i) override;

    int comparePivot(int j) override;

  private:
    const std::shared_ptr<BytesRef> pivot;
    const std::shared_ptr<BytesRef> scratchBytes1;
    const std::shared_ptr<BytesRef> scratchBytes2;
    const std::shared_ptr<BytesRefBuilder> pivotBuilder;
    const std::shared_ptr<BytesRefBuilder> scratch1;
    const std::shared_ptr<BytesRefBuilder> scratch2;

  protected:
    std::shared_ptr<IntroSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntroSorterAnonymousInnerClass>(
          IntroSorter::shared_from_this());
    }
  };

  /**
   * sugar for {@link #iterator(Comparator)} with a <code>null</code> comparator
   */
public:
  std::shared_ptr<BytesRefIterator> iterator();

  /**
   * <p>
   * Returns a {@link BytesRefIterator} with point in time semantics. The
   * iterator provides access to all so far appended {@link BytesRef} instances.
   * </p>
   * <p>
   * If a non <code>null</code> {@link Comparator} is provided the iterator will
   * iterate the byte values in the order specified by the comparator. Otherwise
   * the order is the same as the values were appended.
   * </p>
   * <p>
   * This is a non-destructive operation.
   * </p>
   */
  std::shared_ptr<BytesRefIterator> iterator(
      std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp) override;

private:
  class BytesRefIteratorAnonymousInnerClass
      : public std::enable_shared_from_this<
            BytesRefIteratorAnonymousInnerClass>,
        public BytesRefIterator
  {
    GET_CLASS_NAME(BytesRefIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<BytesRefArray> outerInstance;

    std::shared_ptr<org::apache::lucene::util::BytesRefBuilder> spare;
    std::shared_ptr<org::apache::lucene::util::BytesRef> result;
    int size = 0;
    std::deque<int> indices;

  public:
    BytesRefIteratorAnonymousInnerClass(
        std::shared_ptr<BytesRefArray> outerInstance,
        std::shared_ptr<org::apache::lucene::util::BytesRefBuilder> spare,
        std::shared_ptr<org::apache::lucene::util::BytesRef> result, int size,
        std::deque<int> &indices);

    int pos = 0;

    std::shared_ptr<BytesRef> next() override;
  };
};

} // #include  "core/src/java/org/apache/lucene/util/
