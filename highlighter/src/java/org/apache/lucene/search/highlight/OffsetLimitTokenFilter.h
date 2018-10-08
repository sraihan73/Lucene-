#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"

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
namespace org::apache::lucene::search::highlight
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;

/**
 * This TokenFilter limits the number of tokens while indexing by adding up the
 * current offset.
 */
class OffsetLimitTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(OffsetLimitTokenFilter)

private:
  int offsetCount = 0;
  std::shared_ptr<OffsetAttribute> offsetAttrib =
      getAttribute(OffsetAttribute::typeid);
  int offsetLimit = 0;

public:
  OffsetLimitTokenFilter(std::shared_ptr<TokenStream> input, int offsetLimit);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<OffsetLimitTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<OffsetLimitTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/
