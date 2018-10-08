#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class Pair;
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
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/**
 * An FST {@link Outputs} implementation, holding two other outputs.
 *
 * @lucene.experimental
 */

template <typename A, typename B>
class PairOutputs : public Outputs<PairOutputs::Pair<A, B>>
{
  GET_CLASS_NAME(PairOutputs)

private:
  const std::shared_ptr<Pair<A, B>> NO_OUTPUT;
  const std::shared_ptr<Outputs<A>> outputs1;
  const std::shared_ptr<Outputs<B>> outputs2;

  /** Holds a single pair of two outputs. */
public:
  template <typename A, typename B>
  class Pair : public std::enable_shared_from_this<Pair>
  {
    GET_CLASS_NAME(Pair)
  public:
    const A output1;
    const B output2;

    // use newPair
  private:
    Pair(A output1, B output2) : output1(output1), output2(output2) {}

  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Override @SuppressWarnings("rawtypes") public bool
    // equals(Object other)
    bool equals(std::any other) override
    {
      if (other == shared_from_this()) {
        return true;
      } else if (std::dynamic_pointer_cast<Pair>(other) != nullptr) {
        std::shared_ptr<Pair> pair =
            std::any_cast<std::shared_ptr<Pair>>(other);
        return output1.equals(pair->output1) && output2.equals(pair->output2);
      } else {
        return false;
      }
    }

    int hashCode() override { return output1.hashCode() + output2.hashCode(); }

    std::wstring toString() override
    {
      return L"Pair(" + output1 + L"," + output2 + L")";
    }
  };

public:
  PairOutputs(std::shared_ptr<Outputs<A>> outputs1,
              std::shared_ptr<Outputs<B>> outputs2)
      : NO_OUTPUT(std::make_shared<Pair<A, B>>(outputs1->getNoOutput(),
                                               outputs2->getNoOutput())),
        outputs1(outputs1), outputs2(outputs2)
  {
  }

  /** Create a new Pair */
  virtual std::shared_ptr<Pair<A, B>> newPair(A a, B b)
  {
    if (a.equals(outputs1->getNoOutput())) {
      a = outputs1->getNoOutput();
    }
    if (b.equals(outputs2->getNoOutput())) {
      b = outputs2->getNoOutput();
    }

    if (a == outputs1->getNoOutput() && b == outputs2->getNoOutput()) {
      return NO_OUTPUT;
    } else {
      std::shared_ptr<Pair<A, B>> *const p = std::make_shared<Pair<A, B>>(a, b);
      assert(valid(p));
      return p;
    }
  }

  // for assert
private:
  bool valid(std::shared_ptr<Pair<A, B>> pair)
  {
    constexpr bool noOutput1 = pair->output1.equals(outputs1->getNoOutput());
    constexpr bool noOutput2 = pair->output2.equals(outputs2->getNoOutput());

    if (noOutput1 && pair->output1 != outputs1->getNoOutput()) {
      return false;
    }

    if (noOutput2 && pair->output2 != outputs2->getNoOutput()) {
      return false;
    }

    if (noOutput1 && noOutput2) {
      if (pair != NO_OUTPUT) {
        return false;
      } else {
        return true;
      }
    } else {
      return true;
    }
  }

public:
  std::shared_ptr<Pair<A, B>> common(std::shared_ptr<Pair<A, B>> pair1,
                                     std::shared_ptr<Pair<A, B>> pair2) override
  {
    assert(valid(pair1));
    assert(valid(pair2));
    return newPair(outputs1->common(pair1->output1, pair2->output1),
                   outputs2->common(pair1->output2, pair2->output2));
  }

  std::shared_ptr<Pair<A, B>> subtract(std::shared_ptr<Pair<A, B>> output,
                                       std::shared_ptr<Pair<A, B>> inc) override
  {
    assert(valid(output));
    assert(valid(inc));
    return newPair(outputs1->subtract(output->output1, inc->output1),
                   outputs2->subtract(output->output2, inc->output2));
  }

  std::shared_ptr<Pair<A, B>> add(std::shared_ptr<Pair<A, B>> prefix,
                                  std::shared_ptr<Pair<A, B>> output) override
  {
    assert(valid(prefix));
    assert(valid(output));
    return newPair(outputs1->add(prefix->output1, output->output1),
                   outputs2->add(prefix->output2, output->output2));
  }

  void write(std::shared_ptr<Pair<A, B>> output,
             std::shared_ptr<DataOutput> writer)  override
  {
    assert(valid(output));
    outputs1->write(output->output1, writer);
    outputs2->write(output->output2, writer);
  }

  std::shared_ptr<Pair<A, B>>
  read(std::shared_ptr<DataInput> in_)  override
  {
    A output1 = outputs1->read(in_);
    B output2 = outputs2->read(in_);
    return newPair(output1, output2);
  }

  void skipOutput(std::shared_ptr<DataInput> in_)  override
  {
    outputs1->skipOutput(in_);
    outputs2->skipOutput(in_);
  }

  std::shared_ptr<Pair<A, B>> getNoOutput() override { return NO_OUTPUT; }

  std::wstring outputToString(std::shared_ptr<Pair<A, B>> output) override
  {
    assert(valid(output));
    return L"<pair:" + outputs1->outputToString(output->output1) + L"," +
           outputs2->outputToString(output->output2) + L">";
  }

  std::wstring toString() override
  {
    return L"PairOutputs<" + outputs1 + L"," + outputs2 + L">";
  }

private:
  static const int64_t BASE_NUM_BYTES = RamUsageEstimator::shallowSizeOf(
      std::make_shared<Pair<std::any, std::any>>(nullptr, nullptr));

public:
  int64_t ramBytesUsed(std::shared_ptr<Pair<A, B>> output) override
  {
    int64_t ramBytesUsed = BASE_NUM_BYTES;
    if (output->output1 != nullptr) {
      ramBytesUsed += outputs1->ramBytesUsed(output->output1);
    }
    if (output->output2 != nullptr) {
      ramBytesUsed += outputs2->ramBytesUsed(output->output2);
    }
    return ramBytesUsed;
  }

protected:
  std::shared_ptr<PairOutputs> shared_from_this()
  {
    return std::static_pointer_cast<PairOutputs>(
        Outputs<PairOutputs.Pair<A, B>>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/fst/
