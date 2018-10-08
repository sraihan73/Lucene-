#pragma once
#include "stringhelper.h"
#include <memory>
#include <type_traits>
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

/** Acts like forever growing T[], but internally uses a
 *  circular buffer to reuse instances of T.
 *
 *  @lucene.internal */
template <typename T>
class RollingBuffer : public std::enable_shared_from_this<RollingBuffer>
{
  GET_CLASS_NAME(RollingBuffer)
  static_assert(std::is_base_of<RollingBuffer.Resettable, T>::value,
                L"T must inherit from RollingBuffer.Resettable");

  /**
   * Implement to reset an instance
   */
public:
  class Resettable
  {
    GET_CLASS_NAME(Resettable)
  public:
    virtual void reset() = 0;
  };

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") private T[] buffer = (T[])
  // new RollingBuffer.Resettable[8];
  std::deque<T> buffer = static_cast<std::deque<T>>(
      std::deque<std::shared_ptr<RollingBuffer::Resettable>>(8));

  // Next array index to write to:
  int nextWrite = 0;

  // Next position to write:
  int nextPos = 0;

  // How many valid Position are held in the
  // array:
  int count = 0;

public:
  RollingBuffer()
  {
    for (int idx = 0; idx < buffer.size(); idx++) {
      buffer[idx] = newInstance();
    }
  }

protected:
  virtual T newInstance() = 0;

public:
  virtual void reset()
  {
    nextWrite--;
    while (count > 0) {
      if (nextWrite == -1) {
        nextWrite = buffer.size() - 1;
      }
      buffer[nextWrite--]->reset();
      count--;
    }
    nextWrite = 0;
    nextPos = 0;
    count = 0;
  }

  // For assert:
private:
  bool inBounds(int pos) { return pos < nextPos && pos >= nextPos - count; }

  int getIndex(int pos)
  {
    int index = nextWrite - (nextPos - pos);
    if (index < 0) {
      index += buffer.size();
    }
    return index;
  }

  /** Get T instance for this absolute position;
   *  this is allowed to be arbitrarily far "in the
   *  future" but cannot be before the last freeBefore. */
public:
  virtual T get(int pos)
  {
    // System.out.println("RA.get pos=" + pos + " nextPos=" + nextPos + "
    // nextWrite=" + nextWrite + " count=" + count);
    while (pos >= nextPos) {
      if (count == buffer.size()) {
        // C++ TODO: Most Java annotations will not have direct C++ equivalents:
        // ORIGINAL LINE: @SuppressWarnings("unchecked") T[] newBuffer = (T[])
        // new Resettable[ArrayUtil.oversize(1+count,
        // RamUsageEstimator.NUM_BYTES_OBJECT_REF)];
        std::deque<T> newBuffer = static_cast<std::deque<T>>(
            std::deque<std::shared_ptr<Resettable>>(ArrayUtil::oversize(
                1 + count, RamUsageEstimator::NUM_BYTES_OBJECT_REF)));
        // System.out.println("  grow length=" + newBuffer.length);
        System::arraycopy(buffer, nextWrite, newBuffer, 0,
                          buffer.size() - nextWrite);
        System::arraycopy(buffer, 0, newBuffer, buffer.size() - nextWrite,
                          nextWrite);
        for (int i = buffer.size(); i < newBuffer.size(); i++) {
          newBuffer[i] = newInstance();
        }
        nextWrite = buffer.size();
        buffer = newBuffer;
      }
      if (nextWrite == buffer.size()) {
        nextWrite = 0;
      }
      // Should have already been reset:
      nextWrite++;
      nextPos++;
      count++;
    }
    assert((inBounds(pos), L"pos=" + std::to_wstring(pos) + L" nextPos=" +
                               std::to_wstring(nextPos) + L" count=" +
                               std::to_wstring(count)));
    constexpr int index = getIndex(pos);
    // System.out.println("  pos=" + pos + " nextPos=" + nextPos + " -> index="
    // + index); assert buffer[index].pos == pos;
    return buffer[index];
  }

  /** Returns the maximum position looked up, or -1 if no
   *   position has been looked up since reset/init.  */
  virtual int getMaxPos() { return nextPos - 1; }

  /** Returns how many active positions are in the buffer. */
  virtual int getBufferSize() { return count; }

  virtual void freeBefore(int pos)
  {
    constexpr int toFree = count - (nextPos - pos);
    assert(toFree >= 0);
    assert((toFree <= count, L"toFree=" + std::to_wstring(toFree) + L" count=" +
                                 std::to_wstring(count)));
    int index = nextWrite - count;
    if (index < 0) {
      index += buffer.size();
    }
    for (int i = 0; i < toFree; i++) {
      if (index == buffer.size()) {
        index = 0;
      }
      // System.out.println("  fb idx=" + index);
      buffer[index]->reset();
      index++;
    }
    count -= toFree;
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
