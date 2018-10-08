#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

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

/**
 * <code>SumTotalTermFreqValueSource</code> returns the number of tokens.
 * (sum of term freqs across all documents, across all terms).
 * Returns -1 if frequencies were omitted for the field, or if
 * the codec doesn't support this statistic.
 * @lucene.internal
 */
class SumTotalTermFreqValueSource : public ValueSource
{
  GET_CLASS_NAME(SumTotalTermFreqValueSource)
protected:
  const std::wstring indexedField;

public:
  SumTotalTermFreqValueSource(const std::wstring &indexedField);

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
    std::shared_ptr<SumTotalTermFreqValueSource> outerInstance;

    int64_t ttf = 0;

  public:
    LongDocValuesAnonymousInnerClass(
        std::shared_ptr<SumTotalTermFreqValueSource> outerInstance,
        int64_t ttf);

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
  std::shared_ptr<SumTotalTermFreqValueSource> shared_from_this()
  {
    return std::static_pointer_cast<SumTotalTermFreqValueSource>(
        org.apache.lucene.queries.function.ValueSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
