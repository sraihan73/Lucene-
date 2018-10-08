#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/SetOnce.h"
#include "../miscellaneous/ConditionalTokenFilter.h"
#include "../miscellaneous/ConditionalTokenFilterFactory.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::custom
{
class Builder;
}

namespace org::apache::lucene::analysis::util
{
class ResourceLoader;
}
namespace org::apache::lucene::analysis::util
{
class CharFilterFactory;
}
namespace org::apache::lucene::analysis::util
{
class TokenizerFactory;
}
namespace org::apache::lucene::analysis::util
{
class TokenFilterFactory;
}
namespace org::apache::lucene::util
{
class Version;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::analysis::custom
{
class ConditionBuilder;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::miscellaneous
{
class ConditionalTokenFilterFactory;
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
namespace org::apache::lucene::analysis::custom
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ConditionalTokenFilterFactory =
    org::apache::lucene::analysis::miscellaneous::ConditionalTokenFilterFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using SetOnce = org::apache::lucene::util::SetOnce;
using Version = org::apache::lucene::util::Version;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.util.AnalysisSPILoader.newFactoryClassInstance;

/**
 * A general-purpose Analyzer that can be created with a builder-style API.
 * Under the hood it uses the factory classes {@link TokenizerFactory},
 * {@link TokenFilterFactory}, and {@link CharFilterFactory}.
GET_CLASS_NAME(es)
 * <p>You can create an instance of this Analyzer using the builder:
 * <pre class="prettyprint">
 * Analyzer ana =
CustomAnalyzer.builder(Paths.get(&quot;/path/to/config/dir&quot;))
 *   .withTokenizer(StandardTokenizerFactory.class)
 *   .addTokenFilter(StandardFilterFactory.class)
 *   .addTokenFilter(LowerCaseFilterFactory.class)
 *   .addTokenFilter(StopFilterFactory.class, &quot;ignoreCase&quot;,
&quot;false&quot;, &quot;words&quot;, &quot;stopwords.txt&quot;,
&quot;format&quot;, &quot;wordset&quot;)
 *   .build();
 * </pre>
 * The parameters passed to components are also used by Apache Solr and are
documented
 * on their corresponding factory classes. Refer to documentation of subclasses
 * of {@link TokenizerFactory}, {@link TokenFilterFactory}, and {@link
CharFilterFactory}. GET_CLASS_NAME(es.)
 * <p>You can also use the SPI names (as defined by {@link
java.util.ServiceLoader} interface):
 * <pre class="prettyprint">
 * Analyzer ana =
CustomAnalyzer.builder(Paths.get(&quot;/path/to/config/dir&quot;))
 *   .withTokenizer(&quot;standard&quot;)
 *   .addTokenFilter(&quot;standard&quot;)
 *   .addTokenFilter(&quot;lowercase&quot;)
 *   .addTokenFilter(&quot;stop&quot;, &quot;ignoreCase&quot;,
&quot;false&quot;, &quot;words&quot;, &quot;stopwords.txt&quot;,
&quot;format&quot;, &quot;wordset&quot;)
 *   .build();
 * </pre>
 * <p>The deque of names to be used for components can be looked up through:
 * {@link TokenizerFactory#availableTokenizers()}, {@link
TokenFilterFactory#availableTokenFilters()},
 * and {@link CharFilterFactory#availableCharFilters()}.
 * <p>You can create conditional branches in the analyzer by using {@link
Builder#when(std::wstring, std::wstring...)} and
 * {@link Builder#whenTerm(Predicate)}:
 * <pre class="prettyprint">
 * Analyzer ana = CustomAnalyzer.builder()
 *    .withTokenizer(&quot;standard&quot;)
 *    .addTokenFilter(&quot;lowercase&quot;)
 *    .whenTerm(t -&gt; t.length() &gt; 10)
 *      .addTokenFilter(&quot;reversestring&quot;)
 *    .endwhen()
 *    .build();
 * </pre>
 */
class CustomAnalyzer final : public Analyzer
{
  GET_CLASS_NAME(CustomAnalyzer)

  /**
   * Returns a builder for custom analyzers that loads all resources from
   * Lucene's classloader. All path names given must be absolute with package
   * prefixes.
   */
public:
  static std::shared_ptr<Builder> builder();

  /**
   * Returns a builder for custom analyzers that loads all resources from the
   * given file system base directory. Place, e.g., stop word files there. Files
   * that are not in the given directory are loaded from Lucene's classloader.
   */
  static std::shared_ptr<Builder> builder(std::shared_ptr<Path> configDir);

  /** Returns a builder for custom analyzers that loads all resources using the
   * given {@link ResourceLoader}. */
  static std::shared_ptr<Builder>
  builder(std::shared_ptr<ResourceLoader> loader);

private:
  std::deque<std::shared_ptr<CharFilterFactory>> const charFilters;
  const std::shared_ptr<TokenizerFactory> tokenizer;
  std::deque<std::shared_ptr<TokenFilterFactory>> const tokenFilters;
  const std::optional<int> posIncGap, offsetGap;

public:
  CustomAnalyzer(std::shared_ptr<Version> defaultMatchVersion,
                 std::deque<std::shared_ptr<CharFilterFactory>> &charFilters,
                 std::shared_ptr<TokenizerFactory> tokenizer,
                 std::deque<std::shared_ptr<TokenFilterFactory>> &tokenFilters,
                 std::optional<int> &posIncGap, std::optional<int> &offsetGap);

protected:
  std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                     std::shared_ptr<Reader> reader) override;

  std::shared_ptr<Reader>
  initReaderForNormalization(const std::wstring &fieldName,
                             std::shared_ptr<Reader> reader) override;

  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

public:
  int getPositionIncrementGap(const std::wstring &fieldName) override;

  int getOffsetGap(const std::wstring &fieldName) override;

  /** Returns the deque of char filters that are used in this analyzer. */
  std::deque<std::shared_ptr<CharFilterFactory>> getCharFilterFactories();

  /** Returns the tokenizer that is used in this analyzer. */
  std::shared_ptr<TokenizerFactory> getTokenizerFactory();

  /** Returns the deque of token filters that are used in this analyzer. */
  std::deque<std::shared_ptr<TokenFilterFactory>> getTokenFilterFactories();

  virtual std::wstring toString();

  /** Builder for {@link CustomAnalyzer}.
   * @see CustomAnalyzer#builder()
   * @see CustomAnalyzer#builder(Path)
   * @see CustomAnalyzer#builder(ResourceLoader)
   */
public:
  class Builder final : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)
  private:
    const std::shared_ptr<ResourceLoader> loader;
    const std::shared_ptr<SetOnce<std::shared_ptr<Version>>>
        defaultMatchVersion =
            std::make_shared<SetOnce<std::shared_ptr<Version>>>();
    const std::deque<std::shared_ptr<CharFilterFactory>> charFilters =
        std::deque<std::shared_ptr<CharFilterFactory>>();
    const std::shared_ptr<SetOnce<std::shared_ptr<TokenizerFactory>>>
        tokenizer =
            std::make_shared<SetOnce<std::shared_ptr<TokenizerFactory>>>();
    const std::deque<std::shared_ptr<TokenFilterFactory>> tokenFilters =
        std::deque<std::shared_ptr<TokenFilterFactory>>();
    const std::shared_ptr<SetOnce<int>> posIncGap =
        std::make_shared<SetOnce<int>>();
    const std::shared_ptr<SetOnce<int>> offsetGap =
        std::make_shared<SetOnce<int>>();

    bool componentsAdded = false;

  public:
    Builder(std::shared_ptr<ResourceLoader> loader);

    /** This match version is passed as default to all tokenizers or filters. It
     * is used unless you pass the parameter {code luceneMatchVersion}
     * explicitly. It defaults to undefined, so the
     * underlying factory will (in most cases) use {@link Version#LATEST}. */
    std::shared_ptr<Builder>
    withDefaultMatchVersion(std::shared_ptr<Version> version);

    /** Sets the position increment gap of the analyzer.
     * The default is defined in the analyzer base class.
     * @see Analyzer#getPositionIncrementGap(std::wstring)
     */
    std::shared_ptr<Builder> withPositionIncrementGap(int posIncGap);

    /** Sets the offset gap of the analyzer. The default is defined
     * in the analyzer base class.
     * @see Analyzer#getOffsetGap(std::wstring)
     */
    std::shared_ptr<Builder> withOffsetGap(int offsetGap);

    /** Uses the given tokenizer.
     * @param factory class that is used to create the tokenizer.
     * @param params a deque of factory string params as key/value pairs.
     *  The number of parameters must be an even number, as they are pairs.
     */
    std::shared_ptr<Builder>
    withTokenizer(std::type_info factory,
                  std::deque<std::wstring> &params) ;

    /** Uses the given tokenizer.
     * @param factory class that is used to create the tokenizer.
     * @param params the map_obj of parameters to be passed to factory. The map_obj must
     * be modifiable.
     */
    std::shared_ptr<Builder>
    withTokenizer(std::type_info factory,
                  std::unordered_map<std::wstring, std::wstring>
                      &params) ;

    /** Uses the given tokenizer.
     * @param name is used to look up the factory with {@link
     * TokenizerFactory#forName(std::wstring, Map)}. The deque of possible names can be
     * looked up with {@link TokenizerFactory#availableTokenizers()}.
     * @param params a deque of factory string params as key/value pairs.
     *  The number of parameters must be an even number, as they are pairs.
     */
    std::shared_ptr<Builder>
    withTokenizer(const std::wstring &name,
                  std::deque<std::wstring> &params) ;

    /** Uses the given tokenizer.
     * @param name is used to look up the factory with {@link
     * TokenizerFactory#forName(std::wstring, Map)}. The deque of possible names can be
     * looked up with {@link TokenizerFactory#availableTokenizers()}.
     * @param params the map_obj of parameters to be passed to factory. The map_obj must
     * be modifiable.
     */
    std::shared_ptr<Builder>
    withTokenizer(const std::wstring &name,
                  std::unordered_map<std::wstring, std::wstring>
                      &params) ;

    /** Adds the given token filter.
     * @param factory class that is used to create the token filter.
     * @param params a deque of factory string params as key/value pairs.
     *  The number of parameters must be an even number, as they are pairs.
     */
    std::shared_ptr<Builder>
    addTokenFilter(std::type_info factory,
                   std::deque<std::wstring> &params) ;

    /** Adds the given token filter.
     * @param factory class that is used to create the token filter.
     * @param params the map_obj of parameters to be passed to factory. The map_obj must
     * be modifiable.
     */
    std::shared_ptr<Builder>
    addTokenFilter(std::type_info factory,
                   std::unordered_map<std::wstring, std::wstring>
                       &params) ;

    /** Adds the given token filter.
     * @param name is used to look up the factory with {@link
     * TokenFilterFactory#forName(std::wstring, Map)}. The deque of possible names can
     * be looked up with {@link TokenFilterFactory#availableTokenFilters()}.
     * @param params a deque of factory string params as key/value pairs.
     *  The number of parameters must be an even number, as they are pairs.
     */
    std::shared_ptr<Builder>
    addTokenFilter(const std::wstring &name,
                   std::deque<std::wstring> &params) ;

    /** Adds the given token filter.
     * @param name is used to look up the factory with {@link
     * TokenFilterFactory#forName(std::wstring, Map)}. The deque of possible names can
     * be looked up with {@link TokenFilterFactory#availableTokenFilters()}.
     * @param params the map_obj of parameters to be passed to factory. The map_obj must
     * be modifiable.
     */
    std::shared_ptr<Builder>
    addTokenFilter(const std::wstring &name,
                   std::unordered_map<std::wstring, std::wstring>
                       &params) ;

  private:
    std::shared_ptr<Builder>
    addTokenFilter(std::shared_ptr<TokenFilterFactory> factory);

    /** Adds the given char filter.
     * @param factory class that is used to create the char filter.
     * @param params a deque of factory string params as key/value pairs.
     *  The number of parameters must be an even number, as they are pairs.
     */
  public:
    std::shared_ptr<Builder>
    addCharFilter(std::type_info factory,
                  std::deque<std::wstring> &params) ;

    /** Adds the given char filter.
     * @param factory class that is used to create the char filter.
     * @param params the map_obj of parameters to be passed to factory. The map_obj must
     * be modifiable.
     */
    std::shared_ptr<Builder>
    addCharFilter(std::type_info factory,
                  std::unordered_map<std::wstring, std::wstring>
                      &params) ;

    /** Adds the given char filter.
     * @param name is used to look up the factory with {@link
     * CharFilterFactory#forName(std::wstring, Map)}. The deque of possible names can
     * be looked up with {@link CharFilterFactory#availableCharFilters()}.
     * @param params a deque of factory string params as key/value pairs.
     *  The number of parameters must be an even number, as they are pairs.
     */
    std::shared_ptr<Builder>
    addCharFilter(const std::wstring &name,
                  std::deque<std::wstring> &params) ;

    /** Adds the given char filter.
     * @param name is used to look up the factory with {@link
     * CharFilterFactory#forName(std::wstring, Map)}. The deque of possible names can
     * be looked up with {@link CharFilterFactory#availableCharFilters()}.
     * @param params the map_obj of parameters to be passed to factory. The map_obj must
     * be modifiable.
     */
    std::shared_ptr<Builder>
    addCharFilter(const std::wstring &name,
                  std::unordered_map<std::wstring, std::wstring>
                      &params) ;

    /**
     * Add a {@link ConditionalTokenFilterFactory} to the analysis chain
     *
     * TokenFilters added by subsequent calls to {@link
     * ConditionBuilder#addTokenFilter(std::wstring, std::wstring...)} and related functions
     * will only be used if the current token matches the condition.  Consumers
     * must call {@link ConditionBuilder#endwhen()} to return to the normal
     * tokenfilter chain once conditional filters have been added
     *
     * @param name    is used to look up the factory with {@link
     * TokenFilterFactory#forName(std::wstring, Map)}
     * @param params  the parameters to be passed to the factory
     */
    std::shared_ptr<ConditionBuilder>
    when(const std::wstring &name,
         std::deque<std::wstring> &params) ;

    /**
     * Add a {@link ConditionalTokenFilterFactory} to the analysis chain
     *
     * TokenFilters added by subsequent calls to {@link
     * ConditionBuilder#addTokenFilter(std::wstring, std::wstring...)} and related functions
     * will only be used if the current token matches the condition.  Consumers
     * must call {@link ConditionBuilder#endwhen()} to return to the normal
     * tokenfilter chain once conditional filters have been added
     *
     * @param name    is used to look up the factory with {@link
     * TokenFilterFactory#forName(std::wstring, Map)}
     * @param params  the parameters to be passed to the factory.  The map_obj must
     * be modifiable
     */
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") public ConditionBuilder
    // when(std::wstring name, java.util.Map<std::wstring, std::wstring> params) throws
    // java.io.IOException
    std::shared_ptr<ConditionBuilder>
    when(const std::wstring &name,
         std::unordered_map<std::wstring, std::wstring>
             &params) ;

    /**
     * Add a {@link ConditionalTokenFilterFactory} to the analysis chain
     *
     * TokenFilters added by subsequent calls to {@link
     * ConditionBuilder#addTokenFilter(std::wstring, std::wstring...)} and related functions
     * will only be used if the current token matches the condition.  Consumers
     * must call {@link ConditionBuilder#endwhen()} to return to the normal
     * tokenfilter chain once conditional filters have been added
     *
     * @param factory class that is used to create the ConditionalTokenFilter
     * @param params  the parameters to be passed to the factory
     */
    std::shared_ptr<ConditionBuilder>
    when(std::type_info factory,
         std::deque<std::wstring> &params) ;

    /**
     * Add a {@link ConditionalTokenFilterFactory} to the analysis chain
     *
     * TokenFilters added by subsequent calls to {@link
     * ConditionBuilder#addTokenFilter(std::wstring, std::wstring...)} and related functions
     * will only be used if the current token matches the condition.  Consumers
     * must call {@link ConditionBuilder#endwhen()} to return to the normal
     * tokenfilter chain once conditional filters have been added
     *
     * @param factory class that is used to create the ConditionalTokenFilter
     * @param params  the parameters to be passed to the factory.  The map_obj must
     * be modifiable
     */
    std::shared_ptr<ConditionBuilder>
    when(std::type_info factory, std::unordered_map<std::wstring, std::wstring>
                                     &params) ;

    /**
     * Add a {@link ConditionalTokenFilterFactory} to the analysis chain
     *
     * TokenFilters added by subsequent calls to {@link
     * ConditionBuilder#addTokenFilter(std::wstring, std::wstring...)} and related functions
     * will only be used if the current token matches the condition.  Consumers
     * must call {@link ConditionBuilder#endwhen()} to return to the normal
     * tokenfilter chain once conditional filters have been added
     */
    std::shared_ptr<ConditionBuilder>
    when(std::shared_ptr<ConditionalTokenFilterFactory> factory);

    /**
     * Apply subsequent token filters if the current token's term matches a
predicate
     *
     * This is the equivalent of:
     * <pre>
     *   when(new ConditionalTokenFilterFactory(Collections.emptyMap()) {
     *      {@code @}Override
     *      protected ConditionalTokenFilter create(TokenStream input,
Function&lt;TokenStream, TokenStream&gt; inner) {
     *        return new ConditionalTokenFilter(input, inner) {
     *          CharTermAttribute termAtt =
addAttribute(CharTermAttribute.class);
     *          {@code @}Override
GET_CLASS_NAME();)
     *          protected bool shouldFilter() {
     *            return predicate.test(termAtt);
     *          }
     *        };
     *      }
     *   });
     * </pre>
     */
    std::shared_ptr<ConditionBuilder>
    whenTerm(std::function<bool(std::wstring *)> &predicate);

  private:
    class ConditionalTokenFilterFactoryAnonymousInnerClass
        : public ConditionalTokenFilterFactory
    {
      GET_CLASS_NAME(ConditionalTokenFilterFactoryAnonymousInnerClass)
    private:
      std::shared_ptr<Builder> outerInstance;

      std::function<bool(std::wstring *)> predicate;

    public:
      ConditionalTokenFilterFactoryAnonymousInnerClass(
          std::shared_ptr<Builder> outerInstance,
          std::shared_ptr<UnknownType> emptyMap,
          std::function<bool(std::wstring *)> &predicate);

    protected:
      std::shared_ptr<ConditionalTokenFilter>
      create(std::shared_ptr<TokenStream> input,
             std::function<TokenStream *(TokenStream *)> &inner) override;

    private:
      class ConditionalTokenFilterAnonymousInnerClass
          : public ConditionalTokenFilter
      {
        GET_CLASS_NAME(ConditionalTokenFilterAnonymousInnerClass)
      private:
        std::shared_ptr<ConditionalTokenFilterFactoryAnonymousInnerClass>
            outerInstance;

      public:
        ConditionalTokenFilterAnonymousInnerClass(
            std::shared_ptr<ConditionalTokenFilterFactoryAnonymousInnerClass>
                outerInstance,
            std::shared_ptr<TokenStream> input,
            std::function<TokenStream *(TokenStream *)> &inner);

        std::shared_ptr<CharTermAttribute> termAtt;

      protected:
        bool shouldFilter() override;

      protected:
        std::shared_ptr<ConditionalTokenFilterAnonymousInnerClass>
        shared_from_this()
        {
          return std::static_pointer_cast<
              ConditionalTokenFilterAnonymousInnerClass>(
              org.apache.lucene.analysis.miscellaneous
                  .ConditionalTokenFilter::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<ConditionalTokenFilterFactoryAnonymousInnerClass>
      shared_from_this()
      {
        return std::static_pointer_cast<
            ConditionalTokenFilterFactoryAnonymousInnerClass>(
            org.apache.lucene.analysis.miscellaneous
                .ConditionalTokenFilterFactory::shared_from_this());
      }
    };

    /** Builds the analyzer. */
  public:
    std::shared_ptr<CustomAnalyzer> build();

  private:
    std::unordered_map<std::wstring, std::wstring>
    applyDefaultParams(std::unordered_map<std::wstring, std::wstring> &map_obj);

    std::unordered_map<std::wstring, std::wstring>
    paramsToMap(std::deque<std::wstring> &params);

  public:
    template <typename T>
    T applyResourceLoader(T factory) ;
  };

  /**
   * Factory class for a {@link ConditionalTokenFilter}
   */
public:
  class ConditionBuilder : public std::enable_shared_from_this<ConditionBuilder>
  {
    GET_CLASS_NAME(ConditionBuilder)

  private:
    const std::deque<std::shared_ptr<TokenFilterFactory>> innerFilters =
        std::deque<std::shared_ptr<TokenFilterFactory>>();
    const std::shared_ptr<ConditionalTokenFilterFactory> factory;
    const std::shared_ptr<Builder> parent;

    ConditionBuilder(std::shared_ptr<ConditionalTokenFilterFactory> factory,
                     std::shared_ptr<Builder> parent);

    /** Adds the given token filter.
     * @param name is used to look up the factory with {@link
     * TokenFilterFactory#forName(std::wstring, Map)}. The deque of possible names can
     * be looked up with {@link TokenFilterFactory#availableTokenFilters()}.
     * @param params the map_obj of parameters to be passed to factory. The map_obj must
     * be modifiable.
     */
  public:
    virtual std::shared_ptr<ConditionBuilder>
    addTokenFilter(const std::wstring &name,
                   std::unordered_map<std::wstring, std::wstring>
                       &params) ;

    /** Adds the given token filter.
     * @param name is used to look up the factory with {@link
     * TokenFilterFactory#forName(std::wstring, Map)}. The deque of possible names can
     * be looked up with {@link TokenFilterFactory#availableTokenFilters()}.
     * @param params the map_obj of parameters to be passed to factory. The map_obj must
     * be modifiable.
     */
    virtual std::shared_ptr<ConditionBuilder>
    addTokenFilter(const std::wstring &name,
                   std::deque<std::wstring> &params) ;

    /** Adds the given token filter.
     * @param factory class that is used to create the token filter.
     * @param params the map_obj of parameters to be passed to factory. The map_obj must
     * be modifiable.
     */
    virtual std::shared_ptr<ConditionBuilder>
    addTokenFilter(std::type_info factory,
                   std::unordered_map<std::wstring, std::wstring>
                       &params) ;

    /** Adds the given token filter.
     * @param factory class that is used to create the token filter.
     * @param params the map_obj of parameters to be passed to factory. The map_obj must
     * be modifiable.
     */
    virtual std::shared_ptr<ConditionBuilder>
    addTokenFilter(std::type_info factory,
                   std::deque<std::wstring> &params) ;

    /**
     * Close the branch and return to the main analysis chain
     */
    virtual std::shared_ptr<Builder> endwhen() ;
  };

protected:
  std::shared_ptr<CustomAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<CustomAnalyzer>(
        org.apache.lucene.analysis.Analyzer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::custom
