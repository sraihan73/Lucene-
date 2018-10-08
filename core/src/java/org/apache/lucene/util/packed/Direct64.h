#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/DataInput.h"

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
 * Direct wrapping of 64-bits values to a backing array.
 * @lucene.internal
 */
class Direct64 final : public PackedInts::MutableImpl
{
  GET_CLASS_NAME(Direct64)
public:
  std::deque<int64_t> const values;

  Direct64(int valueCount);

  Direct64(int packedIntsVersion, std::shared_ptr<DataInput> in_,
           int valueCount) ;

  int64_t get(int const index) override;

  void set(int const index, int64_t const value) override;

  int64_t ramBytesUsed() override;

  void clear() override;

  int get(int index, std::deque<int64_t> &arr, int off, int len) override;

  int set(int index, std::deque<int64_t> &arr, int off, int len) override;

  void fill(int fromIndex, int toIndex, int64_t val) override;

protected:
  std::shared_ptr<Direct64> shared_from_this()
  {
    return std::static_pointer_cast<Direct64>(
        PackedInts.MutableImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/packed/
