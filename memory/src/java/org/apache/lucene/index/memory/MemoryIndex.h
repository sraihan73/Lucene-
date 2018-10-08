#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <map_obj>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index::memory
{
class Info;
}

namespace org::apache::lucene::util
{
class ByteBlockPool;
}
namespace org::apache::lucene::util
{
class IntBlockPool;
}
namespace org::apache::lucene::util
{
class SliceWriter;
}
namespace org::apache::lucene::util
{
class BytesRefArray;
}
namespace org::apache::lucene::util
{
class Counter;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::document
{
class FieldType;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::index
{
class IndexableField;
}
namespace org::apache::lucene::index
{
class IndexableFieldType;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::util
{
class BytesRefHash;
}
namespace org::apache::lucene::index::memory
{
class SliceByteStartArray;
}
namespace org::apache::lucene::index::memory
{
class BinaryDocValuesProducer;
}
namespace org::apache::lucene::index::memory
{
class NumericDocValuesProducer;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::index
{
class SortedNumericDocValues;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::index::memory
{
class MemoryFields;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
}
namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class TermState;
}
namespace org::apache::lucene::util
{
class SliceReader;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::index
{
class StoredFieldVisitor;
}
namespace org::apache::lucene::index
{
class LeafMetaData;
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
namespace org::apache::lucene::index::memory
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using FieldType = org::apache::lucene::document::FieldType;
using namespace org::apache::lucene::index;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Bits = org::apache::lucene::util::Bits;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefArray = org::apache::lucene::util::BytesRefArray;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using DirectBytesStartArray =
    org::apache::lucene::util::BytesRefHash::DirectBytesStartArray;
using Counter = org::apache::lucene::util::Counter;
using IntBlockPool = org::apache::lucene::util::IntBlockPool;
using SliceReader = org::apache::lucene::util::IntBlockPool::SliceReader;
using SliceWriter = org::apache::lucene::util::IntBlockPool::SliceWriter;

/**
 * High-performance single-document main memory Apache Lucene fulltext search
index.
 * <p>
 * <b>Overview</b>
 * <p>
 * This class is a replacement/substitute for a large subset of
 * {@link RAMDirectory} functionality. It is designed to
GET_CLASS_NAME(is)
 * enable maximum efficiency for on-the-fly matchmaking combining structured and
 * fuzzy fulltext search in realtime streaming applications such as Nux XQuery
based XML
 * message queues, publish-subscribe systems for Blogs/newsfeeds, text chat,
data acquisition and
 * distribution systems, application level routers, firewalls, classifiers, etc.
 * Rather than targeting fulltext search of infrequent queries over huge
persistent
 * data archives (historic search), this class targets fulltext search of huge
 * numbers of queries over comparatively small transient realtime data
(prospective
 * search).
 * For example as in
 * <pre class="prettyprint">
 * float score = search(std::wstring text, Query query)
 * </pre>
 * <p>
 * Each instance can hold at most one Lucene "document", with a document
containing
 * zero or more "fields", each field having a name and a fulltext value. The
 * fulltext value is tokenized (split and transformed) into zero or more index
terms
 * (aka words) on <code>addField()</code>, according to the policy implemented
by an
 * Analyzer. For example, Lucene analyzers can split on whitespace, normalize to
lower case
 * for case insensitivity, ignore common terms with little discriminatory value
such as "he", "in", "and" (stop
 * words), reduce the terms to their natural linguistic root form such as
"fishing"
 * being reduced to "fish" (stemming), resolve synonyms/inflexions/thesauri
 * (upon indexing and/or querying), etc. For details, see
 * <a target="_blank"
href="http://today.java.net/pub/a/today/2003/07/30/LuceneIntro.html">Lucene
Analyzer Intro</a>.
 * <p>
 * Arbitrary Lucene queries can be run against this class - see <a
target="_blank"
 *
href="{@docRoot}/../queryparser/org/apache/lucene/queryparser/classic/package-summary.html#package.description">
GET_CLASS_NAME(-)
 * Lucene Query Syntax</a>
 * as well as <a target="_blank"
 *
href="http://today.java.net/pub/a/today/2003/11/07/QueryParserRules.html">Query
Parser Rules</a>.
 * Note that a Lucene query selects on the field names and associated (indexed)
 * tokenized terms, not on the original fulltext(s) - the latter are not stored
 * but rather thrown away immediately after tokenization.
 * <p>
 * For some interesting background information on search technology, see Bob
Wyman's
 * <a target="_blank"
 *
href="http://bobwyman.pubsub.com/main/2005/05/mary_hodder_poi.html">Prospective
Search</a>,
 * Jim Gray's
 * <a target="_blank"
href="http://www.acmqueue.org/modules.php?name=Content&amp;pa=showpage&amp;pid=293&amp;page=4">
 * A Call to Arms - Custom subscriptions</a>, and Tim Bray's
 * <a target="_blank"
 * href="http://www.tbray.org/ongoing/When/200x/2003/07/30/OnSearchTOC">On
Search, the Series</a>.
 *
 * <p>
 * <b>Example Usage</b>
 * <br>
 * <pre class="prettyprint">
 * Analyzer analyzer = new SimpleAnalyzer(version);
 * MemoryIndex index = new MemoryIndex();
 * index.addField("content", "Readings about Salmons and other select Alaska
fishing Manuals", analyzer);
 * index.addField("author", "Tales of James", analyzer);
 * QueryParser parser = new QueryParser(version, "content", analyzer);
 * float score = index.search(parser.parse("+author:james +salmon~ +fish*
manual~"));
 * if (score &gt; 0.0f) {
 *     System.out.println("it's a match");
 * } else {
 *     System.out.println("no match found");
 * }
 * System.out.println("indexData=" + index.toString());
 * </pre>
 *
 * <p>
 * <b>Example XQuery Usage</b>
 *
 * <pre class="prettyprint">
 * (: An XQuery that finds all books authored by James that have something to do
with "salmon fishing manuals", sorted by relevance :)
 * declare namespace lucene = "java:nux::xom::pool::FullTextUtil";
 * declare variable $query := "+salmon~ +fish* manual~"; (: any arbitrary Lucene
query can go here :)
 *
 * for $book in /books/book[author="James" and lucene:match(abstract, $query)
&gt; 0.0]
 * let $score := lucene:match($book/abstract, $query)
 * order by $score descending
 * return $book
 * </pre>
 *
 * <p>
 * <b>Thread safety guarantees</b>
 * <p>
 * MemoryIndex is not normally thread-safe for adds or queries.  However,
queries
 * are thread-safe after {@code freeze()} has been called.
 *
 * <p>
 * <b>Performance Notes</b>
 * <p>
 * Internally there's a new data structure geared towards efficient indexing
 * and searching, plus the necessary support code to seamlessly plug into the
Lucene
 * framework.
 * <p>
 * This class performs very well for very small texts (e.g. 10 chars)
 * as well as for large texts (e.g. 10 MB) and everything in between.
 * Typically, it is about 10-100 times faster than <code>RAMDirectory</code>.
 * Note that <code>RAMDirectory</code> has particularly
 * large efficiency overheads for small to medium sized texts, both in time and
space.
 * Indexing a field with N tokens takes O(N) in the best case, and O(N logN) in
the worst
 * case. Memory consumption is probably larger than for
<code>RAMDirectory</code>.
 * <p>
 * Example throughput of many simple term queries over a single MemoryIndex:
 * ~500000 queries/sec on a MacBook Pro, jdk 1.5.0_06, server VM.
 * As always, your mileage may vary.
 * <p>
 * If you're curious about
 * the whereabouts of bottlenecks, run java 1.5 with the non-perturbing '-server
 * -agentlib:hprof=cpu=samples,depth=10' flags, then study the trace log and
 * correlate its hotspot trailer with its call stack headers (see <a
 * target="_blank"
 *
href="http://java.sun.com/developer/technicalArticles/Programming/HPROF.html">
 * hprof tracing </a>).
 *
 */
class MemoryIndex : public std::enable_shared_from_this<MemoryIndex>
{
  GET_CLASS_NAME(MemoryIndex)

private:
  static constexpr bool DEBUG = false;

  /** info for each field: Map&lt;std::wstring fieldName, Info field&gt; */
  const std::shared_ptr<SortedMap<std::wstring, std::shared_ptr<Info>>> fields =
      std::map_obj<std::wstring, std::shared_ptr<Info>>();

  const bool storeOffsets;
  const bool storePayloads;

  const std::shared_ptr<ByteBlockPool> byteBlockPool;
  const std::shared_ptr<IntBlockPool> intBlockPool;
  //  private final IntBlockPool.SliceReader postingsReader;
  const std::shared_ptr<IntBlockPool::SliceWriter> postingsWriter;
  const std::shared_ptr<BytesRefArray>
      payloadsBytesRefs; // non null only when storePayloads

  std::shared_ptr<Counter> bytesUsed;

  bool frozen = false;

  std::shared_ptr<Similarity> normSimilarity =
      IndexSearcher::getDefaultSimilarity();

  std::shared_ptr<FieldType> defaultFieldType = std::make_shared<FieldType>();

  /**
   * Constructs an empty instance that will not store offsets or payloads.
   */
public:
  MemoryIndex();

  /**
   * Constructs an empty instance that can optionally store the start and end
   * character offset of each token term in the text. This can be useful for
   * highlighting of hit locations with the Lucene highlighter package.  But
   * it will not store payloads; use another constructor for that.
   *
   * @param storeOffsets
   *            whether or not to store the start and end character offset of
   *            each token term in the text
   */
  MemoryIndex(bool storeOffsets);

  /**
   * Constructs an empty instance with the option of storing offsets and
   * payloads.
   *
   * @param storeOffsets store term offsets at each position
   * @param storePayloads store term payloads at each position
   */
  MemoryIndex(bool storeOffsets, bool storePayloads);

  /**
   * Expert: This constructor accepts an upper limit for the number of bytes
   * that should be reused if this instance is {@link #reset()}. The payload
   * storage, if used, is unaffected by maxReusuedBytes, however.
   * @param storeOffsets <code>true</code> if offsets should be stored
   * @param storePayloads <code>true</code> if payloads should be stored
   * @param maxReusedBytes the number of bytes that should remain in the
   * internal memory pools after {@link #reset()} is called
   */
  MemoryIndex(bool storeOffsets, bool storePayloads, int64_t maxReusedBytes);

  /**
   * Convenience method; Tokenizes the given field text and adds the resulting
   * terms to the index; Equivalent to adding an indexed non-keyword Lucene
   * {@link org.apache.lucene.document.Field} that is tokenized, not stored,
   * termVectorStored with positions (or termVectorStored with positions and
   * offsets),
   *
   * @param fieldName
   *            a name to be associated with the text
   * @param text
   *            the text to tokenize and index.
   * @param analyzer
   *            the analyzer to use for tokenization
   */
  virtual void addField(const std::wstring &fieldName, const std::wstring &text,
                        std::shared_ptr<Analyzer> analyzer);

  /**
   * Builds a MemoryIndex from a lucene {@link Document} using an analyzer
   *
   * @param document the document to index
   * @param analyzer the analyzer to use
   * @return a MemoryIndex
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public static MemoryIndex fromDocument(Iterable<?
  // extends IndexableField> document, org.apache.lucene.analysis.Analyzer
  // analyzer)
  static std::shared_ptr<MemoryIndex>
  fromDocument(std::deque<T1> document, std::shared_ptr<Analyzer> analyzer);

  /**
   * Builds a MemoryIndex from a lucene {@link Document} using an analyzer
   * @param document the document to index
   * @param analyzer the analyzer to use
   * @param storeOffsets <code>true</code> if offsets should be stored
   * @param storePayloads <code>true</code> if payloads should be stored
   * @return a MemoryIndex
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public static MemoryIndex fromDocument(Iterable<?
  // extends IndexableField> document, org.apache.lucene.analysis.Analyzer
  // analyzer, bool storeOffsets, bool storePayloads)
  static std::shared_ptr<MemoryIndex>
  fromDocument(std::deque<T1> document, std::shared_ptr<Analyzer> analyzer,
               bool storeOffsets, bool storePayloads);

  /**
   * Builds a MemoryIndex from a lucene {@link Document} using an analyzer
   * @param document the document to index
   * @param analyzer the analyzer to use
   * @param storeOffsets <code>true</code> if offsets should be stored
   * @param storePayloads <code>true</code> if payloads should be stored
   * @param maxReusedBytes the number of bytes that should remain in the
   * internal memory pools after {@link #reset()} is called
   * @return a MemoryIndex
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public static MemoryIndex fromDocument(Iterable<?
  // extends IndexableField> document, org.apache.lucene.analysis.Analyzer
  // analyzer, bool storeOffsets, bool storePayloads, long maxReusedBytes)
  static std::shared_ptr<MemoryIndex>
  fromDocument(std::deque<T1> document, std::shared_ptr<Analyzer> analyzer,
               bool storeOffsets, bool storePayloads, int64_t maxReusedBytes);

  /**
   * Convenience method; Creates and returns a token stream that generates a
   * token for each keyword in the given collection, "as is", without any
   * transforming text analysis. The resulting token stream can be fed into
   * {@link #addField(std::wstring, TokenStream)}, perhaps wrapped into another
   * {@link org.apache.lucene.analysis.TokenFilter}, as desired.
   *
   * @param keywords
   *            the keywords to generate tokens for
   * @return the corresponding token stream
   */
  template <typename T>
  std::shared_ptr<TokenStream>
  keywordTokenStream(std::shared_ptr<std::deque<T>> keywords);

private:
  class TokenStreamAnonymousInnerClass : public TokenStream
  {
    GET_CLASS_NAME(TokenStreamAnonymousInnerClass)
  private:
    std::shared_ptr<MemoryIndex> outerInstance;

    std::shared_ptr<std::deque<std::shared_ptr<T>>> keywords;

  public:
    TokenStreamAnonymousInnerClass(
        std::shared_ptr<MemoryIndex> outerInstance,
        std::shared_ptr<std::deque<std::shared_ptr<T>>> keywords);

  private:
    std::shared_ptr<Iterator<std::shared_ptr<T>>> iter;
    int start = 0;
    const std::shared_ptr<CharTermAttribute> termAtt;
    const std::shared_ptr<OffsetAttribute> offsetAtt;

  public:
    bool incrementToken() override;

  protected:
    std::shared_ptr<TokenStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamAnonymousInnerClass>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

  /**
   * Adds a lucene {@link IndexableField} to the MemoryIndex using the provided
   * analyzer. Also stores doc values based on {@link
   * IndexableFieldType#docValuesType()} if set.
   *
   * @param field the field to add
   * @param analyzer the analyzer to use for term analysis
   */
public:
  virtual void addField(std::shared_ptr<IndexableField> field,
                        std::shared_ptr<Analyzer> analyzer);

  /**
   * Iterates over the given token stream and adds the resulting terms to the
   * index; Equivalent to adding a tokenized, indexed, termVectorStored,
   * unstored, Lucene {@link org.apache.lucene.document.Field}. Finally closes
   * the token stream. Note that untokenized keywords can be added with this
   * method via
   * {@link #keywordTokenStream(std::deque)}, the Lucene
   * <code>KeywordTokenizer</code> or similar utilities.
   *
   * @param fieldName
   *            a name to be associated with the text
   * @param stream
   *            the token stream to retrieve tokens from.
   */
  virtual void addField(const std::wstring &fieldName,
                        std::shared_ptr<TokenStream> stream);

  /**
   * Iterates over the given token stream and adds the resulting terms to the
   * index; Equivalent to adding a tokenized, indexed, termVectorStored,
   * unstored, Lucene {@link org.apache.lucene.document.Field}. Finally closes
   * the token stream. Note that untokenized keywords can be added with this
   * method via
   * {@link #keywordTokenStream(std::deque)}, the Lucene
   * <code>KeywordTokenizer</code> or similar utilities.
   *
   * @param fieldName
   *            a name to be associated with the text
   * @param stream
   *            the token stream to retrieve tokens from.
   *
   * @param positionIncrementGap
   *            the position increment gap if fields with the same name are
   * added more than once
   *
   */
  virtual void addField(const std::wstring &fieldName,
                        std::shared_ptr<TokenStream> stream,
                        int positionIncrementGap);

  /**
   * Iterates over the given token stream and adds the resulting terms to the
   * index; Equivalent to adding a tokenized, indexed, termVectorStored,
   * unstored, Lucene {@link org.apache.lucene.document.Field}. Finally closes
   * the token stream. Note that untokenized keywords can be added with this
   * method via
   * {@link #keywordTokenStream(std::deque)}, the Lucene
   * <code>KeywordTokenizer</code> or similar utilities.
   *
   *
   * @param fieldName
   *            a name to be associated with the text
   * @param tokenStream
   *            the token stream to retrieve tokens from. It's guaranteed to be
   * closed no matter what.
   * @param positionIncrementGap
   *            the position increment gap if fields with the same name are
   * added more than once
   * @param offsetGap
   *            the offset gap if fields with the same name are added more than
   * once
   */
  virtual void addField(const std::wstring &fieldName,
                        std::shared_ptr<TokenStream> tokenStream,
                        int positionIncrementGap, int offsetGap);

private:
  std::shared_ptr<Info> getInfo(const std::wstring &fieldName,
                                std::shared_ptr<IndexableFieldType> fieldType);

  std::shared_ptr<FieldInfo>
  createFieldInfo(const std::wstring &fieldName, int ord,
                  std::shared_ptr<IndexableFieldType> fieldType);

  void storePointValues(std::shared_ptr<Info> info,
                        std::shared_ptr<BytesRef> pointValue);

  void storeDocValues(std::shared_ptr<Info> info, DocValuesType docValuesType,
                      std::any docValuesValue);

  void storeTerms(std::shared_ptr<Info> info,
                  std::shared_ptr<TokenStream> tokenStream,
                  int positionIncrementGap, int offsetGap);

  /**
   * Set the Similarity to be used for calculating field norms
   */
public:
  virtual void setSimilarity(std::shared_ptr<Similarity> similarity);

  /**
   * Creates and returns a searcher that can be used to execute arbitrary
   * Lucene queries and to collect the resulting query results as hits.
   *
   * @return a searcher
   */
  virtual std::shared_ptr<IndexSearcher> createSearcher();

  /**
   * Prepares the MemoryIndex for querying in a non-lazy way.
   * <p>
   * After calling this you can query the MemoryIndex from multiple threads, but
   * you cannot subsequently add new data.
   */
  virtual void freeze();

  /**
   * Convenience method that efficiently returns the relevance score by
   * matching this index against the given Lucene query expression.
   *
   * @param query
   *            an arbitrary Lucene query to run against this index
   * @return the relevance score of the matchmaking; A number in the range
   *         [0.0 .. 1.0], with 0.0 indicating no match. The higher the number
   *         the better the match.
   *
   */
  virtual float search(std::shared_ptr<Query> query);

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<MemoryIndex> outerInstance;

    std::deque<float> scores;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<MemoryIndex> outerInstance, std::deque<float> &scores);

  private:
    std::shared_ptr<Scorer> scorer;

  public:
    void collect(int doc)  override;

    void setScorer(std::shared_ptr<Scorer> scorer) override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

  /**
   * Returns a std::wstring representation of the index data for debugging purposes.
   *
   * @return the string representation
   * @lucene.experimental
   */
public:
  virtual std::wstring toStringDebug();

  /**
   * Index data structure for a field; contains the tokenized term texts and
   * their positions.
   */
private:
  class Info final : public std::enable_shared_from_this<Info>
  {
    GET_CLASS_NAME(Info)
  private:
    std::shared_ptr<MemoryIndex> outerInstance;

    std::shared_ptr<FieldInfo> fieldInfo;

    std::optional<int64_t> norm;

    /**
     * Term strings and their positions for this field: Map &lt;std::wstring
     * termText, ArrayIntList positions&gt;
     */
    std::shared_ptr<BytesRefHash>
        terms; // note unfortunate variable name class with Terms type

    std::shared_ptr<SliceByteStartArray> sliceArray;

    /** Terms sorted ascending by term text; computed on demand */
    std::deque<int> sortedTerms;

    /** Number of added tokens for this field */
    int numTokens = 0;

    /** Number of overlapping tokens for this field */
    int numOverlapTokens = 0;

    int64_t sumTotalTermFreq = 0;

    /** the last position encountered in this field for multi field support*/
    int lastPosition = 0;

    /** the last offset encountered in this field for multi field support*/
    int lastOffset = 0;

    std::shared_ptr<BinaryDocValuesProducer> binaryProducer;

    std::shared_ptr<NumericDocValuesProducer> numericProducer;

    bool preparedDocValuesAndPointValues = false;

    std::deque<std::shared_ptr<BytesRef>> pointValues;

    std::deque<char> minPackedValue;

    std::deque<char> maxPackedValue;

    int pointValuesCount = 0;

    Info(std::shared_ptr<MemoryIndex> outerInstance,
         std::shared_ptr<FieldInfo> fieldInfo,
         std::shared_ptr<ByteBlockPool> byteBlockPool);

  public:
    void freeze();

    /**
     * Sorts hashed terms into ascending order, reusing memory along the
     * way. Note that sorting is lazily delayed until required (often it's
     * not required at all). If a sorted view is required then hashing +
     * sort + binary search is still faster and smaller than TreeMap usage
     * (which would be an alternative and somewhat more elegant approach,
     * apart from more sophisticated Tries / prefix trees).
     */
    void sortTerms();

    void prepareDocValuesAndPointValues();

    std::shared_ptr<NumericDocValues> getNormDocValues();
  };

  ///////////////////////////////////////////////////////////////////////////////
  // Nested classes:
  ///////////////////////////////////////////////////////////////////////////////

private:
  class MemoryDocValuesIterator
      : public std::enable_shared_from_this<MemoryDocValuesIterator>
  {
    GET_CLASS_NAME(MemoryDocValuesIterator)

  public:
    int doc = -1;

    virtual int advance(int doc);

    virtual int nextDoc();

    virtual int docId();
  };

private:
  static std::shared_ptr<SortedNumericDocValues>
  numericDocValues(std::deque<int64_t> &values, int count);

private:
  class SortedNumericDocValuesAnonymousInnerClass
      : public SortedNumericDocValues
  {
    GET_CLASS_NAME(SortedNumericDocValuesAnonymousInnerClass)
  private:
    std::deque<int64_t> values;
    int count = 0;
    std::shared_ptr<org::apache::lucene::index::memory::MemoryIndex::
                        MemoryDocValuesIterator>
        it;

  public:
    SortedNumericDocValuesAnonymousInnerClass(
        std::deque<int64_t> &values, int count,
        std::shared_ptr<org::apache::lucene::index::memory::MemoryIndex::
                            MemoryDocValuesIterator>
            it);

    int ord = 0;

    int64_t nextValue()  override;

    int docValueCount() override;

    bool advanceExact(int target)  override;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SortedNumericDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedNumericDocValuesAnonymousInnerClass>(
          SortedNumericDocValues::shared_from_this());
    }
  };

private:
  static std::shared_ptr<NumericDocValues> numericDocValues(int64_t value);

private:
  class NumericDocValuesAnonymousInnerClass : public NumericDocValues
  {
    GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
  private:
    int64_t value = 0;
    std::shared_ptr<org::apache::lucene::index::memory::MemoryIndex::
                        MemoryDocValuesIterator>
        it;

  public:
    NumericDocValuesAnonymousInnerClass(
        int64_t value,
        std::shared_ptr<org::apache::lucene::index::memory::MemoryIndex::
                            MemoryDocValuesIterator>
            it);

    int64_t longValue()  override;

    bool advanceExact(int target)  override;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
          NumericDocValues::shared_from_this());
    }
  };

private:
  static std::shared_ptr<SortedDocValues>
  sortedDocValues(std::shared_ptr<BytesRef> value);

private:
  class SortedDocValuesAnonymousInnerClass : public SortedDocValues
  {
    GET_CLASS_NAME(SortedDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<BytesRef> value;
    std::shared_ptr<org::apache::lucene::index::memory::MemoryIndex::
                        MemoryDocValuesIterator>
        it;

  public:
    SortedDocValuesAnonymousInnerClass(
        std::shared_ptr<BytesRef> value,
        std::shared_ptr<org::apache::lucene::index::memory::MemoryIndex::
                            MemoryDocValuesIterator>
            it);

    int ordValue() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    int getValueCount() override;

    bool advanceExact(int target)  override;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SortedDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SortedDocValuesAnonymousInnerClass>(
          SortedDocValues::shared_from_this());
    }
  };

private:
  static std::shared_ptr<SortedSetDocValues>
  sortedSetDocValues(std::shared_ptr<BytesRefHash> values,
                     std::deque<int> &bytesIds);

private:
  class SortedSetDocValuesAnonymousInnerClass : public SortedSetDocValues
  {
    GET_CLASS_NAME(SortedSetDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<BytesRefHash> values;
    std::deque<int> bytesIds;
    std::shared_ptr<org::apache::lucene::index::memory::MemoryIndex::
                        MemoryDocValuesIterator>
        it;
    std::shared_ptr<BytesRef> scratch;

  public:
    SortedSetDocValuesAnonymousInnerClass(
        std::shared_ptr<BytesRefHash> values, std::deque<int> &bytesIds,
        std::shared_ptr<org::apache::lucene::index::memory::MemoryIndex::
                            MemoryDocValuesIterator>
            it,
        std::shared_ptr<BytesRef> scratch);

    int ord = 0;

    int64_t nextOrd()  override;

    std::shared_ptr<BytesRef>
    lookupOrd(int64_t ord)  override;

    int64_t getValueCount() override;

    bool advanceExact(int target)  override;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SortedSetDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SortedSetDocValuesAnonymousInnerClass>(
          SortedSetDocValues::shared_from_this());
    }
  };

private:
  class BinaryDocValuesProducer final
      : public std::enable_shared_from_this<BinaryDocValuesProducer>
  {
    GET_CLASS_NAME(BinaryDocValuesProducer)

  public:
    std::shared_ptr<BytesRefHash> dvBytesValuesSet;
    std::deque<int> bytesIds;

  private:
    void prepareForUsage();
  };

private:
  class NumericDocValuesProducer final
      : public std::enable_shared_from_this<NumericDocValuesProducer>
  {
    GET_CLASS_NAME(NumericDocValuesProducer)

  public:
    std::deque<int64_t> dvLongValues;
    int count = 0;

  private:
    void prepareForUsage();
  };

  /**
   * Search support for Lucene framework integration; implements all methods
   * required by the Lucene IndexReader contracts.
   */
private:
  class MemoryIndexReader final : public LeafReader
  {
    GET_CLASS_NAME(MemoryIndexReader)
  private:
    std::shared_ptr<MemoryIndex> outerInstance;

    const std::shared_ptr<MemoryFields> memoryFields =
        std::make_shared<MemoryFields>(outerInstance->fields);

    MemoryIndexReader(
        std::shared_ptr<MemoryIndex>
            outerInstance); // avoid as much superclass baggage as possible

    std::shared_ptr<Info>
    getInfoForExpectedDocValuesType(const std::wstring &fieldName,
                                    DocValuesType expectedType);

  public:
    std::shared_ptr<Bits> getLiveDocs() override;

    std::shared_ptr<FieldInfos> getFieldInfos() override;

    std::shared_ptr<NumericDocValues>
    getNumericDocValues(const std::wstring &field)  override;

    std::shared_ptr<BinaryDocValues>
    getBinaryDocValues(const std::wstring &field) override;

    std::shared_ptr<SortedDocValues>
    getSortedDocValues(const std::wstring &field) override;

  private:
    std::shared_ptr<SortedDocValues>
    getSortedDocValues(const std::wstring &field, DocValuesType docValuesType);

  public:
    std::shared_ptr<SortedNumericDocValues>
    getSortedNumericDocValues(const std::wstring &field) override;

    std::shared_ptr<SortedSetDocValues>
    getSortedSetDocValues(const std::wstring &field) override;

    std::shared_ptr<PointValues>
    getPointValues(const std::wstring &fieldName) override;

    void checkIntegrity()  override;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

  private:
    class MemoryFields : public Fields
    {
      GET_CLASS_NAME(MemoryFields)
    private:
      std::shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance;

      const std::unordered_map<std::wstring, std::shared_ptr<Info>> fields;

    public:
      MemoryFields(
          std::shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance,
          std::unordered_map<std::wstring, std::shared_ptr<Info>> &fields);

      std::shared_ptr<Iterator<std::wstring>> iterator() override;

      std::shared_ptr<Terms> terms(const std::wstring &field) override;

    private:
      class TermsAnonymousInnerClass : public Terms
      {
        GET_CLASS_NAME(TermsAnonymousInnerClass)
      private:
        std::shared_ptr<MemoryFields> outerInstance;

        std::shared_ptr<org::apache::lucene::index::memory::MemoryIndex::Info>
            info;

      public:
        TermsAnonymousInnerClass(
            std::shared_ptr<MemoryFields> outerInstance,
            std::shared_ptr<
                org::apache::lucene::index::memory::MemoryIndex::Info>
                info);

        std::shared_ptr<TermsEnum> iterator() override;

        int64_t size() override;

        int64_t getSumTotalTermFreq() override;

        int64_t getSumDocFreq() override;

        int getDocCount() override;

        bool hasFreqs() override;

        bool hasOffsets() override;

        bool hasPositions() override;

        bool hasPayloads() override;

      protected:
        std::shared_ptr<TermsAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<TermsAnonymousInnerClass>(
              Terms::shared_from_this());
        }
      };

    public:
      int size() override;

    protected:
      std::shared_ptr<MemoryFields> shared_from_this()
      {
        return std::static_pointer_cast<MemoryFields>(
            Fields::shared_from_this());
      }
    };

  private:
    class MemoryTermsEnum : public TermsEnum
    {
      GET_CLASS_NAME(MemoryTermsEnum)
    private:
      std::shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance;

      const std::shared_ptr<Info> info;
      const std::shared_ptr<BytesRef> br = std::make_shared<BytesRef>();

    public:
      int termUpto = -1;

      MemoryTermsEnum(
          std::shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance,
          std::shared_ptr<Info> info);

    private:
      int binarySearch(std::shared_ptr<BytesRef> b,
                       std::shared_ptr<BytesRef> bytesRef, int low, int high,
                       std::shared_ptr<BytesRefHash> hash,
                       std::deque<int> &ords);

    public:
      bool seekExact(std::shared_ptr<BytesRef> text) override;

      SeekStatus seekCeil(std::shared_ptr<BytesRef> text) override;

      void seekExact(int64_t ord) override;

      std::shared_ptr<BytesRef> next() override;

      std::shared_ptr<BytesRef> term() override;

      int64_t ord() override;

      int docFreq() override;

      int64_t totalTermFreq() override;

      std::shared_ptr<PostingsEnum>
      postings(std::shared_ptr<PostingsEnum> reuse, int flags) override;

      void
      seekExact(std::shared_ptr<BytesRef> term,
                std::shared_ptr<TermState> state)  override;

      std::shared_ptr<TermState> termState()  override;

    protected:
      std::shared_ptr<MemoryTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<MemoryTermsEnum>(
            TermsEnum::shared_from_this());
      }
    };

  private:
    class MemoryPostingsEnum : public PostingsEnum
    {
      GET_CLASS_NAME(MemoryPostingsEnum)
    private:
      std::shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance;

      const std::shared_ptr<IntBlockPool::SliceReader> sliceReader;
      int posUpto = 0; // for assert
      bool hasNext = false;
      int doc = -1;
      // C++ NOTE: Fields cannot have the same name as methods:
      int freq_ = 0;
      int pos = 0;
      // C++ NOTE: Fields cannot have the same name as methods:
      int startOffset_ = 0;
      // C++ NOTE: Fields cannot have the same name as methods:
      int endOffset_ = 0;
      int payloadIndex = 0;
      const std::shared_ptr<BytesRefBuilder>
          payloadBuilder; // only non-null when storePayloads

    public:
      MemoryPostingsEnum(
          std::shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance);

      virtual std::shared_ptr<PostingsEnum> reset(int start, int end, int freq);

      int docID() override;

      int nextDoc() override;

      int advance(int target)  override;

      int freq()  override;

      int nextPosition() override;

      int startOffset() override;

      int endOffset() override;

      std::shared_ptr<BytesRef> getPayload() override;

      int64_t cost() override;

    protected:
      std::shared_ptr<MemoryPostingsEnum> shared_from_this()
      {
        return std::static_pointer_cast<MemoryPostingsEnum>(
            PostingsEnum::shared_from_this());
      }
    };

  private:
    class MemoryIndexPointValues : public PointValues
    {
      GET_CLASS_NAME(MemoryIndexPointValues)
    private:
      std::shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance;

    public:
      const std::shared_ptr<Info> info;

      MemoryIndexPointValues(
          std::shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance,
          std::shared_ptr<Info> info);

      void intersect(std::shared_ptr<IntersectVisitor> visitor) throw(
          IOException) override;

      int64_t
      estimatePointCount(std::shared_ptr<IntersectVisitor> visitor) override;

      std::deque<char> getMinPackedValue()  override;

      std::deque<char> getMaxPackedValue()  override;

      int getNumDimensions()  override;

      int getBytesPerDimension()  override;

      int64_t size() override;

      int getDocCount() override;

    protected:
      std::shared_ptr<MemoryIndexPointValues> shared_from_this()
      {
        return std::static_pointer_cast<MemoryIndexPointValues>(
            PointValues::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Fields> getTermVectors(int docID) override;

    int numDocs() override;

    int maxDoc() override;

    void document(int docID,
                  std::shared_ptr<StoredFieldVisitor> visitor) override;

  protected:
    void doClose() override;

  public:
    std::shared_ptr<NumericDocValues>
    getNormValues(const std::wstring &field) override;

    std::shared_ptr<LeafMetaData> getMetaData() override;

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<MemoryIndexReader> shared_from_this()
    {
      return std::static_pointer_cast<MemoryIndexReader>(
          LeafReader::shared_from_this());
    }
  };

  /**
   * Resets the {@link MemoryIndex} to its initial state and recycles all
   * internal buffers.
   */
public:
  virtual void reset();

private:
  class SliceByteStartArray final : public BytesRefHash::DirectBytesStartArray
  {
    GET_CLASS_NAME(SliceByteStartArray)
  public:
    std::deque<int> start; // the start offset in the IntBlockPool per term
    std::deque<int> end;   // the end pointer in the IntBlockPool for the
                            // postings slice per term
    std::deque<int> freq;  // the term frequency

    SliceByteStartArray(int initSize);

    std::deque<int> init() override;

    std::deque<int> grow() override;

    std::deque<int> clear() override;

  protected:
    std::shared_ptr<SliceByteStartArray> shared_from_this()
    {
      return std::static_pointer_cast<SliceByteStartArray>(
          org.apache.lucene.util.BytesRefHash
              .DirectBytesStartArray::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::index::memory
