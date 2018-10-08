#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::search
{
class CollectionStatistics;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class TermStatistics;
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
namespace org::apache::lucene::search::similarities
{

using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Expert: Historical scoring implementation. You might want to consider using
 * {@link BM25Similarity} instead, which is generally considered superior to
 * TF-IDF.
 */
class ClassicSimilarity : public TFIDFSimilarity
{
  GET_CLASS_NAME(ClassicSimilarity)

  /** Sole constructor: parameter-free */
public:
  ClassicSimilarity();

  /** Implemented as
   *  <code>1/sqrt(length)</code>.
   *
   *  @lucene.experimental */
  float lengthNorm(int numTerms) override;

  /** Implemented as <code>sqrt(freq)</code>. */
  float tf(float freq) override;

  /** Implemented as <code>1 / (distance + 1)</code>. */
  float sloppyFreq(int distance) override;

  /** The default implementation returns <code>1</code> */
  float scorePayload(int doc, int start, int end,
                     std::shared_ptr<BytesRef> payload) override;

  std::shared_ptr<Explanation>
  idfExplain(std::shared_ptr<CollectionStatistics> collectionStats,
             std::shared_ptr<TermStatistics> termStats) override;

  /** Implemented as <code>log((docCount+1)/(docFreq+1)) + 1</code>. */
  float idf(int64_t docFreq, int64_t docCount) override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<ClassicSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<ClassicSimilarity>(
        TFIDFSimilarity::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
