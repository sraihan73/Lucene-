#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
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
class BinaryDocValues;
}
namespace org::apache::lucene::index
{
class TermsEnum;
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

/**
 * Use a field value and find the Document Frequency within another field.
 *
 * @since solr 4.0
 */
class JoinDocFreqValueSource : public FieldCacheSource
{
  GET_CLASS_NAME(JoinDocFreqValueSource)

public:
  static const std::wstring NAME;

protected:
  const std::wstring qfield;

public:
  JoinDocFreqValueSource(const std::wstring &field, const std::wstring &qfield);

  std::wstring description() override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class IntDocValuesAnonymousInnerClass : public IntDocValues
  {
    GET_CLASS_NAME(IntDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<JoinDocFreqValueSource> outerInstance;

    std::shared_ptr<BinaryDocValues> terms;
    std::shared_ptr<TermsEnum> termsEnum;

  public:
    IntDocValuesAnonymousInnerClass(
        std::shared_ptr<JoinDocFreqValueSource> outerInstance,
        std::shared_ptr<BinaryDocValues> terms,
        std::shared_ptr<TermsEnum> termsEnum);

    int lastDocID = 0;

    int intVal(int doc)  override;

  protected:
    std::shared_ptr<IntDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .IntDocValues::shared_from_this());
    }
  };

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<JoinDocFreqValueSource> shared_from_this()
  {
    return std::static_pointer_cast<JoinDocFreqValueSource>(
        FieldCacheSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
