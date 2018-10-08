#pragma once
#include "CommonGramsFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenFilter.h"

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
namespace org::apache::lucene::analysis::commongrams
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * Construct {@link CommonGramsQueryFilter}.
 *
 * <pre class="prettyprint">
 * &lt;fieldType name="text_cmmngrmsqry" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.CommonGramsQueryFilterFactory"
 * words="commongramsquerystopwords.txt" ignoreCase="false"/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 */
class CommonGramsQueryFilterFactory : public CommonGramsFilterFactory
{
  GET_CLASS_NAME(CommonGramsQueryFilterFactory)

  /** Creates a new CommonGramsQueryFilterFactory */
public:
  CommonGramsQueryFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  /**
   * Create a CommonGramsFilter and wrap it with a CommonGramsQueryFilter
   */
  std::shared_ptr<TokenFilter>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<CommonGramsQueryFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<CommonGramsQueryFilterFactory>(
        CommonGramsFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/commongrams/
