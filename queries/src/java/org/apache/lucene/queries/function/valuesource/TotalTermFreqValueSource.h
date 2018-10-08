#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
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
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * <code>TotalTermFreqValueSource</code> returns the total term freq
 * (sum of term freqs across all documents).
 * Returns -1 if frequencies were omitted for the field, or if
 * the codec doesn't support this statistic.
 * @lucene.internal
 */
class TotalTermFreqValueSource : public ValueSource
{
  GET_CLASS_NAME(TotalTermFreqValueSource)
protected:
  const std::wstring field;
  const std::wstring indexedField;
  const std::wstring val;
  const std::shared_ptr<BytesRef> indexedBytes;

public:
  TotalTermFreqValueSource(const std::wstring &field, const std::wstring &val,
                           const std::wstring &indexedField,
                           std::shared_ptr<BytesRef> indexedBytes);

  virtual std::wstring name();

  std::wstring description() override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

  void createWeight(
      std::unordered_map context,
      std::shared_ptr<IndexSearcher> searcher)  override;

private:
  class LongDocValuesAnonymousInnerClass : public LongDocValues
  {
    GET_CLASS_NAME(LongDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<TotalTermFreqValueSource> outerInstance;

    int64_t ttf = 0;

  public:
    LongDocValuesAnonymousInnerClass(
        std::shared_ptr<TotalTermFreqValueSource> outerInstance, int64_t ttf);

    int64_t longVal(int doc) override;

  protected:
    std::shared_ptr<LongDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LongDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .LongDocValues::shared_from_this());
    }
  };

public:
  virtual int hashCode();

  virtual bool equals(std::any o);

protected:
  std::shared_ptr<TotalTermFreqValueSource> shared_from_this()
  {
    return std::static_pointer_cast<TotalTermFreqValueSource>(
        org.apache.lucene.queries.function.ValueSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
