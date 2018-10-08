#pragma once
#include "../util/ResourceLoaderAware.h"
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
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
namespace org::apache::lucene::analysis::miscellaneous
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

using namespace org::apache::lucene::analysis::miscellaneous;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.miscellaneous.WordDelimiterFilter.*;

/**
 * Factory for {@link WordDelimiterFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_wd" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.WordDelimiterFilterFactory" protected="protectedword.txt"
 *             preserveOriginal="0" splitOnNumerics="1" splitOnCaseChange="1"
 *             catenateWords="0" catenateNumbers="0" catenateAll="0"
 *             generateWordParts="1" generateNumberParts="1"
 * stemEnglishPossessive="1" types="wdfftypes.txt" /&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * @deprecated Use {@link WordDelimiterGraphFilterFactory} instead: it produces
 * a correct token graph so that e.g. {@link PhraseQuery} works correctly when
 * it's used in the search time analyzer.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public class WordDelimiterFilterFactory extends
// org.apache.lucene.analysis.util.TokenFilterFactory implements
// org.apache.lucene.analysis.util.ResourceLoaderAware
class WordDelimiterFilterFactory : public TokenFilterFactory,
                                   public ResourceLoaderAware
{
public:
  static const std::wstring PROTECTED_TOKENS;
  static const std::wstring TYPES;

private:
  const std::wstring wordFiles;
  const std::wstring types;
  const int flags;

public:
  std::deque<char> typeTable;

private:
  std::shared_ptr<CharArraySet> protectedWords = nullptr;

  /** Creates a new WordDelimiterFilterFactory */
public:
  WordDelimiterFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  std::shared_ptr<TokenFilter>
  create(std::shared_ptr<TokenStream> input) override;

  // source => type
private:
  static std::shared_ptr<Pattern> typePattern;

  // parses a deque of MappingCharFilter style rules into a custom byte[] type
  // table
  std::deque<char> parseTypes(std::deque<std::wstring> &rules);

  std::optional<char> parseType(const std::wstring &s);

public:
  std::deque<wchar_t> out = std::deque<wchar_t>(256);

private:
  std::wstring parseString(const std::wstring &s);

protected:
  std::shared_ptr<WordDelimiterFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<WordDelimiterFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
