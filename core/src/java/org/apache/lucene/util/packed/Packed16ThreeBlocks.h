#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataInput;
}

// This file has been automatically generated, DO NOT EDIT

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
namespace org::apache::lucene::util::packed
{

using DataInput = org::apache::lucene::store::DataInput;

/**
 * Packs integers into 3 shorts (48 bits per value).
 * @lucene.internal
 */
class Packed16ThreeBlocks final : public PackedInts::MutableImpl
{
  GET_CLASS_NAME(Packed16ThreeBlocks)
public:
  std::deque<short> const blocks;

  static const int MAX_SIZE = std::numeric_limits<int>::max() / 3;

  Packed16ThreeBlocks(int valueCount);

  Packed16ThreeBlocks(int packedIntsVersion, std::shared_ptr<DataInput> in_,
                      int valueCount) ;

  int64_t get(int index) override;

  int get(int index, std::deque<int64_t> &arr, int off, int len) override;

  void set(int index, int64_t value) override;

  int set(int index, std::deque<int64_t> &arr, int off, int len) override;

  void fill(int fromIndex, int toIndex, int64_t val) override;

  void clear() override;

  int64_t ramBytesUsed() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<Packed16ThreeBlocks> shared_from_this()
  {
    return std::static_pointer_cast<Packed16ThreeBlocks>(
        PackedInts.MutableImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::packed
