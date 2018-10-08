#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Outputs.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::memory
{
class TermData;
}

namespace org::apache::lucene::index
{
class FieldInfo;
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
namespace org::apache::lucene::codecs::memory
{

using FieldInfo = org::apache::lucene::index::FieldInfo;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Accountable = org::apache::lucene::util::Accountable;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using Outputs = org::apache::lucene::util::fst::Outputs;

/**
 * An FST {@link Outputs} implementation for
 * {@link FSTTermsWriter}.
 *
 * @lucene.experimental
 */

// NOTE: outputs should be per-field, since
// longsSize is fixed for each field
class FSTTermOutputs : public Outputs<std::shared_ptr<FSTTermOutputs::TermData>>
{
  GET_CLASS_NAME(FSTTermOutputs)
private:
  static const std::shared_ptr<TermData> NO_OUTPUT;
  // private static bool TEST = false;
  const bool hasPos;
  const int longsSize;

  /**
   * Represents the metadata for one term.
   * On an FST, only long[] part is 'shared' and pushed towards root.
   * byte[] and term stats will be kept on deeper arcs.
   */
public:
  class TermData : public std::enable_shared_from_this<TermData>,
                   public Accountable
  {
    GET_CLASS_NAME(TermData)
  private:
    static const int64_t BASE_RAM_BYTES_USED =
        RamUsageEstimator::shallowSizeOfInstance(TermData::typeid);

  public:
    std::deque<int64_t> longs;
    std::deque<char> bytes;
    int docFreq = 0;
    int64_t totalTermFreq = 0;
    TermData();
    TermData(std::deque<int64_t> &longs, std::deque<char> &bytes,
             int docFreq, int64_t totalTermFreq);

    int64_t ramBytesUsed() override;

    // NOTE: actually, FST nodes are seldom
    // identical when outputs on their arcs
    // aren't NO_OUTPUTs.
    virtual int hashCode();

    virtual std::wstring toString();

    bool equals(std::any other_) override;
  };

protected:
  FSTTermOutputs(std::shared_ptr<FieldInfo> fieldInfo, int longsSize);

public:
  int64_t ramBytesUsed(std::shared_ptr<TermData> output) override;

  std::shared_ptr<TermData> common(std::shared_ptr<TermData> t1,
                                   std::shared_ptr<TermData> t2) override;

  std::shared_ptr<TermData> subtract(std::shared_ptr<TermData> t1,
                                     std::shared_ptr<TermData> t2) override;

  // TODO: if we refactor a 'addSelf(TermData other)',
  // we can gain about 5~7% for fuzzy queries, however this also
  // means we are putting too much stress on FST Outputs decoding?
  std::shared_ptr<TermData> add(std::shared_ptr<TermData> t1,
                                std::shared_ptr<TermData> t2) override;

  void write(std::shared_ptr<TermData> data,
             std::shared_ptr<DataOutput> out)  override;

  std::shared_ptr<TermData>
  read(std::shared_ptr<DataInput> in_)  override;

  void skipOutput(std::shared_ptr<DataInput> in_)  override;

  std::shared_ptr<TermData> getNoOutput() override;

  std::wstring outputToString(std::shared_ptr<TermData> data) override;

  static bool statsEqual(std::shared_ptr<TermData> t1,
                         std::shared_ptr<TermData> t2);
  static bool bytesEqual(std::shared_ptr<TermData> t1,
                         std::shared_ptr<TermData> t2);
  static bool longsEqual(std::shared_ptr<TermData> t1,
                         std::shared_ptr<TermData> t2);
  static bool allZero(std::deque<int64_t> &l);

protected:
  std::shared_ptr<FSTTermOutputs> shared_from_this()
  {
    return std::static_pointer_cast<FSTTermOutputs>(
        org.apache.lucene.util.fst
            .Outputs<FSTTermOutputs.TermData>::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::memory
