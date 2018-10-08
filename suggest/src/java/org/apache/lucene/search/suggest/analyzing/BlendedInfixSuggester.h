#pragma once
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::search::suggest
{
class Lookup;
}
namespace org::apache::lucene::search::suggest
{
class LookupResult;
}
namespace org::apache::lucene::search
{
class BooleanClause;
}
namespace org::apache::lucene::search
{
class BooleanQuery;
}
namespace org::apache::lucene::document
{
class FieldType;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class TopFieldDocs;
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
namespace org::apache::lucene::search::suggest::analyzing
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using FieldType = org::apache::lucene::document::FieldType;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;

// TODO:
// - allow to use the search score

/**
 * Extension of the AnalyzingInfixSuggester which transforms the weight
 * after search to take into account the position of the searched term into
 * the indexed text.
 * Please note that it increases the number of elements searched and applies the
 * ponderation after. It might be costly for long suggestions.
 *
 * @lucene.experimental
 */
class BlendedInfixSuggester : public AnalyzingInfixSuggester
{
  GET_CLASS_NAME(BlendedInfixSuggester)

  /**
   * Coefficient used for linear blending
   */
protected:
  static double LINEAR_COEF;

private:
  std::optional<double> exponent = 2.0;

  /**
   * Default factor
   */
public:
  static int DEFAULT_NUM_FACTOR;

  /**
   * Factor to multiply the number of searched elements
   */
private:
  const int numFactor;

  /**
   * Type of blender used by the suggester
   */
  const BlenderType blenderType;

  /**
   * The different types of blender.
   */
public:
  enum class BlenderType {
    GET_CLASS_NAME(BlenderType)
    /** Application dependent; override {@link
     *  #calculateCoefficient} to compute it. */
    CUSTOM,
    /** weight*(1 - 0.10*position) */
    POSITION_LINEAR,
    /** weight/(1+position) */
    POSITION_RECIPROCAL,
    /** weight/pow(1+position, exponent) */
    POSITION_EXPONENTIAL_RECIPROCAL
    // TODO:
    // SCORE
  };

  /**
   * Create a new instance, loading from a previously built
   * directory, if it exists.
   */
public:
  BlendedInfixSuggester(std::shared_ptr<Directory> dir,
                        std::shared_ptr<Analyzer> analyzer) ;

  /**
   * Create a new instance, loading from a previously built
   * directory, if it exists.
   *
   * @param blenderType Type of blending strategy, see BlenderType for more
   * precisions
   * @param numFactor   Factor to multiply the number of searched elements
   * before ponderate
   * @param commitOnBuild Call commit after the index has finished building.
   * This would persist the suggester index to disk and future instances of this
   * suggester can use this pre-built dictionary.
   * @throws IOException If there are problems opening the underlying Lucene
   * index.
   */
  BlendedInfixSuggester(std::shared_ptr<Directory> dir,
                        std::shared_ptr<Analyzer> indexAnalyzer,
                        std::shared_ptr<Analyzer> queryAnalyzer,
                        int minPrefixChars, BlenderType blenderType,
                        int numFactor, bool commitOnBuild) ;

  /**
   * Create a new instance, loading from a previously built
   * directory, if it exists.
   *
   * @param blenderType Type of blending strategy, see BlenderType for more
   * precisions
   * @param numFactor   Factor to multiply the number of searched elements
   * before ponderate
   * @param exponent exponent used only when blenderType is
   * BlenderType.POSITION_EXPONENTIAL_RECIPROCAL
   * @param commitOnBuild Call commit after the index has finished building.
   * This would persist the suggester index to disk and future instances of this
   * suggester can use this pre-built dictionary.
   * @param allTermsRequired All terms in the suggest query must be matched.
   * @param highlight Highlight suggest query in suggestions.
   * @throws IOException If there are problems opening the underlying Lucene
   * index.
   */
  BlendedInfixSuggester(std::shared_ptr<Directory> dir,
                        std::shared_ptr<Analyzer> indexAnalyzer,
                        std::shared_ptr<Analyzer> queryAnalyzer,
                        int minPrefixChars, BlenderType blenderType,
                        int numFactor, std::optional<double> &exponent,
                        bool commitOnBuild, bool allTermsRequired,
                        bool highlight) ;

  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
         bool onlyMorePopular, int num)  override;

  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts, int num,
         bool allTermsRequired, bool doHighlight)  override;

  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::unordered_map<std::shared_ptr<BytesRef>, BooleanClause::Occur>
             &contextInfo,
         int num, bool allTermsRequired,
         bool doHighlight)  override;

  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<BooleanQuery> contextQuery, int num,
         bool allTermsRequired, bool doHighlight)  override;

protected:
  std::shared_ptr<FieldType> getTextFieldType() override;

  std::deque<std::shared_ptr<Lookup::LookupResult>>
  createResults(std::shared_ptr<IndexSearcher> searcher,
                std::shared_ptr<TopFieldDocs> hits, int num,
                std::shared_ptr<std::wstring> key, bool doHighlight,
                std::shared_ptr<Set<std::wstring>> matchedTokens,
                const std::wstring &prefixToken)  override;

  /**
   * Add an element to the tree respecting a size limit
   *
   * @param results the tree to add in
   * @param result the result we try to add
   * @param num size limit
   */
private:
  static void
  boundedTreeAdd(std::set<std::shared_ptr<Lookup::LookupResult>> &results,
                 std::shared_ptr<Lookup::LookupResult> result, int num);

  /**
   * Create the coefficient to transform the weight.
   *
   * @param doc id of the document
   * @param matchedTokens tokens found in the query
   * @param prefixToken unfinished token in the query
   * @return the coefficient
   * @throws IOException If there are problems reading term vectors from the
   * underlying Lucene index.
   */
  double createCoefficient(std::shared_ptr<IndexSearcher> searcher, int doc,
                           std::shared_ptr<Set<std::wstring>> matchedTokens,
                           const std::wstring &prefixToken) ;

  /**
   * Calculate the weight coefficient based on the position of the first
   * matching word. Subclass should override it to adapt it to particular needs
   * @param position of the first matching word in text
   * @return the coefficient
   */
protected:
  virtual double calculateCoefficient(int position);

private:
  static std::shared_ptr<Comparator<std::shared_ptr<Lookup::LookupResult>>>
      LOOKUP_COMP;

private:
  class LookUpComparator
      : public std::enable_shared_from_this<LookUpComparator>,
        public Comparator<std::shared_ptr<Lookup::LookupResult>>
  {
    GET_CLASS_NAME(LookUpComparator)

  public:
    int compare(std::shared_ptr<Lookup::LookupResult> o1,
                std::shared_ptr<Lookup::LookupResult> o2) override;
  };

protected:
  std::shared_ptr<BlendedInfixSuggester> shared_from_this()
  {
    return std::static_pointer_cast<BlendedInfixSuggester>(
        AnalyzingInfixSuggester::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::analyzing
