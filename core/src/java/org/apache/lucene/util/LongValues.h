#pragma once
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::util
{

/** Abstraction over an array of longs.
 *  @lucene.internal */
class LongValues : public std::enable_shared_from_this<LongValues>
{
  GET_CLASS_NAME(LongValues)

  /** An instance that returns the provided value. */
public:
  static const std::shared_ptr<LongValues> IDENTITY;

private:
  class LongValuesAnonymousInnerClass;

public:
  static const std::shared_ptr<LongValues> ZEROES;

private:
  class LongValuesAnonymousInnerClass2;

  /** Get value at <code>index</code>. */
public:
  virtual int64_t get(int64_t index) = 0;
};

} // namespace org::apache::lucene::util
class LongValues::LongValuesAnonymousInnerClass : public LongValues
{
  GET_CLASS_NAME(LongValues::LongValuesAnonymousInnerClass)
public:
  LongValuesAnonymousInnerClass();

  int64_t get(int64_t index) override;

protected:
  std::shared_ptr<LongValuesAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<LongValuesAnonymousInnerClass>(
        LongValues::shared_from_this());
  }
};
class LongValues::LongValuesAnonymousInnerClass2 : public LongValues
{
  GET_CLASS_NAME(LongValues::LongValuesAnonymousInnerClass2)
public:
  LongValuesAnonymousInnerClass2();

  int64_t get(int64_t index) override;

protected:
  std::shared_ptr<LongValuesAnonymousInnerClass2> shared_from_this()
  {
    return std::static_pointer_cast<LongValuesAnonymousInnerClass2>(
        LongValues::shared_from_this());
  }
};
