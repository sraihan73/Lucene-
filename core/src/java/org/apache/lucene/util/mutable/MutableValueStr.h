#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}

namespace org::apache::lucene::util::@ mutable { class MutableValue; }

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
namespace org::apache::lucene::util::mutable_
{

using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * {@link MutableValue} implementation of type {@link std::wstring}.
 * When mutating instances of this object, the caller is responsible for
 * ensuring that any instance where <code>exists</code> is set to
 * <code>false</code> must also have a <code>value</code> with a length set to
 * 0.
 */
class MutableValueStr : public MutableValue
{
  GET_CLASS_NAME(MutableValueStr)
public:
  std::shared_ptr<BytesRefBuilder> value = std::make_shared<BytesRefBuilder>();

  std::any toObject() override;

  void copy(std::shared_ptr<MutableValue> source) override;

  std::shared_ptr<MutableValue> duplicate() override;

  bool equalsSameType(std::any other) override;

  int compareSameType(std::any other) override;

  virtual int hashCode();

protected:
  std::shared_ptr<MutableValueStr> shared_from_this()
  {
    return std::static_pointer_cast<MutableValueStr>(
        MutableValue::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::mutable_
