#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::packed
{
class PackedInts;
}

namespace org::apache::lucene::util::packed
{
class Mutable;
}
namespace org::apache::lucene::store
{
class DataOutput;
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
namespace org::apache::lucene::util::packed
{

using DataOutput = org::apache::lucene::store::DataOutput;

/**
 * Implements {@link PackedInts.Mutable}, but grows the
 * bit count of the underlying packed ints on-demand.
 * <p>Beware that this class will accept to set negative values but in order
 * to do this, it will grow the number of bits per value to 64.
 *
 * <p>@lucene.internal</p>
 */
class GrowableWriter : public PackedInts::Mutable
{
  GET_CLASS_NAME(GrowableWriter)

private:
  int64_t currentMask = 0;
  std::shared_ptr<PackedInts::Mutable> current;
  const float acceptableOverheadRatio;

  /**
   * @param startBitsPerValue       the initial number of bits per value, may
   * grow depending on the data
   * @param valueCount              the number of values
   * @param acceptableOverheadRatio an acceptable overhead ratio
   */
public:
  GrowableWriter(int startBitsPerValue, int valueCount,
                 float acceptableOverheadRatio);

private:
  static int64_t mask(int bitsPerValue);

public:
  int64_t get(int index) override;

  int size() override;

  int getBitsPerValue() override;

  virtual std::shared_ptr<PackedInts::Mutable> getMutable();

private:
  void ensureCapacity(int64_t value);

public:
  void set(int index, int64_t value) override;

  void clear() override;

  virtual std::shared_ptr<GrowableWriter> resize(int newSize);

  int get(int index, std::deque<int64_t> &arr, int off, int len) override;

  int set(int index, std::deque<int64_t> &arr, int off, int len) override;

  void fill(int fromIndex, int toIndex, int64_t val) override;

  int64_t ramBytesUsed() override;

  void save(std::shared_ptr<DataOutput> out)  override;

protected:
  std::shared_ptr<GrowableWriter> shared_from_this()
  {
    return std::static_pointer_cast<GrowableWriter>(
        PackedInts.Mutable::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::packed
