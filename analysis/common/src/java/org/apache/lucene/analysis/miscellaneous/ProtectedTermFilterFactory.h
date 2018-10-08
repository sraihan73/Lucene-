#pragma once
#include "../util/ResourceLoaderAware.h"
#include "ConditionalTokenFilterFactory.h"
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/analysis/miscellaneous/ConditionalTokenFilter.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"

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
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;

/**
 * Factory for a {@link ProtectedTermFilter}
 *
 * <p>CustomAnalyzer example:
 * <pre class="prettyprint">
 * Analyzer ana = CustomAnalyzer.builder()
 *   .withTokenizer("standard")
 *   .when("protectedterm", "ignoreCase", "true", "protected",
 * "protectedTerms.txt") .addTokenFilter("truncate", "prefixLength", "4")
 *     .addTokenFilter("lowercase")
 *   .endwhen()
 *   .build();
 * </pre>
 *
 * <p>Solr example, in which conditional filters are specified via the
 * <code>wrappedFilters</code> parameter - a comma-separated deque of
 * case-insensitive TokenFilter SPI names - and conditional filter args are
 * specified via <code>filterName.argName</code> parameters: <pre
 * class="prettyprint"> &lt;fieldType name="reverse_lower_with_exceptions"
 * class="solr.TextField" positionIncrementGap="100"&gt; &lt;analyzer&gt;
 *     &lt;tokenizer class="solr.WhitespaceTokenizerFactory"/&gt;
 *     &lt;filter class="solr.ProtectedTermFilterFactory" ignoreCase="true"
 * protected="protectedTerms.txt" wrappedFilters="truncate,lowercase"
 * truncate.prefixLength="4" /&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre>
 *
 * <p>When using the <code>wrappedFilters</code> parameter, each filter name
 * must be unique, so if you need to specify the same filter more than once, you
 * must add case-insensitive unique '-id' suffixes (note that the '-id' suffix
 * is stripped prior to SPI lookup), e.g.: <pre class="prettyprint">
 * &lt;fieldType name="double_synonym_with_exceptions" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.ProtectedTermFilterFactory" ignoreCase="true"
 * protected="protectedTerms.txt" wrappedFilters="synonymgraph-A,synonymgraph-B"
 *             synonymgraph-A.synonyms="synonyms-1.txt"
 *             synonymgraph-B.synonyms="synonyms-2.txt"/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * <p>See related {@link
 * org.apache.lucene.analysis.custom.CustomAnalyzer.Builder#whenTerm(Predicate)}
 */
class ProtectedTermFilterFactory : public ConditionalTokenFilterFactory,
                                   public ResourceLoaderAware
{
  GET_CLASS_NAME(ProtectedTermFilterFactory)

public:
  static const std::wstring PROTECTED_TERMS;
  static constexpr wchar_t FILTER_ARG_SEPARATOR = L'.';
  static constexpr wchar_t FILTER_NAME_ID_SEPARATOR = L'-';

private:
  const std::wstring termFiles;
  const bool ignoreCase;
  const std::wstring wrappedFilters;

  std::shared_ptr<CharArraySet> protectedTerms;

public:
  ProtectedTermFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

private:
  void
  handleWrappedFilterArgs(std::unordered_map<std::wstring, std::wstring> &args);

  void populateInnerFilters(
      std::shared_ptr<LinkedHashMap<
          std::wstring, std::unordered_map<std::wstring, std::wstring>>>
          wrappedFilterArgs);

public:
  virtual bool isIgnoreCase();

  virtual std::shared_ptr<CharArraySet> getProtectedTerms();

protected:
  std::shared_ptr<ConditionalTokenFilter>
  create(std::shared_ptr<TokenStream> input,
         std::function<TokenStream *(TokenStream *)> &inner) override;

public:
  void
  doInform(std::shared_ptr<ResourceLoader> loader)  override;

protected:
  std::shared_ptr<ProtectedTermFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<ProtectedTermFilterFactory>(
        ConditionalTokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
