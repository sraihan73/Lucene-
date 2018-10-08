#pragma once
#include "../../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoaderAware.h"
#include "../../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "DefaultICUTokenizerConfig.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/ICUTokenizerConfig.h"

#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/ICUTokenizer.h"
#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

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
namespace org::apache::lucene::analysis::icu::segmentation
{

using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

using com::ibm::icu::text::BreakIterator;

/**
 * Factory for {@link ICUTokenizer}.
 * Words are broken across script boundaries, then segmented according to
 * the BreakIterator and typing provided by the {@link
 * DefaultICUTokenizerConfig}.
 *
 * <p>
 * To use the default set of per-script rules:
 *
 * <pre class="prettyprint" >
 * &lt;fieldType name="text_icu" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.ICUTokenizerFactory"/&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * <p>
 * You can customize this tokenizer's behavior by specifying per-script rule
 * files, which are compiled by the ICU RuleBasedBreakIterator.  See the <a
 * href="http://userguide.icu-project.org/boundaryanalysis#TOC-RBBI-Rules" >ICU
 * RuleBasedBreakIterator syntax reference</a>.
 *
 * <p>
 * To add per-script rules, add a "rulefiles" argument, which should contain a
 * comma-separated deque of <tt>code:rulefile</tt> pairs in the following format:
 * <a href="http://unicode.org/iso15924/iso15924-codes.html"
 * >four-letter ISO 15924 script code</a>, followed by a colon, then a resource
 * path.  E.g. to specify rules for Latin (script code "Latn") and Cyrillic
 * (script code "Cyrl"):
 *
 * <pre class="prettyprint" >
 * &lt;fieldType name="text_icu_custom" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.ICUTokenizerFactory" cjkAsWords="true"
 *                rulefiles="Latn:my.Latin.rules.rbbi,Cyrl:my.Cyrillic.rules.rbbi"/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 */
class ICUTokenizerFactory : public TokenizerFactory, public ResourceLoaderAware
{
  GET_CLASS_NAME(ICUTokenizerFactory)
public:
  static const std::wstring RULEFILES;

private:
  const std::unordered_map<int, std::wstring> tailored;
  std::shared_ptr<ICUTokenizerConfig> config;
  const bool cjkAsWords;
  const bool myanmarAsWords;

  /** Creates a new ICUTokenizerFactory */
public:
  ICUTokenizerFactory(std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

private:
  class DefaultICUTokenizerConfigAnonymousInnerClass
      : public DefaultICUTokenizerConfig
  {
    GET_CLASS_NAME(DefaultICUTokenizerConfigAnonymousInnerClass)
  private:
    std::shared_ptr<ICUTokenizerFactory> outerInstance;

    std::deque<std::shared_ptr<BreakIterator>> breakers;

  public:
    DefaultICUTokenizerConfigAnonymousInnerClass(
        std::shared_ptr<ICUTokenizerFactory> outerInstance, bool cjkAsWords,
        bool myanmarAsWords,
        std::deque<std::shared_ptr<BreakIterator>> &breakers);

    std::shared_ptr<RuleBasedBreakIterator>
    getBreakIterator(int script) override;
    // TODO: we could also allow codes->types mapping

  protected:
    std::shared_ptr<DefaultICUTokenizerConfigAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DefaultICUTokenizerConfigAnonymousInnerClass>(
          DefaultICUTokenizerConfig::shared_from_this());
    }
  };

private:
  std::shared_ptr<BreakIterator>
  parseRules(const std::wstring &filename,
             std::shared_ptr<ResourceLoader> loader) ;

public:
  std::shared_ptr<ICUTokenizer>
  create(std::shared_ptr<AttributeFactory> factory) override;

protected:
  std::shared_ptr<ICUTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<ICUTokenizerFactory>(
        org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/
