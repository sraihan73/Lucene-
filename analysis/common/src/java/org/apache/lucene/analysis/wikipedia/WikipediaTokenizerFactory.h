#pragma once
#include "../util/TokenizerFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::wikipedia
{
class WikipediaTokenizer;
}

namespace org::apache::lucene::util
{
class AttributeFactory;
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
namespace org::apache::lucene::analysis::wikipedia
{

using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Factory for {@link WikipediaTokenizer}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_wiki" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WikipediaTokenizerFactory"/&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 */
class WikipediaTokenizerFactory : public TokenizerFactory
{
  GET_CLASS_NAME(WikipediaTokenizerFactory)
public:
  static const std::wstring TOKEN_OUTPUT;
  static const std::wstring UNTOKENIZED_TYPES;

protected:
  const int tokenOutput;
  std::shared_ptr<Set<std::wstring>> untokenizedTypes;

  /** Creates a new WikipediaTokenizerFactory */
public:
  WikipediaTokenizerFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<WikipediaTokenizer>
  create(std::shared_ptr<AttributeFactory> factory) override;

protected:
  std::shared_ptr<WikipediaTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<WikipediaTokenizerFactory>(
        org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::wikipedia
