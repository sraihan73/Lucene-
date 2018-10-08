#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class IntsRef;
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
using IntsRef = org::apache::lucene::util::IntsRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/**
 * An FST {@link Outputs} implementation where each output
 * is a sequence of ints.
 *
 * @lucene.experimental
 */

class IntSequenceOutputs final : public Outputs<std::shared_ptr<IntsRef>>
{
  GET_CLASS_NAME(IntSequenceOutputs)

private:
  static const std::shared_ptr<IntsRef> NO_OUTPUT;
  static const std::shared_ptr<IntSequenceOutputs> singleton;

  IntSequenceOutputs();

public:
  static std::shared_ptr<IntSequenceOutputs> getSingleton();

  std::shared_ptr<IntsRef> common(std::shared_ptr<IntsRef> output1,
                                  std::shared_ptr<IntsRef> output2) override;

  std::shared_ptr<IntsRef> subtract(std::shared_ptr<IntsRef> output,
                                    std::shared_ptr<IntsRef> inc) override;

  std::shared_ptr<IntsRef> add(std::shared_ptr<IntsRef> prefix,
                               std::shared_ptr<IntsRef> output) override;

  void write(std::shared_ptr<IntsRef> prefix,
             std::shared_ptr<DataOutput> out)  override;

  std::shared_ptr<IntsRef>
  read(std::shared_ptr<DataInput> in_)  override;

  void skipOutput(std::shared_ptr<DataInput> in_)  override;

  std::shared_ptr<IntsRef> getNoOutput() override;

  std::wstring outputToString(std::shared_ptr<IntsRef> output) override;

private:
  static const int64_t BASE_NUM_BYTES =
      RamUsageEstimator::shallowSizeOf(NO_OUTPUT);

public:
  int64_t ramBytesUsed(std::shared_ptr<IntsRef> output) override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<IntSequenceOutputs> shared_from_this()
  {
    return std::static_pointer_cast<IntSequenceOutputs>(
        Outputs<org.apache.lucene.util.IntsRef>::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::fst
