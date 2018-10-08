#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <typeinfo>

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

/**
 * Base class for all mutable values.
 *
 * @lucene.internal
 */
class MutableValue : public std::enable_shared_from_this<MutableValue>,
                     public Comparable<std::shared_ptr<MutableValue>>
{
  GET_CLASS_NAME(MutableValue)
public:
  // C++ NOTE: Fields cannot have the same name as methods:
  bool exists_ = true;

  virtual void copy(std::shared_ptr<MutableValue> source) = 0;
  virtual std::shared_ptr<MutableValue> duplicate() = 0;
  virtual bool equalsSameType(std::any other) = 0;
  virtual int compareSameType(std::any other) = 0;
  virtual std::any toObject() = 0;

  virtual bool exists();

  int compareTo(std::shared_ptr<MutableValue> other) override;

  bool equals(std::any other) override;

  int hashCode() = 0;
  override

      std::wstring
      toString() override;
};

} // namespace org::apache::lucene::util::mutable_
