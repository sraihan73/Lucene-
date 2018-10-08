#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
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

/**
 * A null FST {@link Outputs} implementation; use this if
 * you just want to build an FSA.
 *
 * @lucene.experimental
 */

class NoOutputs final : public Outputs<std::any>
{
  GET_CLASS_NAME(NoOutputs)

public:
  static const std::any NO_OUTPUT;

private:
  class ObjectAnonymousInnerClass : public std::any
  {
    GET_CLASS_NAME(ObjectAnonymousInnerClass)
  public:
    ObjectAnonymousInnerClass();

    // NodeHash calls hashCode for this output; we fix this
    // so we get deterministic hashing.
    virtual int hashCode();

    bool equals(std::any other) override;

  protected:
    std::shared_ptr<ObjectAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ObjectAnonymousInnerClass>(
          Object::shared_from_this());
    }
  };

private:
  static const std::shared_ptr<NoOutputs> singleton;

  NoOutputs();

public:
  static std::shared_ptr<NoOutputs> getSingleton();

  std::any common(std::any output1, std::any output2) override;

  std::any subtract(std::any output, std::any inc) override;

  std::any add(std::any prefix, std::any output) override;

  std::any merge(std::any first, std::any second) override;

  void write(std::any prefix, std::shared_ptr<DataOutput> out) override;

  std::any read(std::shared_ptr<DataInput> in_) override;

  std::any getNoOutput() override;

  std::wstring outputToString(std::any output) override;

  int64_t ramBytesUsed(std::any output) override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<NoOutputs> shared_from_this()
  {
    return std::static_pointer_cast<NoOutputs>(
        Outputs<Object>::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::fst
