#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::search::similarities
{
class TFIDFSimilarity;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
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
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Function that returns {@link TFIDFSimilarity#tf(float)}
 * for every document.
 * <p>
 * Note that the configured Similarity for the field must be
 * a subclass of {@link TFIDFSimilarity}
 * @lucene.internal */
class TFValueSource : public TermFreqValueSource
{
  GET_CLASS_NAME(TFValueSource)
public:
  TFValueSource(const std::wstring &field, const std::wstring &val,
                const std::wstring &indexedField,
                std::shared_ptr<BytesRef> indexedBytes);

  std::wstring name() override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class FloatDocValuesAnonymousInnerClass : public FloatDocValues
  {
    GET_CLASS_NAME(FloatDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<TFValueSource> outerInstance;

    std::shared_ptr<Terms> terms;
    std::shared_ptr<TFIDFSimilarity> similarity;

  public:
    FloatDocValuesAnonymousInnerClass(
        std::shared_ptr<TFValueSource> outerInstance,
        std::shared_ptr<Terms> terms,
        std::shared_ptr<TFIDFSimilarity> similarity);

    std::shared_ptr<PostingsEnum> docs;
    int atDoc = 0;
    int lastDocRequested = 0;

    void reset() ;

  private:
    class PostingsEnumAnonymousInnerClass : public PostingsEnum
    {
      GET_CLASS_NAME(PostingsEnumAnonymousInnerClass)
    private:
      std::shared_ptr<FloatDocValuesAnonymousInnerClass> outerInstance;

    public:
      PostingsEnumAnonymousInnerClass(
          std::shared_ptr<FloatDocValuesAnonymousInnerClass> outerInstance);

      int freq() override;

      int nextPosition()  override;

      int startOffset()  override;

      int endOffset()  override;

      std::shared_ptr<BytesRef> getPayload()  override;

      int docID() override;

      int nextDoc() override;

      int advance(int target) override;

      int64_t cost() override;

    protected:
      std::shared_ptr<PostingsEnumAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<PostingsEnumAnonymousInnerClass>(
            org.apache.lucene.index.PostingsEnum::shared_from_this());
      }
    };

  public:
    float floatVal(int doc) override;

  protected:
    std::shared_ptr<FloatDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FloatDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .FloatDocValues::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TFValueSource> shared_from_this()
  {
    return std::static_pointer_cast<TFValueSource>(
        TermFreqValueSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
