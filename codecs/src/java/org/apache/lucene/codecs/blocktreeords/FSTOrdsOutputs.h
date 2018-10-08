#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Outputs.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::blocktreeords
{
class Output;
}

namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::codecs::blocktreeords
{

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using Outputs = org::apache::lucene::util::fst::Outputs;

/** A custom FST outputs implementation that stores block data
 *  (BytesRef), long ordStart, long numTerms. */

class FSTOrdsOutputs final
    : public Outputs<std::shared_ptr<FSTOrdsOutputs::Output>>
{
  GET_CLASS_NAME(FSTOrdsOutputs)

public:
  static const std::shared_ptr<Output> NO_OUTPUT;

private:
  static const std::shared_ptr<BytesRef> NO_BYTES;

public:
  class Output final : public std::enable_shared_from_this<Output>
  {
    GET_CLASS_NAME(Output)
  public:
    const std::shared_ptr<BytesRef> bytes;
    // Inclusive:
    const int64_t startOrd;
    // Inclusive:
    const int64_t endOrd;

    Output(std::shared_ptr<BytesRef> bytes, int64_t startOrd,
           int64_t endOrd);

    virtual std::wstring toString();

    virtual int hashCode();

    bool equals(std::any _other) override;
  };

public:
  std::shared_ptr<Output> common(std::shared_ptr<Output> output1,
                                 std::shared_ptr<Output> output2) override;

  std::shared_ptr<Output> subtract(std::shared_ptr<Output> output,
                                   std::shared_ptr<Output> inc) override;

  std::shared_ptr<Output> add(std::shared_ptr<Output> prefix,
                              std::shared_ptr<Output> output) override;

  void write(std::shared_ptr<Output> prefix,
             std::shared_ptr<DataOutput> out)  override;

  std::shared_ptr<Output>
  read(std::shared_ptr<DataInput> in_)  override;

  void skipOutput(std::shared_ptr<DataInput> in_)  override;

  void
  skipFinalOutput(std::shared_ptr<DataInput> in_)  override;

  std::shared_ptr<Output> getNoOutput() override;

  std::wstring outputToString(std::shared_ptr<Output> output) override;

  std::shared_ptr<Output> newOutput(std::shared_ptr<BytesRef> bytes,
                                    int64_t startOrd, int64_t endOrd);

  int64_t ramBytesUsed(std::shared_ptr<Output> output) override;

protected:
  std::shared_ptr<FSTOrdsOutputs> shared_from_this()
  {
    return std::static_pointer_cast<FSTOrdsOutputs>(
        org.apache.lucene.util.fst
            .Outputs<FSTOrdsOutputs.Output>::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blocktreeords
