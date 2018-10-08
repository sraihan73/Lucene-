#pragma once
#include "stringhelper.h"
#include <any>
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
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/**
 * An FST {@link Outputs} implementation where each output
 * is one or two non-negative long values.  If it's a
 * single output, Long is returned; else, TwoLongs.  Order
 * is preserved in the TwoLongs case, ie .first is the first
 * input/output added to Builder, and .second is the
 * second.  You cannot store 0 output with this (that's
 * reserved to mean "no output")!
 *
 * <p>NOTE: the only way to create a TwoLongs output is to
 * add the same input to the FST twice in a row.  This is
 * how the FST maps a single input to two outputs (e.g. you
 * cannot pass a TwoLongs to {@link Builder#add}.  If you
 * need more than two then use {@link ListOfOutputs}, but if
 * you only have at most 2 then this implementation will
 * require fewer bytes as it steals one bit from each long
 * value.
 *
 * <p>NOTE: the resulting FST is not guaranteed to be minimal!
 * See {@link Builder}.
 *
 * @lucene.experimental
 */

class UpToTwoPositiveIntOutputs final : public Outputs<std::any>
{
  GET_CLASS_NAME(UpToTwoPositiveIntOutputs)

  /** Holds two long outputs. */
public:
  class TwoLongs final : public std::enable_shared_from_this<TwoLongs>
  {
    GET_CLASS_NAME(TwoLongs)
  public:
    const int64_t first;
    const int64_t second;

    TwoLongs(int64_t first, int64_t second);

    virtual std::wstring toString();

    bool equals(std::any _other) override;

    virtual int hashCode();
  };

private:
  static const std::optional<int64_t> NO_OUTPUT;

  const bool doShare;

  static const std::shared_ptr<UpToTwoPositiveIntOutputs> singletonShare;
  static const std::shared_ptr<UpToTwoPositiveIntOutputs> singletonNoShare;

  UpToTwoPositiveIntOutputs(bool doShare);

public:
  static std::shared_ptr<UpToTwoPositiveIntOutputs> getSingleton(bool doShare);

  std::optional<int64_t> get(int64_t v);

  std::shared_ptr<TwoLongs> get(int64_t first, int64_t second);

  std::optional<int64_t> common(std::any _output1,
                                  std::any _output2) override;

  std::optional<int64_t> subtract(std::any _output, std::any _inc) override;

  std::any add(std::any _prefix, std::any _output) override;

  void write(std::any _output,
             std::shared_ptr<DataOutput> out)  override;

  std::any read(std::shared_ptr<DataInput> in_)  override;

private:
  bool valid(std::optional<int64_t> &o);

  // Used only by assert
  bool valid(std::any _o, bool allowDouble);

public:
  std::any getNoOutput() override;

  std::wstring outputToString(std::any output) override;

  std::any merge(std::any first, std::any second) override;

private:
  static const int64_t TWO_LONGS_NUM_BYTES =
      RamUsageEstimator::shallowSizeOf(std::make_shared<TwoLongs>(0, 0));

public:
  int64_t ramBytesUsed(std::any o) override;

protected:
  std::shared_ptr<UpToTwoPositiveIntOutputs> shared_from_this()
  {
    return std::static_pointer_cast<UpToTwoPositiveIntOutputs>(
        Outputs<Object>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/fst/
