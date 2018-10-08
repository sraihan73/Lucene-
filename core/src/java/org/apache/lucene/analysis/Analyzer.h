#pragma once
#include "TokenStream.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
//#include  "core/src/java/org/apache/lucene/analysis/ReuseStrategy.h"

#include  "core/src/java/org/apache/lucene/util/Version.h"
//#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include  "core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include  "core/src/java/org/apache/lucene/analysis/ReusableStringReader.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"

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
namespace org::apache::lucene::analysis
{

using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using BytesRef = org::apache::lucene::util::BytesRef;
using Version = org::apache::lucene::util::Version;

/**
 * An Analyzer builds TokenStreams, which analyze text.  It thus represents a
 * policy for extracting index terms from text.
 * <p>
 * In order to define what analysis is done, subclasses must define their
 * {@link TokenStreamComponents TokenStreamComponents} in {@link
#createComponents(std::wstring)}. GET_CLASS_NAME(es)
 * The components are then reused in each call to {@link #tokenStream(std::wstring,
Reader)}.
 * <p>
 * Simple example:
 * <pre class="prettyprint">
 * Analyzer analyzer = new Analyzer() {
GET_CLASS_NAME(="prettyprint">)
 *  {@literal @Override}
 *   protected TokenStreamComponents createComponents(std::wstring fieldName) {
 *     Tokenizer source = new FooTokenizer(reader);
 *     TokenStream filter = new FooFilter(source);
 *     filter = new BarFilter(filter);
 *     return new TokenStreamComponents(source, filter);
 *   }
 *   {@literal @Override}
 *   protected TokenStream normalize(TokenStream in) {
 *     // Assuming FooFilter is about normalization and BarFilter is about
 *     // stemming, only FooFilter should be applied
 *     return new FooFilter(in);
 *   }
 * };
 * </pre>
 * For more examples, see the {@link org.apache.lucene.analysis Analysis package
documentation}.
 * <p>
 * For some concrete implementations bundled with Lucene, look in the analysis
modules:
 * <ul>
 *   <li><a
href="{@docRoot}/../analyzers-common/overview-summary.html">Common</a>:
 *       Analyzers for indexing content in different languages and domains.
 *   <li><a href="{@docRoot}/../analyzers-icu/overview-summary.html">ICU</a>:
 *       Exposes functionality from ICU to Apache Lucene.
 *   <li><a
href="{@docRoot}/../analyzers-kuromoji/overview-summary.html">Kuromoji</a>:
 *       Morphological analyzer for Japanese text.
 *   <li><a
href="{@docRoot}/../analyzers-morfologik/overview-summary.html">Morfologik</a>:
 *       Dictionary-driven lemmatization for the Polish language.
 *   <li><a
href="{@docRoot}/../analyzers-phonetic/overview-summary.html">Phonetic</a>:
 *       Analysis for indexing phonetic signatures (for sounds-alike search).
 *   <li><a href="{@docRoot}/../analyzers-smartcn/overview-summary.html">Smart
Chinese</a>:
 *       Analyzer for Simplified Chinese, which indexes words.
 *   <li><a
href="{@docRoot}/../analyzers-stempel/overview-summary.html">Stempel</a>:
 *       Algorithmic Stemmer for the Polish Language.
 *   <li><a href="{@docRoot}/../analyzers-uima/overview-summary.html">UIMA</a>:
 *       Analysis integration with Apache UIMA.
 * </ul>
 */
class Analyzer : public std::enable_shared_from_this<Analyzer>
{
  GET_CLASS_NAME(Analyzer)

private:
  const std::shared_ptr<ReuseStrategy> reuseStrategy;
  std::shared_ptr<Version> version = Version::LATEST;

  // non final as it gets nulled if closed; pkg private for access by
  // ReuseStrategy's final helper methods:
public:
  std::shared_ptr<std::any> storedValue = std::make_shared<std::any>();

  /**
   * Create a new Analyzer, reusing the same set of components per-thread
   * across calls to {@link #tokenStream(std::wstring, Reader)}.
   */
  Analyzer();

  /**
   * Expert: create a new Analyzer with a custom {@link ReuseStrategy}.
   * <p>
   * NOTE: if you just want to reuse on a per-field basis, it's easier to
   * use a subclass of {@link AnalyzerWrapper} such as
   * <a
href="{@docRoot}/../analyzers-common/org/apache/lucene/analysis/miscellaneous/PerFieldAnalyzerWrapper.html">
GET_CLASS_NAME(of)
   * PerFieldAnalyerWrapper</a> instead.
   */
  Analyzer(std::shared_ptr<ReuseStrategy> reuseStrategy);

  /**
   * Creates a new {@link TokenStreamComponents} instance for this analyzer.
   *
   * @param fieldName
   *          the name of the fields content passed to the
   *          {@link TokenStreamComponents} sink as a reader

   * @return the {@link TokenStreamComponents} for this analyzer.
   */
protected:
  virtual std::shared_ptr<TokenStreamComponents>
  createComponents(const std::wstring &fieldName) = 0;

  /**
   * Wrap the given {@link TokenStream} in order to apply normalization filters.
   * The default implementation returns the {@link TokenStream} as-is. This is
   * used by {@link #normalize(std::wstring, std::wstring)}.
   */
  virtual std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName, std::shared_ptr<TokenStream> in_);

  /**
   * Returns a TokenStream suitable for <code>fieldName</code>, tokenizing
   * the contents of <code>reader</code>.
   * <p>
   * This method uses {@link #createComponents(std::wstring)} to obtain an
   * instance of {@link TokenStreamComponents}. It returns the sink of the
   * components and stores the components internally. Subsequent calls to this
   * method will reuse the previously stored components after resetting them
   * through {@link TokenStreamComponents#setReader(Reader)}.
   * <p>
   * <b>NOTE:</b> After calling this method, the consumer must follow the
   * workflow described in {@link TokenStream} to properly consume its contents.
   * See the {@link org.apache.lucene.analysis Analysis package documentation}
   * for some examples demonstrating this.
   *
   * <b>NOTE:</b> If your data is available as a {@code std::wstring}, use
   * {@link #tokenStream(std::wstring, std::wstring)} which reuses a {@code
   * StringReader}-like instance internally.
   *
   * @param fieldName the name of the field the created TokenStream is used for
   * @param reader the reader the streams source reads from
   * @return TokenStream for iterating the analyzed content of
   * <code>reader</code>
   * @throws AlreadyClosedException if the Analyzer is closed.
   * @see #tokenStream(std::wstring, std::wstring)
   */
public:
  std::shared_ptr<TokenStream> tokenStream(const std::wstring &fieldName,
                                           std::shared_ptr<Reader> reader);

  /**
   * Returns a TokenStream suitable for <code>fieldName</code>, tokenizing
   * the contents of <code>text</code>.
   * <p>
   * This method uses {@link #createComponents(std::wstring)} to obtain an
   * instance of {@link TokenStreamComponents}. It returns the sink of the
   * components and stores the components internally. Subsequent calls to this
   * method will reuse the previously stored components after resetting them
   * through {@link TokenStreamComponents#setReader(Reader)}.
   * <p>
   * <b>NOTE:</b> After calling this method, the consumer must follow the
   * workflow described in {@link TokenStream} to properly consume its contents.
   * See the {@link org.apache.lucene.analysis Analysis package documentation}
   * for some examples demonstrating this.
   *
   * @param fieldName the name of the field the created TokenStream is used for
   * @param text the std::wstring the streams source reads from
   * @return TokenStream for iterating the analyzed content of
   * <code>reader</code>
   * @throws AlreadyClosedException if the Analyzer is closed.
   * @see #tokenStream(std::wstring, Reader)
   */
  std::shared_ptr<TokenStream> tokenStream(const std::wstring &fieldName,
                                           const std::wstring &text);

  /**
   * Normalize a string down to the representation that it would have in the
   * index.
   * <p>
   * This is typically used by query parsers in order to generate a query on
   * a given term, without tokenizing or stemming, which are undesirable if
   * the string to analyze is a partial word (eg. in case of a wildcard or
   * fuzzy query).
   * <p>
   * This method uses {@link #initReaderForNormalization(std::wstring, Reader)} in
   * order to apply necessary character-level normalization and then
   * {@link #normalize(std::wstring, TokenStream)} in order to apply the normalizing
   * token filters.
   */
  std::shared_ptr<BytesRef> normalize(const std::wstring &fieldName,
                                      const std::wstring &text);

  /**
   * Override this if you want to add a CharFilter chain.
   * <p>
   * The default implementation returns <code>reader</code>
   * unchanged.
   *
   * @param fieldName IndexableField name being indexed
   * @param reader original Reader
   * @return reader, optionally decorated with CharFilter(s)
   */
protected:
  virtual std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                             std::shared_ptr<Reader> reader);

  /** Wrap the given {@link Reader} with {@link CharFilter}s that make sense
   *  for normalization. This is typically a subset of the {@link CharFilter}s
   *  that are applied in {@link #initReader(std::wstring, Reader)}. This is used by
   *  {@link #normalize(std::wstring, std::wstring)}. */
  virtual std::shared_ptr<Reader>
  initReaderForNormalization(const std::wstring &fieldName,
                             std::shared_ptr<Reader> reader);

  /** Return the {@link AttributeFactory} to be used for
   *  {@link #tokenStream analysis} and
   *  {@link #normalize(std::wstring, std::wstring) normalization} on the given
   *  {@code FieldName}. The default implementation returns
   *  {@link TokenStream#DEFAULT_TOKEN_ATTRIBUTE_FACTORY}. */
  virtual std::shared_ptr<AttributeFactory>
  attributeFactory(const std::wstring &fieldName);

  /**
   * Invoked before indexing a IndexableField instance if
   * terms have already been added to that field.  This allows custom
   * analyzers to place an automatic position increment gap between
   * IndexbleField instances using the same field name.  The default value
   * position increment gap is 0.  With a 0 position increment gap and
   * the typical default token position increment of 1, all terms in a field,
   * including across IndexableField instances, are in successive positions,
   * allowing exact PhraseQuery matches, for instance, across IndexableField
   * instance boundaries.
   *
   * @param fieldName IndexableField name being indexed.
   * @return position increment gap, added to the next token emitted from {@link
   * #tokenStream(std::wstring,Reader)}. This value must be {@code >= 0}.
   */
public:
  virtual int getPositionIncrementGap(const std::wstring &fieldName);

  /**
   * Just like {@link #getPositionIncrementGap}, except for
   * Token offsets instead.  By default this returns 1.
   * This method is only called if the field
   * produced at least one token for indexing.
   *
   * @param fieldName the field just indexed
   * @return offset gap, added to the next token emitted from {@link
   * #tokenStream(std::wstring,Reader)}. This value must be {@code >= 0}.
   */
  virtual int getOffsetGap(const std::wstring &fieldName);

  /**
   * Returns the used {@link ReuseStrategy}.
   */
  std::shared_ptr<ReuseStrategy> getReuseStrategy();

  /**
   * Set the version of Lucene this analyzer should mimic the behavior for for
   * analysis.
   */
  virtual void setVersion(std::shared_ptr<Version> v);

  /**
   * Return the version of Lucene this analyzer will mimic the behavior of for
   * analysis.
   */
  virtual std::shared_ptr<Version> getVersion();

  /** Frees persistent resources used by this Analyzer */
  virtual ~Analyzer();

  /**
   * This class encapsulates the outer components of a token stream. It provides
   * access to the source ({@link Tokenizer}) and the outer end (sink), an
GET_CLASS_NAME(encapsulates)
   * instance of {@link TokenFilter} which also serves as the
   * {@link TokenStream} returned by
   * {@link Analyzer#tokenStream(std::wstring, Reader)}.
   */
public:
  class TokenStreamComponents
      : public std::enable_shared_from_this<TokenStreamComponents>
  {
    GET_CLASS_NAME(TokenStreamComponents)
    /**
     * Original source of the tokens.
     */
  protected:
    const std::shared_ptr<Tokenizer> source;
    /**
     * Sink tokenstream, such as the outer tokenfilter decorating
     * the chain. This can be the source if there are no filters.
     */
    const std::shared_ptr<TokenStream> sink;

    /** Internal cache only used by {@link Analyzer#tokenStream(std::wstring,
     * std::wstring)}. */
  public:
    std::shared_ptr<ReusableStringReader> reusableStringReader;

    /**
     * Creates a new {@link TokenStreamComponents} instance.
     *
     * @param source
     *          the analyzer's tokenizer
     * @param result
     *          the analyzer's resulting token stream
     */
    TokenStreamComponents(std::shared_ptr<Tokenizer> source,
                          std::shared_ptr<TokenStream> result);

    /**
     * Creates a new {@link TokenStreamComponents} instance.
     *
     * @param source
     *          the analyzer's tokenizer
     */
    TokenStreamComponents(std::shared_ptr<Tokenizer> source);

    /**
     * Resets the encapsulated components with the given reader. If the
     * components cannot be reset, an Exception should be thrown.
     *
     * @param reader
     *          a reader to reset the source component
     */
  protected:
    virtual void setReader(std::shared_ptr<Reader> reader);

    /**
     * Returns the sink {@link TokenStream}
     *
     * @return the sink {@link TokenStream}
     */
  public:
    virtual std::shared_ptr<TokenStream> getTokenStream();

    /**
     * Returns the component's {@link Tokenizer}
     *
     * @return Component's {@link Tokenizer}
     */
    virtual std::shared_ptr<Tokenizer> getTokenizer();
  };

  /**
   * Strategy defining how TokenStreamComponents are reused per call to
   * {@link Analyzer#tokenStream(std::wstring, java.io.Reader)}.
   */
public:
  class ReuseStrategy : public std::enable_shared_from_this<ReuseStrategy>
  {
    GET_CLASS_NAME(ReuseStrategy)

    /** Sole constructor. (For invocation by subclass constructors, typically
     * implicit.) */
  public:
    ReuseStrategy();

    /**
     * Gets the reusable TokenStreamComponents for the field with the given
     * name.
     *
     * @param analyzer Analyzer from which to get the reused components. Use
     *        {@link #getStoredValue(Analyzer)} and {@link
     * #setStoredValue(Analyzer, Object)} to access the data on the Analyzer.
     * @param fieldName Name of the field whose reusable TokenStreamComponents
     *        are to be retrieved
     * @return Reusable TokenStreamComponents for the field, or {@code null}
     *         if there was no previous components for the field
     */
    virtual std::shared_ptr<TokenStreamComponents>
    getReusableComponents(std::shared_ptr<Analyzer> analyzer,
                          const std::wstring &fieldName) = 0;

    /**
     * Stores the given TokenStreamComponents as the reusable components for the
     * field with the give name.
     *
     * @param fieldName Name of the field whose TokenStreamComponents are being
     * set
     * @param components TokenStreamComponents which are to be reused for the
     * field
     */
    virtual void setReusableComponents(
        std::shared_ptr<Analyzer> analyzer, const std::wstring &fieldName,
        std::shared_ptr<TokenStreamComponents> components) = 0;

    /**
     * Returns the currently stored value.
     *
     * @return Currently stored value or {@code null} if no value is stored
     * @throws AlreadyClosedException if the Analyzer is closed.
     */
  protected:
    std::any getStoredValue(std::shared_ptr<Analyzer> analyzer);

    /**
     * Sets the stored value.
     *
     * @param storedValue Value to store
     * @throws AlreadyClosedException if the Analyzer is closed.
     */
    void setStoredValue(std::shared_ptr<Analyzer> analyzer,
                        std::any storedValue);
  };

  /**
   * A predefined {@link ReuseStrategy}  that reuses the same components for
   * every field.
   */
public:
  static const std::shared_ptr<ReuseStrategy> GLOBAL_REUSE_STRATEGY;

private:
  class ReuseStrategyAnonymousInnerClass : public ReuseStrategy
  {
    GET_CLASS_NAME(ReuseStrategyAnonymousInnerClass)
  public:
    ReuseStrategyAnonymousInnerClass();

    std::shared_ptr<TokenStreamComponents>
    getReusableComponents(std::shared_ptr<Analyzer> analyzer,
                          const std::wstring &fieldName) override;

    void setReusableComponents(
        std::shared_ptr<Analyzer> analyzer, const std::wstring &fieldName,
        std::shared_ptr<TokenStreamComponents> components) override;

  protected:
    std::shared_ptr<ReuseStrategyAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ReuseStrategyAnonymousInnerClass>(
          ReuseStrategy::shared_from_this());
    }
  };

  /**
   * A predefined {@link ReuseStrategy} that reuses components per-field by
   * maintaining a Map of TokenStreamComponent per field name.
   */
public:
  static const std::shared_ptr<ReuseStrategy> PER_FIELD_REUSE_STRATEGY;

private:
  class ReuseStrategyAnonymousInnerClass2 : public ReuseStrategy
  {
    GET_CLASS_NAME(ReuseStrategyAnonymousInnerClass2)
  public:
    ReuseStrategyAnonymousInnerClass2();

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public
    // TokenStreamComponents getReusableComponents(Analyzer analyzer, std::wstring
    // fieldName)
    std::shared_ptr<TokenStreamComponents>
    getReusableComponents(std::shared_ptr<Analyzer> analyzer,
                          const std::wstring &fieldName) override;

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public void
    // setReusableComponents(Analyzer analyzer, std::wstring fieldName,
    // TokenStreamComponents components)
    void setReusableComponents(
        std::shared_ptr<Analyzer> analyzer, const std::wstring &fieldName,
        std::shared_ptr<TokenStreamComponents> components) override;

  protected:
    std::shared_ptr<ReuseStrategyAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ReuseStrategyAnonymousInnerClass2>(
          ReuseStrategy::shared_from_this());
    }
  };

private:
  class StringTokenStream final : public TokenStream
  {
    GET_CLASS_NAME(StringTokenStream)

  private:
    const std::wstring value;
    const int length;
    bool used = true;
    const std::shared_ptr<CharTermAttribute> termAttribute =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<OffsetAttribute> offsetAttribute =
        addAttribute(OffsetAttribute::typeid);

  public:
    StringTokenStream(std::shared_ptr<AttributeFactory> attributeFactory,
                      const std::wstring &value, int length);

    void reset() override;

    bool incrementToken() override;

    void end()  override;

  protected:
    std::shared_ptr<StringTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<StringTokenStream>(
          TokenStream::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/analysis/
