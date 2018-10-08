#pragma once
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::util::fst
{

using IntsRef = org::apache::lucene::util::IntsRef;

/** Enumerates all input (IntsRef) + output pairs in an
 *  FST.
 *
 * @lucene.experimental
 */

template <typename T>
class IntsRefFSTEnum final : public FSTEnum<T>
{
  GET_CLASS_NAME(IntsRefFSTEnum)
private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<IntsRef> current_ = std::make_shared<IntsRef>(10);
  const std::shared_ptr<InputOutput<T>> result =
      std::make_shared<InputOutput<T>>();
  std::shared_ptr<IntsRef> target;

  /** Holds a single input (IntsRef) + output pair. */
public:
  template <typename T>
  class InputOutput : public std::enable_shared_from_this<InputOutput>
  {
    GET_CLASS_NAME(InputOutput)
  public:
    std::shared_ptr<IntsRef> input;
    T output;
  };

  /** doFloor controls the behavior of advance: if it's true
   *  doFloor is true, advance positions to the biggest
   *  term before target.  */
public:
  IntsRefFSTEnum(std::shared_ptr<FST<T>> fst) : FSTEnum<T>(fst)
  {
    result->input = current_;
    current_->offset = 1;
  }

  std::shared_ptr<InputOutput<T>> current() { return result; }

  std::shared_ptr<InputOutput<T>> next() 
  {
    // System.out.println("  enum.next");
    doNext();
    return setResult();
  }

  /** Seeks to smallest term that's &gt;= target. */
  std::shared_ptr<InputOutput<T>>
  seekCeil(std::shared_ptr<IntsRef> target) 
  {
    this->target = target;
    targetLength = target->length;
    FSTEnum<T>::doSeekCeil();
    return setResult();
  }

  /** Seeks to biggest term that's &lt;= target. */
  std::shared_ptr<InputOutput<T>>
  seekFloor(std::shared_ptr<IntsRef> target) 
  {
    this->target = target;
    targetLength = target->length;
    FSTEnum<T>::doSeekFloor();
    return setResult();
  }

  /** Seeks to exactly this term, returning null if the term
   *  doesn't exist.  This is faster than using {@link
   *  #seekFloor} or {@link #seekCeil} because it
   *  short-circuits as soon the match is not found. */
  std::shared_ptr<InputOutput<T>>
  seekExact(std::shared_ptr<IntsRef> target) 
  {
    this->target = target;
    targetLength = target->length;
    if (FSTEnum<T>::doSeekExact()) {
      assert(upto == 1 + target->length);
      return setResult();
    } else {
      return nullptr;
    }
  }

protected:
  int getTargetLabel() override
  {
    if (upto - 1 == target->length) {
      return FST::END_LABEL;
    } else {
      return target->ints[target->offset + upto - 1];
    }
  }

  int getCurrentLabel() override
  {
    // current.offset fixed at 1
    return current_->ints[upto];
  }

  void setCurrentLabel(int label) override { current_->ints[upto] = label; }

  void grow() override
  {
    current_->ints = ArrayUtil::grow(current_->ints, upto + 1);
  }

private:
  std::shared_ptr<InputOutput<T>> setResult()
  {
    if (upto == 0) {
      return nullptr;
    } else {
      current_->length = upto - 1;
      result->output = output[upto];
      return result;
    }
  }

protected:
  std::shared_ptr<IntsRefFSTEnum> shared_from_this()
  {
    return std::static_pointer_cast<IntsRefFSTEnum>(
        FSTEnum<T>::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::fst
