#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../util/ResourceLoaderAware.h"
#include "../util/TokenFilterFactory.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/synonym/SynonymMap.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include  "core/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"

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
namespace org::apache::lucene::analysis::synonym
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;

/**
 * Factory for {@link SynonymFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_synonym" class="solr.TextField"
positionIncrementGap="100"&gt;
 *   &lt;analyzer&gt;
 *     &lt;tokenizer class="solr.WhitespaceTokenizerFactory"/&gt;
 *     &lt;filter class="solr.SynonymFilterFactory" synonyms="synonyms.txt"
 *             format="solr" ignoreCase="false" expand="true"
 *             tokenizerFactory="solr.WhitespaceTokenizerFactory"
 *             [optional tokenizer factory parameters]/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * <p>
 * An optional param name prefix of "tokenizerFactory." may be used for any
 * init params that the SynonymFilterFactory needs to pass to the specified
 * TokenizerFactory.  If the TokenizerFactory expects an init parameters with
 * the same name as an init param used by the SynonymFilterFactory, the prefix
 * is mandatory.
 * </p>
 *
 * <p>
 * The optional {@code format} parameter controls how the synonyms will be
parsed:
 * It supports the short names of {@code solr} for {@link SolrSynonymParser}
 * and {@code wordnet} for and {@link WordnetSynonymParser}, or your own
 * {@code SynonymMap.Parser} class name. The default is {@code solr}.
 * A custom {@link SynonymMap.Parser} is expected to have a constructor taking:
GET_CLASS_NAME(name.)
 * <ul>
 *   <li><code>bool dedup</code> - true if duplicates should be ignored,
false otherwise</li>
 *   <li><code>bool expand</code> - true if conflation groups should be
expanded, false if they are one-directional</li>
 *   <li><code>{@link Analyzer} analyzer</code> - an analyzer used for each raw
synonym</li>
 * </ul>
 * @see SolrSynonymParser SolrSynonymParser: default format
 *
 * @deprecated Use {@link SynonymGraphFilterFactory} instead, but be sure to
also
 * use {@link FlattenGraphFilterFactory} at index time (not at search time) as
well.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public class SynonymFilterFactory extends
// org.apache.lucene.analysis.util.TokenFilterFactory implements
// org.apache.lucene.analysis.util.ResourceLoaderAware
class SynonymFilterFactory : public TokenFilterFactory,
                             public ResourceLoaderAware
{
private:
  const bool ignoreCase;
  const std::wstring tokenizerFactory;
  const std::wstring synonyms;
  const std::wstring format;
  const bool expand;
  const std::wstring analyzerName;
  const std::unordered_map<std::wstring, std::wstring> tokArgs =
      std::unordered_map<std::wstring, std::wstring>();

  std::shared_ptr<SynonymMap> map_obj;

public:
  SynonymFilterFactory(std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<SynonymFilterFactory> outerInstance;

    std::shared_ptr<TokenizerFactory> factory;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<SynonymFilterFactory> outerInstance,
        std::shared_ptr<TokenizerFactory> factory);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  /**
   * Load synonyms with the given {@link SynonymMap.Parser} class.
   */
protected:
  virtual std::shared_ptr<SynonymMap>
  loadSynonyms(std::shared_ptr<ResourceLoader> loader,
               const std::wstring &cname, bool dedup,
               std::shared_ptr<Analyzer> analyzer) throw(IOException,
                                                         ParseException);

  // (there are no tests for this functionality)
private:
  std::shared_ptr<TokenizerFactory>
  loadTokenizerFactory(std::shared_ptr<ResourceLoader> loader,
                       const std::wstring &cname) ;

  std::shared_ptr<Analyzer>
  loadAnalyzer(std::shared_ptr<ResourceLoader> loader,
               const std::wstring &cname) ;

protected:
  std::shared_ptr<SynonymFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<SynonymFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/synonym/
