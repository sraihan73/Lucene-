#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/PhraseQuery.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
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
namespace org::apache::lucene::search::vectorhighlight
{

using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class AbstractTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(AbstractTestCase)

protected:
  static const std::wstring F;
  static const std::wstring F1;
  static const std::wstring F2;
  std::shared_ptr<Directory> dir;
  std::shared_ptr<Analyzer> analyzerW;
  std::shared_ptr<Analyzer> analyzerB;
  std::shared_ptr<Analyzer> analyzerK;
  std::shared_ptr<IndexReader> reader;

  static std::deque<std::wstring> const shortMVValues;

  static std::deque<std::wstring> const longMVValues;

  // test data for LUCENE-1448 bug
  static std::deque<std::wstring> const biMVValues;

  static std::deque<std::wstring> const strMVValues;

public:
  void setUp()  override;

  void tearDown()  override;

protected:
  virtual std::shared_ptr<Query> tq(const std::wstring &text);

  virtual std::shared_ptr<Query> tq(float boost, const std::wstring &text);

  virtual std::shared_ptr<Query> tq(const std::wstring &field,
                                    const std::wstring &text);

  virtual std::shared_ptr<Query> tq(float boost, const std::wstring &field,
                                    const std::wstring &text);

  virtual std::shared_ptr<Query> pqF(std::deque<std::wstring> &texts);

  virtual std::shared_ptr<Query> pqF(float boost,
                                     std::deque<std::wstring> &texts);

  virtual std::shared_ptr<Query> pqF(float boost, int slop,
                                     std::deque<std::wstring> &texts);

  virtual std::shared_ptr<Query> pq(const std::wstring &field,
                                    std::deque<std::wstring> &texts);

  virtual std::shared_ptr<Query> pq(float boost, const std::wstring &field,
                                    std::deque<std::wstring> &texts);

  virtual std::shared_ptr<Query> pq(float boost, int slop,
                                    const std::wstring &field,
                                    std::deque<std::wstring> &texts);

  virtual std::shared_ptr<Query> dmq(std::deque<Query> &queries);

  virtual std::shared_ptr<Query> dmq(float tieBreakerMultiplier,
                                     std::deque<Query> &queries);

  virtual void assertCollectionQueries(
      std::shared_ptr<std::deque<std::shared_ptr<Query>>> actual,
      std::deque<Query> &expected);

  virtual std::deque<std::shared_ptr<BytesRef>>
  analyze(const std::wstring &text, const std::wstring &field,
          std::shared_ptr<Analyzer> analyzer) ;

  virtual std::shared_ptr<PhraseQuery>
  toPhraseQuery(std::deque<std::shared_ptr<BytesRef>> &bytesRefs,
                const std::wstring &field);

public:
  class BigramAnalyzer final : public Analyzer
  {
    GET_CLASS_NAME(BigramAnalyzer)
  public:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<BigramAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<BigramAnalyzer>(
          Analyzer::shared_from_this());
    }
  };

public:
  class BasicNGramTokenizer final : public Tokenizer
  {
    GET_CLASS_NAME(BasicNGramTokenizer)

  public:
    static constexpr int DEFAULT_N_SIZE = 2;
    static const std::wstring DEFAULT_DELIMITERS;

  private:
    const int n;
    const std::wstring delimiters;
    int startTerm = 0;
    int lenTerm = 0;
    int startOffset = 0;
    int nextStartOffset = 0;
    int ch = 0;
    std::wstring snippet;
    std::shared_ptr<StringBuilder> snippetBuffer;
    static constexpr int BUFFER_SIZE = 4096;
    std::deque<wchar_t> charBuffer;
    int charBufferIndex = 0;
    int charBufferLen = 0;

  public:
    BasicNGramTokenizer();

    BasicNGramTokenizer(int n);

    BasicNGramTokenizer(const std::wstring &delimiters);

    BasicNGramTokenizer(int n, const std::wstring &delimiters);

    std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    std::shared_ptr<OffsetAttribute> offsetAtt =
        addAttribute(OffsetAttribute::typeid);
    bool incrementToken()  override;

  private:
    int getFinalOffset();

  public:
    void end()  override final;

  protected:
    bool getNextPartialSnippet() ;

    bool getNextSnippet() ;

    int readCharFromBuffer() ;

    bool isDelimiter(int c);

  public:
    void reset()  override;

  protected:
    std::shared_ptr<BasicNGramTokenizer> shared_from_this()
    {
      return std::static_pointer_cast<BasicNGramTokenizer>(
          Tokenizer::shared_from_this());
    }
  };

protected:
  virtual void
  make1d1fIndex(const std::wstring &value) ;

  virtual void
  make1d1fIndexB(const std::wstring &value) ;

  virtual void
  make1dmfIndex(std::deque<std::wstring> &values) ;

  virtual void
  make1dmfIndexB(std::deque<std::wstring> &values) ;

  // make 1 doc with multi valued field
  virtual void
  make1dmfIndex(std::shared_ptr<Analyzer> analyzer,
                std::deque<std::wstring> &values) ;

  // make 1 doc with multi valued & not analyzed field
  virtual void
  make1dmfIndexNA(std::deque<std::wstring> &values) ;

  virtual void makeIndexShortMV() ;

  virtual void makeIndexLongMV() ;

  virtual void makeIndexLongMVB() ;

  virtual void makeIndexStrMV() ;

protected:
  std::shared_ptr<AbstractTestCase> shared_from_this()
  {
    return std::static_pointer_cast<AbstractTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
