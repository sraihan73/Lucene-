#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

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
 * Wraps another Outputs implementation and encodes one or
 * more of its output values.  You can use this when a single
 * input may need to map_obj to more than one output,
 * maintaining order: pass the same input with a different
 * output by calling {@link Builder#add(IntsRef,Object)} multiple
 * times.  The builder will then combine the outputs using
 * the {@link Outputs#merge(Object,Object)} method.
 *
 * <p>The resulting FST may not be minimal when an input has
 * more than one output, as this requires pushing all
 * multi-output values to a final state.
 *
 * <p>NOTE: the only way to create multiple outputs is to
 * add the same input to the FST multiple times in a row.  This is
 * how the FST maps a single input to multiple outputs (e.g. you
 * cannot pass a List&lt;Object&gt; to {@link Builder#add}).  If
 * your outputs are longs, and you need at most 2, then use
 * {@link UpToTwoPositiveIntOutputs} instead since it stores
 * the outputs more compactly (by stealing a bit from each
 * long value).
 *
 * <p>NOTE: this cannot wrap itself (ie you cannot make an
 * FST with List&lt;List&lt;Object&gt;&gt; outputs using this).
 *
 * @lucene.experimental
 */

// NOTE: i think we could get a more compact FST if, instead
// of adding the same input multiple times with a different
// output each time, we added it only once with a
// pre-constructed List<T> output.  This way the "multiple
// values" is fully opaque to the Builder/FST.  It would
// require implementing the full algebra using set
// arithmetic (I think?); maybe SetOfOutputs is a good name.

template <typename T>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") public final class
// ListOfOutputs<T> extends Outputs<Object>
class ListOfOutputs final : public Outputs<std::any>
{

private:
  const std::shared_ptr<Outputs<T>> outputs;

public:
  ListOfOutputs(std::shared_ptr<Outputs<T>> outputs) : outputs(outputs) {}

  std::any common(std::any output1, std::any output2) override
  {
    // These will never be a deque:
    return outputs->common(std::any_cast<T>(output1),
                           std::any_cast<T>(output2));
  }

  std::any subtract(std::any object, std::any inc) override
  {
    // These will never be a deque:
    return outputs->subtract(std::any_cast<T>(object), std::any_cast<T>(inc));
  }

  std::any add(std::any prefix, std::any output) override
  {
    assert(!(dynamic_cast<std::deque>(prefix) != nullptr));
    if (!(dynamic_cast<std::deque>(output) != nullptr)) {
      return outputs->add(std::any_cast<T>(prefix), std::any_cast<T>(output));
    } else {
      std::deque<T> outputList = std::any_cast<std::deque<T>>(output);
      std::deque<T> addedList(outputList.size());
      for (auto _output : outputList) {
        addedList.push_back(outputs->add(std::any_cast<T>(prefix), _output));
      }
      return addedList;
    }
  }

  void write(std::any output,
             std::shared_ptr<DataOutput> out)  override
  {
    assert(!(dynamic_cast<std::deque>(output) != nullptr));
    outputs->write(std::any_cast<T>(output), out);
  }

  void
  writeFinalOutput(std::any output,
                   std::shared_ptr<DataOutput> out)  override
  {
    if (!(dynamic_cast<std::deque>(output) != nullptr)) {
      out->writeVInt(1);
      outputs->write(std::any_cast<T>(output), out);
    } else {
      std::deque<T> outputList = std::any_cast<std::deque<T>>(output);
      out->writeVInt(outputList.size());
      for (auto eachOutput : outputList) {
        outputs->write(eachOutput, out);
      }
    }
  }

  std::any read(std::shared_ptr<DataInput> in_)  override
  {
    return outputs->read(in_);
  }

  void skipOutput(std::shared_ptr<DataInput> in_)  override
  {
    outputs->skipOutput(in_);
  }

  std::any
  readFinalOutput(std::shared_ptr<DataInput> in_)  override
  {
    int count = in_->readVInt();
    if (count == 1) {
      return outputs->read(in_);
    } else {
      std::deque<T> outputList(count);
      for (int i = 0; i < count; i++) {
        outputList.push_back(outputs->read(in_));
      }
      return outputList;
    }
  }

  void
  skipFinalOutput(std::shared_ptr<DataInput> in_)  override
  {
    int count = in_->readVInt();
    for (int i = 0; i < count; i++) {
      outputs->skipOutput(in_);
    }
  }

  std::any getNoOutput() override { return outputs->getNoOutput(); }

  std::wstring outputToString(std::any output) override
  {
    if (!(dynamic_cast<std::deque>(output) != nullptr)) {
      return outputs->outputToString(std::any_cast<T>(output));
    } else {
      std::deque<T> outputList = std::any_cast<std::deque<T>>(output);

      std::shared_ptr<StringBuilder> b = std::make_shared<StringBuilder>();
      b->append(L'[');

      for (int i = 0; i < outputList.size(); i++) {
        if (i > 0) {
          b->append(L", ");
        }
        b->append(outputs->outputToString(outputList[i]));
      }
      b->append(L']');
      return b->toString();
    }
  }

  std::any merge(std::any first, std::any second) override
  {
    std::deque<T> outputList;
    if (!(dynamic_cast<std::deque>(first) != nullptr)) {
      outputList.push_back(std::any_cast<T>(first));
    } else {
      outputList.addAll(std::any_cast<std::deque<T>>(first));
    }
    if (!(dynamic_cast<std::deque>(second) != nullptr)) {
      outputList.push_back(std::any_cast<T>(second));
    } else {
      outputList.addAll(std::any_cast<std::deque<T>>(second));
    }
    // System.out.println("MERGE: now " + outputList.size() + " first=" +
    // outputToString(first) + " second=" + outputToString(second));
    // System.out.println("  return " + outputToString(outputList));
    return outputList;
  }

  std::wstring toString() override
  {
    return L"OneOrMoreOutputs(" + outputs + L")";
  }

  std::deque<T> asList(std::any output)
  {
    if (!(dynamic_cast<std::deque>(output) != nullptr)) {
      std::deque<T> result(1);
      result.push_back(std::any_cast<T>(output));
      return result;
    } else {
      return std::any_cast<std::deque<T>>(output);
    }
  }

private:
  static const int64_t BASE_LIST_NUM_BYTES =
      RamUsageEstimator::shallowSizeOf(std::deque<std::any>());

public:
  int64_t ramBytesUsed(std::any output) override
  {
    int64_t bytes = 0;
    if (dynamic_cast<std::deque>(output) != nullptr) {
      bytes += BASE_LIST_NUM_BYTES;
      std::deque<T> outputList = std::any_cast<std::deque<T>>(output);
      for (auto _output : outputList) {
        bytes += outputs->ramBytesUsed(_output);
      }
      // 2 * to allow for ArrayList's oversizing:
      bytes += 2 * outputList.size() * RamUsageEstimator::NUM_BYTES_OBJECT_REF;
    } else {
      bytes += outputs->ramBytesUsed(std::any_cast<T>(output));
    }

    return bytes;
  }

protected:
  std::shared_ptr<ListOfOutputs> shared_from_this()
  {
    return std::static_pointer_cast<ListOfOutputs>(
        Outputs<Object>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/fst/
