#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class SortedSetSelector;
}

namespace org::apache::lucene::search
{
class SortField;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
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
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using SortField = org::apache::lucene::search::SortField;
using SortedSetSelector = org::apache::lucene::search::SortedSetSelector;

/**
 * Retrieves {@link FunctionValues} instances for multi-valued string based
 * fields. <p> A SortedSetDocValues contains multiple values for a field, so
 * this technique "selects" a value as the representative value for the
 * document.
 *
 * @see SortedSetSelector
 */
class SortedSetFieldSource : public FieldCacheSource
{
  GET_CLASS_NAME(SortedSetFieldSource)
protected:
  const SortedSetSelector::Type selector;

public:
  SortedSetFieldSource(const std::wstring &field);

  SortedSetFieldSource(const std::wstring &field,
                       SortedSetSelector::Type selector);

  std::shared_ptr<SortField> getSortField(bool reverse) override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class DocTermsIndexDocValuesAnonymousInnerClass
      : public DocTermsIndexDocValues
  {
    GET_CLASS_NAME(DocTermsIndexDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<SortedSetFieldSource> outerInstance;

  public:
    DocTermsIndexDocValuesAnonymousInnerClass(
        std::shared_ptr<SortedSetFieldSource> outerInstance,
        const std::wstring &field, std::shared_ptr<SortedDocValues> view);

  protected:
    std::wstring toTerm(const std::wstring &readableValue) override;

  public:
    std::any objectVal(int doc)  override;

  protected:
    std::shared_ptr<DocTermsIndexDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DocTermsIndexDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .DocTermsIndexDocValues::shared_from_this());
    }
  };

public:
  std::wstring description() override;

  virtual int hashCode();

  bool equals(std::any obj) override;

protected:
  std::shared_ptr<SortedSetFieldSource> shared_from_this()
  {
    return std::static_pointer_cast<SortedSetFieldSource>(
        FieldCacheSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
