#pragma once
#include "../util/CharFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/charfilter/HTMLStripCharFilter.h"

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
namespace org::apache::lucene::analysis::charfilter
{

using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;

/**
 * Factory for {@link HTMLStripCharFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_html" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;charFilter
 * class="solr.HTMLStripCharFilterFactory" escapedTags="a, title" /&gt;
 *     &lt;tokenizer class="solr.WhitespaceTokenizerFactory"/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 */
class HTMLStripCharFilterFactory : public CharFilterFactory
{
  GET_CLASS_NAME(HTMLStripCharFilterFactory)
public:
  const std::shared_ptr<Set<std::wstring>> escapedTags;
  static const std::shared_ptr<Pattern> TAG_NAME_PATTERN;

  /** Creates a new HTMLStripCharFilterFactory */
  HTMLStripCharFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<HTMLStripCharFilter>
  create(std::shared_ptr<Reader> input) override;

protected:
  std::shared_ptr<HTMLStripCharFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<HTMLStripCharFilterFactory>(
        org.apache.lucene.analysis.util.CharFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/charfilter/
