#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"

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
namespace org::apache::lucene::index
{

/**
 * Exposes multi-valued view over a single-valued instance.
 * <p>
 * This can be used if you want to have one multi-valued implementation
 * that works for single or multi-valued types.
 */
class SingletonSortedNumericDocValues final : public SortedNumericDocValues
{
  GET_CLASS_NAME(SingletonSortedNumericDocValues)
private:
  const std::shared_ptr<NumericDocValues> in_;

public:
  SingletonSortedNumericDocValues(std::shared_ptr<NumericDocValues> in_);

  /** Return the wrapped {@link NumericDocValues} */
  std::shared_ptr<NumericDocValues> getNumericDocValues();

  int docID() override;

  int nextDoc()  override;

  int advance(int target)  override;

  bool advanceExact(int target)  override;

  int64_t cost() override;

  int64_t nextValue()  override;

  int docValueCount() override;

protected:
  std::shared_ptr<SingletonSortedNumericDocValues> shared_from_this()
  {
    return std::static_pointer_cast<SingletonSortedNumericDocValues>(
        SortedNumericDocValues::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
