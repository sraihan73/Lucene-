#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
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

/**
 * Function that returns the decoded norm for every document.
 * <p>
 * Note that the configured Similarity for the field must be
 * a subclass of {@link TFIDFSimilarity} and the contribution of
 * the TF needs to be 1 when the freq is 1 and the contribution
 * of the IDF needs to be 1 when docFreq == docCount == 1.
 * @lucene.internal */
class NormValueSource : public ValueSource
{
  GET_CLASS_NAME(NormValueSource)
protected:
  const std::wstring field;

public:
  NormValueSource(const std::wstring &field);

  virtual std::wstring name();

  std::wstring description() override;

  void createWeight(
      std::unordered_map context,
      std::shared_ptr<IndexSearcher> searcher)  override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class FloatDocValuesAnonymousInnerClass : public FloatDocValues
  {
    GET_CLASS_NAME(FloatDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<NormValueSource> outerInstance;

    std::shared_ptr<SimScorer> simScorer;

  public:
    FloatDocValuesAnonymousInnerClass(
        std::shared_ptr<NormValueSource> outerInstance,
        std::shared_ptr<SimScorer> simScorer);

    int lastDocID = 0;
    float floatVal(int docID)  override;

  protected:
    std::shared_ptr<FloatDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FloatDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .FloatDocValues::shared_from_this());
    }
  };

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<NormValueSource> shared_from_this()
  {
    return std::static_pointer_cast<NormValueSource>(
        org.apache.lucene.queries.function.ValueSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
