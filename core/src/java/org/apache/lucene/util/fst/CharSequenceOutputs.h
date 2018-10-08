#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class CharsRef;
}

namespace org::apache::lucene::store
{
class DataOutput;
}
namespace org::apache::lucene::store
{
class DataInput;
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
namespace org::apache::lucene::util::fst
{

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using CharsRef = org::apache::lucene::util::CharsRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/**
 * An FST {@link Outputs} implementation where each output
 * is a sequence of characters.
 *
 * @lucene.experimental
 */

class CharSequenceOutputs final : public Outputs<std::shared_ptr<CharsRef>>
{
  GET_CLASS_NAME(CharSequenceOutputs)

private:
  static const std::shared_ptr<CharsRef> NO_OUTPUT;
  static const std::shared_ptr<CharSequenceOutputs> singleton;

  CharSequenceOutputs();

public:
  static std::shared_ptr<CharSequenceOutputs> getSingleton();

  std::shared_ptr<CharsRef> common(std::shared_ptr<CharsRef> output1,
                                   std::shared_ptr<CharsRef> output2) override;

  std::shared_ptr<CharsRef> subtract(std::shared_ptr<CharsRef> output,
                                     std::shared_ptr<CharsRef> inc) override;

  std::shared_ptr<CharsRef> add(std::shared_ptr<CharsRef> prefix,
                                std::shared_ptr<CharsRef> output) override;

  void write(std::shared_ptr<CharsRef> prefix,
             std::shared_ptr<DataOutput> out)  override;

  std::shared_ptr<CharsRef>
  read(std::shared_ptr<DataInput> in_)  override;

  void skipOutput(std::shared_ptr<DataInput> in_)  override;

  std::shared_ptr<CharsRef> getNoOutput() override;

  std::wstring outputToString(std::shared_ptr<CharsRef> output) override;

private:
  static const int64_t BASE_NUM_BYTES =
      RamUsageEstimator::shallowSizeOf(NO_OUTPUT);

public:
  int64_t ramBytesUsed(std::shared_ptr<CharsRef> output) override;

protected:
  std::shared_ptr<CharSequenceOutputs> shared_from_this()
  {
    return std::static_pointer_cast<CharSequenceOutputs>(
        Outputs<org.apache.lucene.util.CharsRef>::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::fst
