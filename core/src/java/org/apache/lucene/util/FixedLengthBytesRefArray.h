#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/util/BytesRefComparator.h"
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
 * Just like {@link BytesRefArray} except all values have the same length.
 *
 * <b>Note: This class is not Thread-Safe!</b>
 *
 * @lucene.internal
 * @lucene.experimental
 */
class FixedLengthBytesRefArray final
    : public std::enable_shared_from_this<FixedLengthBytesRefArray>,
      public SortableBytesRefArray
{
  GET_CLASS_NAME(FixedLengthBytesRefArray)
private:
  const int valueLength;
  const int valuesPerBlock;

  /** How many values have been appended */
  // C++ NOTE: Fields cannot have the same name as methods:
  int size_ = 0;

  /** How many blocks are used */
  int currentBlock = -1;
  int nextEntry = 0;

  std::deque<std::deque<char>> blocks;

  /**
   * Creates a new {@link BytesRefArray} with a counter to track allocated bytes
   */
public:
  FixedLengthBytesRefArray(int valueLength);

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
   * Returns the current size of this {@link FixedLengthBytesRefArray}
   * @return the current size of this {@link FixedLengthBytesRefArray}
   */
  int size() override;

private:
  std::deque<int>
  sort(std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp);

private:
  class MSBRadixSorterAnonymousInnerClass : public MSBRadixSorter
  {
    GET_CLASS_NAME(MSBRadixSorterAnonymousInnerClass)
  private:
    std::shared_ptr<FixedLengthBytesRefArray> outerInstance;

    std::deque<int> orderedEntries;
    std::shared_ptr<org::apache::lucene::util::BytesRefComparator> bComp;

  public:
    MSBRadixSorterAnonymousInnerClass(
        std::shared_ptr<FixedLengthBytesRefArray> outerInstance,
        int comparedBytesCount, std::deque<int> &orderedEntries,
        std::shared_ptr<org::apache::lucene::util::BytesRefComparator> bComp);

    std::shared_ptr<BytesRef> scratch;

  protected:
    void swap(int i, int j) override;

    int byteAt(int i, int k) override;

  protected:
    std::shared_ptr<MSBRadixSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<MSBRadixSorterAnonymousInnerClass>(
          MSBRadixSorter::shared_from_this());
    }
  };

private:
  class IntroSorterAnonymousInnerClass : public IntroSorter
  {
    GET_CLASS_NAME(IntroSorterAnonymousInnerClass)
  private:
    std::shared_ptr<FixedLengthBytesRefArray> outerInstance;

    std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp;
    std::deque<int> orderedEntries;
    std::shared_ptr<org::apache::lucene::util::BytesRef> pivot;
    std::shared_ptr<org::apache::lucene::util::BytesRef> scratch1;
    std::shared_ptr<org::apache::lucene::util::BytesRef> scratch2;

  public:
    IntroSorterAnonymousInnerClass(
        std::shared_ptr<FixedLengthBytesRefArray> outerInstance,
        std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp,
        std::deque<int> &orderedEntries,
        std::shared_ptr<org::apache::lucene::util::BytesRef> pivot,
        std::shared_ptr<org::apache::lucene::util::BytesRef> scratch1,
        std::shared_ptr<org::apache::lucene::util::BytesRef> scratch2);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

    void setPivot(int i) override;

    int comparePivot(int j) override;

  protected:
    std::shared_ptr<IntroSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntroSorterAnonymousInnerClass>(
          IntroSorter::shared_from_this());
    }
  };

  /**
   * <p>
   * Returns a {@link BytesRefIterator} with point in time semantics. The
   * iterator provides access to all so far appended {@link BytesRef} instances.
   * </p>
   * <p>
   * The iterator will iterate the byte values in the order specified by the
   * comparator.
   * </p>
   * <p>
   * This is a non-destructive operation.
   * </p>
   */
public:
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
    std::shared_ptr<FixedLengthBytesRefArray> outerInstance;

    std::shared_ptr<org::apache::lucene::util::BytesRef> result;
    int size = 0;
    std::deque<int> indices;

  public:
    BytesRefIteratorAnonymousInnerClass(
        std::shared_ptr<FixedLengthBytesRefArray> outerInstance,
        std::shared_ptr<org::apache::lucene::util::BytesRef> result, int size,
        std::deque<int> &indices);

    int pos = 0;

    std::shared_ptr<BytesRef> next() override;
  };
};

} // #include  "core/src/java/org/apache/lucene/util/
