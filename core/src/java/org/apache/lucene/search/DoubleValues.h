#pragma once
#include "exceptionhelper.h"
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

namespace org::apache::lucene::search
{

/**
 * Per-segment, per-document double values, which can be calculated at
 * search-time
 */
class DoubleValues : public std::enable_shared_from_this<DoubleValues>
{
  GET_CLASS_NAME(DoubleValues)

  /**
   * Get the double value for the current document
   */
public:
  virtual double doubleValue() = 0;

  /**
   * Advance this instance to the given document id
   * @return true if there is a value for this document
   */
  virtual bool advanceExact(int doc) = 0;

  /**
   * Wrap a DoubleValues instance, returning a default if the wrapped instance
   * has no value
   */
  static std::shared_ptr<DoubleValues>
  withDefault(std::shared_ptr<DoubleValues> in_, double missingValue);

private:
  class DoubleValuesAnonymousInnerClass;

  /**
   * An empty DoubleValues instance that always returns {@code false} from
   * {@link #advanceExact(int)}
   */
public:
  static const std::shared_ptr<DoubleValues> EMPTY;

private:
  class DoubleValuesAnonymousInnerClass2;
};

} // namespace org::apache::lucene::search
class DoubleValues::DoubleValuesAnonymousInnerClass : public DoubleValues
{
  GET_CLASS_NAME(DoubleValues::DoubleValuesAnonymousInnerClass)
private:
  std::shared_ptr<org::apache::lucene::search::DoubleValues> in_;
  double missingValue = 0;

public:
  DoubleValuesAnonymousInnerClass(
      std::shared_ptr<org::apache::lucene::search::DoubleValues> in_,
      double missingValue);

  bool hasValue = false;

  double doubleValue()  override;

  bool advanceExact(int doc)  override;

protected:
  std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
        DoubleValues::shared_from_this());
  }
};
class DoubleValues::DoubleValuesAnonymousInnerClass2 : public DoubleValues
{
  GET_CLASS_NAME(DoubleValues::DoubleValuesAnonymousInnerClass2)
public:
  DoubleValuesAnonymousInnerClass2();

  double doubleValue()  override;

  bool advanceExact(int doc)  override;

protected:
  std::shared_ptr<DoubleValuesAnonymousInnerClass2> shared_from_this()
  {
    return std::static_pointer_cast<DoubleValuesAnonymousInnerClass2>(
        DoubleValues::shared_from_this());
  }
};
