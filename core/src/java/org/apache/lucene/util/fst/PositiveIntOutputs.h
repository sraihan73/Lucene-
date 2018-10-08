#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"

#include  "core/src/java/org/apache/lucene/store/DataInput.h"

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

/**
 * An FST {@link Outputs} implementation where each output
 * is a non-negative long value.
 *
 * @lucene.experimental
 */

class PositiveIntOutputs final : public Outputs<int64_t>
{
  GET_CLASS_NAME(PositiveIntOutputs)

private:
  static const std::optional<int64_t> NO_OUTPUT;

  static const std::shared_ptr<PositiveIntOutputs> singleton;

  PositiveIntOutputs();

public:
  static std::shared_ptr<PositiveIntOutputs> getSingleton();

  std::optional<int64_t> common(std::optional<int64_t> &output1,
                                  std::optional<int64_t> &output2) override;

  std::optional<int64_t> subtract(std::optional<int64_t> &output,
                                    std::optional<int64_t> &inc) override;

  std::optional<int64_t> add(std::optional<int64_t> &prefix,
                               std::optional<int64_t> &output) override;

  void write(std::optional<int64_t> &output,
             std::shared_ptr<DataOutput> out)  override;

  std::optional<int64_t>
  read(std::shared_ptr<DataInput> in_)  override;

private:
  bool valid(std::optional<int64_t> &o);

public:
  std::optional<int64_t> getNoOutput() override;

  std::wstring outputToString(std::optional<int64_t> &output) override;

  virtual std::wstring toString();

  int64_t ramBytesUsed(std::optional<int64_t> &output) override;

protected:
  std::shared_ptr<PositiveIntOutputs> shared_from_this()
  {
    return std::static_pointer_cast<PositiveIntOutputs>(
        Outputs<long>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/fst/
