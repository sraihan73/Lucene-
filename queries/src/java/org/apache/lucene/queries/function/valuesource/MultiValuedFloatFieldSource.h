#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class SortedNumericSelector;
}

namespace org::apache::lucene::search
{
class SortField;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
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
namespace org::apache::lucene::queries::function::valuesource
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortField = org::apache::lucene::search::SortField;
using SortedNumericSelector =
    org::apache::lucene::search::SortedNumericSelector;
using Type = org::apache::lucene::search::SortedNumericSelector::Type;

/**
 * Obtains float field values from {@link
 * org.apache.lucene.index.LeafReader#getSortedNumericDocValues} and using a
 * {@link org.apache.lucene.search.SortedNumericSelector} it gives a
 * single-valued ValueSource view of a field.
 */
class MultiValuedFloatFieldSource : public FloatFieldSource
{
  GET_CLASS_NAME(MultiValuedFloatFieldSource)

protected:
  const SortedNumericSelector::Type selector;

public:
  MultiValuedFloatFieldSource(const std::wstring &field,
                              SortedNumericSelector::Type selector);

  std::shared_ptr<SortField> getSortField(bool reverse) override;

  std::wstring description() override;

protected:
  std::shared_ptr<NumericDocValues>
  getNumericDocValues(std::unordered_map context,
                      std::shared_ptr<LeafReaderContext>
                          readerContext)  override;

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<MultiValuedFloatFieldSource> shared_from_this()
  {
    return std::static_pointer_cast<MultiValuedFloatFieldSource>(
        FloatFieldSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
