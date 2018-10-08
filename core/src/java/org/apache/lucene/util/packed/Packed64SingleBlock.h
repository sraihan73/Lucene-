#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/packed/PackedInts.h"

#include  "core/src/java/org/apache/lucene/store/DataInput.h"

// This file has been automatically generated, DO NOT EDIT

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements. See the NOTICE file distributed with this
 * work for additional information regarding copyright ownership. sraihan.com
 * licenses this file to You under GPLv3 License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
namespace org::apache::lucene::util::packed
{

using DataInput = org::apache::lucene::store::DataInput;

/**
 * This class is similar to {@link Packed64} except that it trades space for
 * speed by ensuring that a single block needs to be read/written in order to
 * read/write a value.
 */
class Packed64SingleBlock : public PackedInts::MutableImpl
{
  GET_CLASS_NAME(Packed64SingleBlock)

public:
  static constexpr int MAX_SUPPORTED_BITS_PER_VALUE = 32;

private:
  static std::deque<int> const SUPPORTED_BITS_PER_VALUE;

public:
  static bool isSupported(int bitsPerValue);

private:
  static int requiredCapacity(int valueCount, int valuesPerBlock);

public:
  std::deque<int64_t> const blocks;

  Packed64SingleBlock(int valueCount, int bitsPerValue);

  void clear() override;

  int64_t ramBytesUsed() override;

  int get(int index, std::deque<int64_t> &arr, int off, int len) override;

  int set(int index, std::deque<int64_t> &arr, int off, int len) override;

  void fill(int fromIndex, int toIndex, int64_t val) override;

protected:
  PackedInts::Format getFormat() override;

public:
  virtual std::wstring toString();

  static std::shared_ptr<Packed64SingleBlock>
  create(std::shared_ptr<DataInput> in_, int valueCount,
         int bitsPerValue) ;

  static std::shared_ptr<Packed64SingleBlock> create(int valueCount,
                                                     int bitsPerValue);

public:
  class Packed64SingleBlock1;

public:
  class Packed64SingleBlock2;

public:
  class Packed64SingleBlock3;

public:
  class Packed64SingleBlock4;

public:
  class Packed64SingleBlock5;

public:
  class Packed64SingleBlock6;

public:
  class Packed64SingleBlock7;

public:
  class Packed64SingleBlock8;

public:
  class Packed64SingleBlock9;

public:
  class Packed64SingleBlock10;

public:
  class Packed64SingleBlock12;

public:
  class Packed64SingleBlock16;

public:
  class Packed64SingleBlock21;

public:
  class Packed64SingleBlock32;

protected:
  std::shared_ptr<Packed64SingleBlock> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock>(
        PackedInts.MutableImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::packed
class Packed64SingleBlock::Packed64SingleBlock1 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock1)

public:
  Packed64SingleBlock1(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock1> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock1>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock2 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock2)

public:
  Packed64SingleBlock2(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock2> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock2>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock3 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock3)

public:
  Packed64SingleBlock3(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock3> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock3>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock4 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock4)

public:
  Packed64SingleBlock4(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock4> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock4>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock5 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock5)

public:
  Packed64SingleBlock5(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock5> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock5>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock6 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock6)

public:
  Packed64SingleBlock6(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock6> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock6>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock7 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock7)

public:
  Packed64SingleBlock7(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock7> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock7>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock8 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock8)

public:
  Packed64SingleBlock8(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock8> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock8>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock9 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock9)

public:
  Packed64SingleBlock9(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock9> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock9>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock10 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock10)

public:
  Packed64SingleBlock10(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock10> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock10>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock12 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock12)

public:
  Packed64SingleBlock12(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock12> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock12>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock16 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock16)

public:
  Packed64SingleBlock16(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock16> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock16>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock21 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock21)

public:
  Packed64SingleBlock21(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock21> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock21>(
        Packed64SingleBlock::shared_from_this());
  }
};
class Packed64SingleBlock::Packed64SingleBlock32 : public Packed64SingleBlock
{
  GET_CLASS_NAME(Packed64SingleBlock::Packed64SingleBlock32)

public:
  Packed64SingleBlock32(int valueCount);

  int64_t get(int index) override;

  void set(int index, int64_t value) override;

protected:
  std::shared_ptr<Packed64SingleBlock32> shared_from_this()
  {
    return std::static_pointer_cast<Packed64SingleBlock32>(
        Packed64SingleBlock::shared_from_this());
  }
};
