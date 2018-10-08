#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis::tr
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * Strips all characters after an apostrophe (including the apostrophe itself).
 * <p>
 * In Turkish, apostrophe is used to separate suffixes from proper names
 * (continent, sea, river, lake, mountain, upland, proper names related to
 * religion and mythology). This filter intended to be used before stem filters.
 * For more information, see <a
 * href="http://www.ipcsit.com/vol57/015-ICNI2012-M021.pdf"> Role of Apostrophes
 * in Turkish Information Retrieval</a>
 * </p>
 */
class ApostropheFilter final : public TokenFilter
{
  GET_CLASS_NAME(ApostropheFilter)

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

public:
  ApostropheFilter(std::shared_ptr<TokenStream> in_);

  bool incrementToken()  override final;

protected:
  std::shared_ptr<ApostropheFilter> shared_from_this()
  {
    return std::static_pointer_cast<ApostropheFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/tr/
