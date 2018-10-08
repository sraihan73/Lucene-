#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionLengthAttribute;
}
namespace org::apache::lucene::analysis::commongrams
{
class CommonGramsFilter;
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
namespace org::apache::lucene::analysis::commongrams
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.commongrams.CommonGramsFilter.GRAM_TYPE;

/**
 * Wrap a CommonGramsFilter optimizing phrase queries by only returning single
 * words when they are not a member of a bigram.
 *
 * Example:
 * <ul>
 * <li>query input to CommonGramsFilter: "the rain in spain falls mainly"
 * <li>output of CommomGramsFilter/input to CommonGramsQueryFilter:
 * |"the, "the-rain"|"rain" "rain-in"|"in, "in-spain"|"spain"|"falls"|"mainly"
 * <li>output of CommonGramsQueryFilter:"the-rain", "rain-in" ,"in-spain",
 * "falls", "mainly"
 * </ul>
 */

/*
 * See:http://hudson.zones.apache.org/hudson/job/Lucene-trunk/javadoc//all/org/apache/lucene/analysis/TokenStream.html
 * and
 * http://svn.apache.org/viewvc/lucene/dev/trunk/lucene/src/java/org/apache/lucene/analysis/package.html?revision=718798
 */
class CommonGramsQueryFilter final : public TokenFilter
{
  GET_CLASS_NAME(CommonGramsQueryFilter)

private:
  const std::shared_ptr<TypeAttribute> typeAttribute =
      addAttribute(TypeAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAttribute =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLengthAttribute =
      addAttribute(PositionLengthAttribute::typeid);

  std::shared_ptr<State> previous;
  std::wstring previousType;
  bool exhausted = false;

  /**
   * Constructs a new CommonGramsQueryFilter based on the provided
   * CommomGramsFilter
   *
   * @param input CommonGramsFilter the QueryFilter will use
   */
public:
  CommonGramsQueryFilter(std::shared_ptr<CommonGramsFilter> input);

  void reset()  override;

  /**
   * Output bigrams whenever possible to optimize queries. Only output unigrams
   * when they are not a member of a bigram. Example:
   * <ul>
   * <li>input: "the rain in spain falls mainly"
   * <li>output:"the-rain", "rain-in" ,"in-spain", "falls", "mainly"
   * </ul>
   */
  bool incrementToken()  override;

  // ================================================= Helper Methods
  // ================================================

  /**
   * Convenience method to check if the current type is a gram type
   *
   * @return {@code true} if the current type is a gram type, {@code false}
   * otherwise
   */
  bool isGramType();

protected:
  std::shared_ptr<CommonGramsQueryFilter> shared_from_this()
  {
    return std::static_pointer_cast<CommonGramsQueryFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::commongrams
