#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/search/Explanation.h"

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
namespace org::apache::lucene::queries
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Explanation = org::apache::lucene::search::Explanation;

/**
 * An instance of this subclass should be returned by
 * {@link CustomScoreQuery#getCustomScoreProvider}, if you want
GET_CLASS_NAME(should)
 * to modify the custom score calculation of a {@link CustomScoreQuery}.
 * <p>Since Lucene 2.9, queries operate on each segment of an index separately,
 * so the protected {@link #context} field can be used to resolve doc IDs,
 * as the supplied <code>doc</code> ID is per-segment and without knowledge
 * of the IndexReader you cannot access the document or DocValues.
 *
 * @lucene.experimental
 * @since 2.9.2
 */
class CustomScoreProvider
    : public std::enable_shared_from_this<CustomScoreProvider>
{
  GET_CLASS_NAME(CustomScoreProvider)

protected:
  const std::shared_ptr<LeafReaderContext> context;

  /**
   * Creates a new instance of the provider class for the given {@link
   * IndexReader}.
   */
public:
  CustomScoreProvider(std::shared_ptr<LeafReaderContext> context);

  /**
   * Compute a custom score by the subQuery score and a number of
   * {@link org.apache.lucene.queries.function.FunctionQuery} scores.
   * <p>
   * Subclasses can override this method to modify the custom score.
   * <p>
   * If your custom scoring is different than the default herein you
   * should override at least one of the two customScore() methods.
   * If the number of {@link FunctionQuery function queries} is always &lt; 2 it
   * is sufficient to override the other
   * {@link #customScore(int, float, float) customScore()}
   * method, which is simpler.
   * <p>
   * The default computation herein is a multiplication of given scores:
   * <pre>
   *     ModifiedScore = valSrcScore * valSrcScores[0] * valSrcScores[1] * ...
   * </pre>
   *
   * @param doc id of scored doc.
   * @param subQueryScore score of that doc by the subQuery.
   * @param valSrcScores scores of that doc by the {@link FunctionQuery}.
   * @return custom score.
   */
  virtual float
  customScore(int doc, float subQueryScore,
              std::deque<float> &valSrcScores) ;

  /**
   * Compute a custom score by the subQuery score and the {@link FunctionQuery}
   * score. <p> Subclasses can override this method to modify the custom score.
   * <p>
   * If your custom scoring is different than the default herein you
   * should override at least one of the two customScore() methods.
   * If the number of {@link FunctionQuery function queries} is always &lt; 2 it
   * is sufficient to override this customScore() method, which is simpler. <p>
   * The default computation herein is a multiplication of the two scores:
   * <pre>
   *     ModifiedScore = subQueryScore * valSrcScore
   * </pre>
   *
   * @param doc id of scored doc.
   * @param subQueryScore score of that doc by the subQuery.
   * @param valSrcScore score of that doc by the {@link FunctionQuery}.
   * @return custom score.
   */
  virtual float customScore(int doc, float subQueryScore,
                            float valSrcScore) ;

  /**
   * Explain the custom score.
   * Whenever overriding {@link #customScore(int, float, float[])},
   * this method should also be overridden to provide the correct explanation
   * for the part of the custom scoring.
   *
   * @param doc doc being explained.
   * @param subQueryExpl explanation for the sub-query part.
   * @param valSrcExpls explanation for the value source part.
   * @return an explanation for the custom score
   */
  virtual std::shared_ptr<Explanation>
  customExplain(int doc, std::shared_ptr<Explanation> subQueryExpl,
                std::deque<std::shared_ptr<Explanation>>
                    &valSrcExpls) ;

  /**
   * Explain the custom score.
   * Whenever overriding {@link #customScore(int, float, float)},
   * this method should also be overridden to provide the correct explanation
   * for the part of the custom scoring.
   *
   * @param doc doc being explained.
   * @param subQueryExpl explanation for the sub-query part.
   * @param valSrcExpl explanation for the value source part.
   * @return an explanation for the custom score
   */
  virtual std::shared_ptr<Explanation>
  customExplain(int doc, std::shared_ptr<Explanation> subQueryExpl,
                std::shared_ptr<Explanation> valSrcExpl) ;
};

} // #include  "core/src/java/org/apache/lucene/queries/
