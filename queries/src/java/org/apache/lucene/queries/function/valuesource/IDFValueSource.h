#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
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
class TFIDFSimilarity;
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

using namespace org::apache::lucene::index;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Function that returns {@link TFIDFSimilarity #idf(long, long)}
 * for every document.
 * <p>
 * Note that the configured Similarity for the field must be
 * a subclass of {@link TFIDFSimilarity}
 * @lucene.internal */
class IDFValueSource : public DocFreqValueSource
{
  GET_CLASS_NAME(IDFValueSource)
public:
  IDFValueSource(const std::wstring &field, const std::wstring &val,
                 const std::wstring &indexedField,
                 std::shared_ptr<BytesRef> indexedBytes);

  std::wstring name() override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

  // tries extra hard to cast the sim to TFIDFSimilarity
  static std::shared_ptr<TFIDFSimilarity>
  asTFIDF(std::shared_ptr<Similarity> sim, const std::wstring &field);

protected:
  std::shared_ptr<IDFValueSource> shared_from_this()
  {
    return std::static_pointer_cast<IDFValueSource>(
        DocFreqValueSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
