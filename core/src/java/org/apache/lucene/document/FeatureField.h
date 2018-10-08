#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TermFrequencyAttribute;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::search
{
class Query;
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
namespace org::apache::lucene::document
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using Explanation = org::apache::lucene::search::Explanation;
using Query = org::apache::lucene::search::Query;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;

/**
 * {@link Field} that can be used to store static scoring factors into
 * documents. This is mostly inspired from the work from Nick Craswell,
 * Stephen Robertson, Hugo Zaragoza and Michael Taylor. Relevance weighting
 * for query independent evidence. Proceedings of the 28th annual international
 * ACM SIGIR conference on Research and development in information retrieval.
 * August 15-19, 2005, Salvador, Brazil.
 * <p>
 * Feature values are internally encoded as term frequencies. Putting
 * feature queries as
 * {@link org.apache.lucene.search.BooleanClause.Occur#SHOULD} clauses of a
 * {@link BooleanQuery} allows to combine query-dependent scores (eg. BM25)
 * with query-independent scores using a linear combination. The fact that
 * feature values are stored as frequencies will allow search logic to
 * efficiently skip documents that can't be competitive when total hit counts
 * are not requested in the future. This makes it a compelling option compared
 * to storing such factors eg. in a doc-value field.
 * <p>
 * This field may only store factors that are positively correlated with the
 * final score, like pagerank. In case of factors that are inversely correlated
 * with the score like url length, the inverse of the scoring factor should be
 * stored, ie. {@code 1/urlLength}.
 * <p>
 * This field only considers the top 9 significant bits for storage efficiency
 * which allows to store them on 16 bits internally. In practice this limitation
 * means that values are stored with a relative precision of
 * 2<sup>-8</sup> = 0.00390625.
 * <p>
 * Given a scoring factor {@code S > 0} and its weight {@code w > 0}, there
 * are three ways that S can be turned into a score:
 * <ul>
 *   <li>{@link #newLogQuery w * log(a + S)}, with a &ge; 1. This function
 *       usually makes sense because the distribution of scoring factors
 *       often follows a power law. This is typically the case for pagerank for
 *       instance. However the paper suggested that the {@code satu} and
 *       {@code sigm} functions give even better results.
 *   <li>{@link #newSaturationQuery satu(S) = w * S / (S + k)}, with k &gt; 0.
 * This function is similar to the one used by {@link BM25Similarity} in order
 *       to incorporate term frequency into the final score and produces values
 *       between 0 and 1. A value of 0.5 is obtained when S and k are equal.
 *   <li>{@link #newSigmoidQuery sigm(S) = w * S<sup>a</sup> / (S<sup>a</sup> +
 * k<sup>a</sup>)}, with k &gt; 0, a &gt; 0. This function provided even better
 * results than the two above but is also harder to tune due to the fact it has
 *       2 parameters. Like with {@code satu}, values are in the 0..1 range and
 *       0.5 is obtained when S and k are equal.
 * </ul>
 * <p>
 * The constants in the above formulas typically need training in order to
 * compute optimal values. If you don't know where to start, the
 * {@link #newSaturationQuery(std::wstring, std::wstring)} method uses
 * {@code 1f} as a weight and tries to guess a sensible value for the
 * {@code pivot} parameter of the saturation function based on index
 * statistics, which shouldn't perform too bad. Here is an example, assuming
 * that documents have a {@link FeatureField} called 'features' with values for
 * the 'pagerank' feature.
 * <pre class="prettyprint">
 * Query query = new BooleanQuery.Builder()
 *     .add(new TermQuery(new Term("body", "apache")), Occur.SHOULD)
 *     .add(new TermQuery(new Term("body", "lucene")), Occur.SHOULD)
 *     .build();
 * Query boost = FeatureField.newSaturationQuery("features", "pagerank");
 * Query boostedQuery = new BooleanQuery.Builder()
 *     .add(query, Occur.MUST)
 *     .add(boost, Occur.SHOULD)
 *     .build();
 * TopDocs topDocs = searcher.search(boostedQuery, 10);
 * </pre>
 * @lucene.experimental
 */
class FeatureField final : public Field
{
  GET_CLASS_NAME(FeatureField)

private:
  static const std::shared_ptr<FieldType> FIELD_TYPE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static FeatureField::StaticConstructor staticConstructor;

  float featureValue = 0;

  /**
   * Create a feature.
   * @param fieldName The name of the field to store the information into. All
   * features may be stored in the same field.
   * @param featureName The name of the feature, eg. 'pagerank`. It will be
   * indexed as a term.
   * @param featureValue The value of the feature, must be a positive, finite,
   * normal float.
   */
public:
  FeatureField(const std::wstring &fieldName, const std::wstring &featureName,
               float featureValue);

  /**
   * Update the feature value of this field.
   */
  void setFeatureValue(float featureValue);

  std::shared_ptr<TokenStream>
  tokenStream(std::shared_ptr<Analyzer> analyzer,
              std::shared_ptr<TokenStream> reuse) override;

private:
  class FeatureTokenStream final : public TokenStream
  {
    GET_CLASS_NAME(FeatureTokenStream)
  private:
    const std::shared_ptr<CharTermAttribute> termAttribute =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<TermFrequencyAttribute> freqAttribute =
        addAttribute(TermFrequencyAttribute::typeid);
    bool used = true;
    std::wstring value = L"";
    int freq = 0;

    FeatureTokenStream();

    /** Sets the values */
  public:
    void setValues(const std::wstring &value, int freq);

    bool incrementToken() override;

    void reset() override;

    virtual ~FeatureTokenStream();

  protected:
    std::shared_ptr<FeatureTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<FeatureTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

private:
  static const int MAX_FREQ =
      static_cast<int>(static_cast<unsigned int>(Float::floatToIntBits(
                           std::numeric_limits<float>::max())) >>
                       15);

  static float decodeFeatureValue(float freq);

public:
  class FeatureFunction : public std::enable_shared_from_this<FeatureFunction>
  {
    GET_CLASS_NAME(FeatureFunction)
  public:
    virtual std::shared_ptr<SimScorer> scorer(const std::wstring &field,
                                              float w) = 0;
    virtual std::shared_ptr<Explanation> explain(const std::wstring &field,
                                                 const std::wstring &feature,
                                                 float w, int doc,
                                                 int freq) = 0;
    virtual std::shared_ptr<FeatureFunction>
    rewrite(std::shared_ptr<IndexReader> reader) ;
  };

public:
  class LogFunction final : public FeatureFunction
  {
    GET_CLASS_NAME(LogFunction)

  private:
    const float scalingFactor;

  public:
    LogFunction(float a);

    bool equals(std::any obj) override;

    virtual int hashCode();

    virtual std::wstring toString();

    std::shared_ptr<SimScorer> scorer(const std::wstring &field,
                                      float weight) override;

  private:
    class SimScorerAnonymousInnerClass : public SimScorer
    {
      GET_CLASS_NAME(SimScorerAnonymousInnerClass)
    private:
      std::shared_ptr<LogFunction> outerInstance;

      float weight = 0;

    public:
      SimScorerAnonymousInnerClass(std::shared_ptr<LogFunction> outerInstance,
                                   float weight);

      float score(int doc, float freq) override;

      float computeSlopFactor(int distance) override;

      float computePayloadFactor(int doc, int start, int end,
                                 std::shared_ptr<BytesRef> payload) override;

    protected:
      std::shared_ptr<SimScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SimScorerAnonymousInnerClass>(
            org.apache.lucene.search.similarities.Similarity
                .SimScorer::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Explanation> explain(const std::wstring &field,
                                         const std::wstring &feature, float w,
                                         int doc,
                                         int freq)  override;

  protected:
    std::shared_ptr<LogFunction> shared_from_this()
    {
      return std::static_pointer_cast<LogFunction>(
          FeatureFunction::shared_from_this());
    }
  };

public:
  class SaturationFunction final : public FeatureFunction
  {
    GET_CLASS_NAME(SaturationFunction)

  private:
    const std::wstring field, feature;
    const std::optional<float> pivot;

  public:
    SaturationFunction(const std::wstring &field, const std::wstring &feature,
                       std::optional<float> &pivot);

    std::shared_ptr<FeatureFunction>
    rewrite(std::shared_ptr<IndexReader> reader)  override;

    bool equals(std::any obj) override;

    virtual int hashCode();

    virtual std::wstring toString();

    std::shared_ptr<SimScorer> scorer(const std::wstring &field,
                                      float weight) override;

  private:
    class SimScorerAnonymousInnerClass : public SimScorer
    {
      GET_CLASS_NAME(SimScorerAnonymousInnerClass)
    private:
      std::shared_ptr<SaturationFunction> outerInstance;

      float weight = 0;
      float pivot = 0;

    public:
      SimScorerAnonymousInnerClass(
          std::shared_ptr<SaturationFunction> outerInstance, float weight,
          float pivot);

      float score(int doc, float freq) override;

      float computeSlopFactor(int distance) override;

      float computePayloadFactor(int doc, int start, int end,
                                 std::shared_ptr<BytesRef> payload) override;

    protected:
      std::shared_ptr<SimScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SimScorerAnonymousInnerClass>(
            org.apache.lucene.search.similarities.Similarity
                .SimScorer::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Explanation> explain(const std::wstring &field,
                                         const std::wstring &feature,
                                         float weight, int doc,
                                         int freq)  override;

  protected:
    std::shared_ptr<SaturationFunction> shared_from_this()
    {
      return std::static_pointer_cast<SaturationFunction>(
          FeatureFunction::shared_from_this());
    }
  };

public:
  class SigmoidFunction final : public FeatureFunction
  {
    GET_CLASS_NAME(SigmoidFunction)

  private:
    const float pivot, a;
    const double pivotPa;

  public:
    SigmoidFunction(float pivot, float a);

    bool equals(std::any obj) override;

    virtual int hashCode();

    virtual std::wstring toString();

    std::shared_ptr<SimScorer> scorer(const std::wstring &field,
                                      float weight) override;

  private:
    class SimScorerAnonymousInnerClass : public SimScorer
    {
      GET_CLASS_NAME(SimScorerAnonymousInnerClass)
    private:
      std::shared_ptr<SigmoidFunction> outerInstance;

      float weight = 0;

    public:
      SimScorerAnonymousInnerClass(
          std::shared_ptr<SigmoidFunction> outerInstance, float weight);

      float score(int doc, float freq) override;

      float computeSlopFactor(int distance) override;

      float computePayloadFactor(int doc, int start, int end,
                                 std::shared_ptr<BytesRef> payload) override;

    protected:
      std::shared_ptr<SimScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SimScorerAnonymousInnerClass>(
            org.apache.lucene.search.similarities.Similarity
                .SimScorer::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Explanation> explain(const std::wstring &field,
                                         const std::wstring &feature,
                                         float weight, int doc,
                                         int freq)  override;

  protected:
    std::shared_ptr<SigmoidFunction> shared_from_this()
    {
      return std::static_pointer_cast<SigmoidFunction>(
          FeatureFunction::shared_from_this());
    }
  };

  /**
   * Given that IDFs are logs, similarities that incorporate term freq and
   * document length in sane (ie. saturated) ways should have their score
   * bounded by a log. So we reject weights that are too high as it would mean
   * that this clause would completely dominate ranking, removing the need for
   * query-dependent scores.
   */
private:
  static const float MAX_WEIGHT;

  /**
   * Return a new {@link Query} that will score documents as
   * {@code weight * Math.log(scalingFactor + S)} where S is the value of the
   * static feature.
   * @param fieldName     field that stores features
   * @param featureName   name of the feature
   * @param weight        weight to give to this feature, must be in (0,64]
   * @param scalingFactor scaling factor applied before taking the logarithm,
   * must be in [1, +Infinity)
   * @throws IllegalArgumentException if weight is not in (0,64] or
   * scalingFactor is not in [1, +Infinity)
   */
public:
  static std::shared_ptr<Query> newLogQuery(const std::wstring &fieldName,
                                            const std::wstring &featureName,
                                            float weight, float scalingFactor);

  /**
   * Return a new {@link Query} that will score documents as
   * {@code weight * S / (S + pivot)} where S is the value of the static
   * feature.
   * @param fieldName   field that stores features
   * @param featureName name of the feature
   * @param weight      weight to give to this feature, must be in (0,64]
   * @param pivot       feature value that would give a score contribution equal
   * to weight/2, must be in (0, +Infinity)
   * @throws IllegalArgumentException if weight is not in (0,64] or pivot is not
   * in (0, +Infinity)
   */
  static std::shared_ptr<Query>
  newSaturationQuery(const std::wstring &fieldName,
                     const std::wstring &featureName, float weight,
                     float pivot);

  /**
   * Same as {@link #newSaturationQuery(std::wstring, std::wstring, float, float)} but
   * {@code 1f} is used as a weight and a reasonably good default pivot value
   * is computed based on index statistics and is approximately equal to the
   * geometric mean of all values that exist in the index.
   * @param fieldName   field that stores features
   * @param featureName name of the feature
   * @throws IllegalArgumentException if weight is not in (0,64] or pivot is not
   * in (0, +Infinity)
   */
  static std::shared_ptr<Query>
  newSaturationQuery(const std::wstring &fieldName,
                     const std::wstring &featureName);

private:
  static std::shared_ptr<Query>
  newSaturationQuery(const std::wstring &fieldName,
                     const std::wstring &featureName, float weight,
                     std::optional<float> &pivot);

  /**
   * Return a new {@link Query} that will score documents as
   * {@code weight * S^a / (S^a + pivot^a)} where S is the value of the static
   * feature.
   * @param fieldName   field that stores features
   * @param featureName name of the feature
   * @param weight      weight to give to this feature, must be in (0,64]
   * @param pivot       feature value that would give a score contribution equal
   * to weight/2, must be in (0, +Infinity)
   * @param exp         exponent, higher values make the function grow slower
   * before 'pivot' and faster after 'pivot', must be in (0, +Infinity)
   * @throws IllegalArgumentException if w is not in (0,64] or either k or a are
   * not in (0, +Infinity)
   */
public:
  static std::shared_ptr<Query> newSigmoidQuery(const std::wstring &fieldName,
                                                const std::wstring &featureName,
                                                float weight, float pivot,
                                                float exp);

  /**
   * Compute a feature value that may be used as the {@code pivot} parameter of
   * the {@link #newSaturationQuery(std::wstring, std::wstring, float, float)} and
   * {@link #newSigmoidQuery(std::wstring, std::wstring, float, float, float)} factory
   * methods. The implementation takes the average of the int bits of the float
   * representation in practice before converting it back to a float. Given that
   * floats store the exponent in the higher bits, it means that the result will
   * be an approximation of the geometric mean of all feature values.
   * @param reader       the {@link IndexReader} to search against
   * @param featureField the field that stores features
   * @param featureName  the name of the feature
   */
  static float
  computePivotFeatureValue(std::shared_ptr<IndexReader> reader,
                           const std::wstring &featureField,
                           const std::wstring &featureName) ;

protected:
  std::shared_ptr<FeatureField> shared_from_this()
  {
    return std::static_pointer_cast<FeatureField>(Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
