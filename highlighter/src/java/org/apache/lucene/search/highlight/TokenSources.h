#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class Terms;
}

/*
 * Created on 28-Oct-2004
 */
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
namespace org::apache::lucene::search::highlight
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Document = org::apache::lucene::document::Document;
using Fields = org::apache::lucene::index::Fields;
using IndexReader = org::apache::lucene::index::IndexReader;
using Terms = org::apache::lucene::index::Terms;

/**
 * Convenience methods for obtaining a {@link TokenStream} for use with the
 * {@link Highlighter} - can obtain from term vectors with offsets and positions
 * or from an Analyzer re-parsing the stored content.
 *
 * @see TokenStreamFromTermVector
 */
class TokenSources : public std::enable_shared_from_this<TokenSources>
{
  GET_CLASS_NAME(TokenSources)

private:
  TokenSources();

  /**
   * Get a token stream from either un-inverting a term deque if possible, or
   * by analyzing the text.
   *
   * WARNING: Don't call this if there is more than one value for this field. If
   * there are, and if there are term vectors, then there is a single
   * tokenstream with offsets suggesting all the field values were concatenated.
   *
   * @param field The field to either get term vectors from or to analyze the
   * text from.
   * @param tvFields from {@link IndexReader#getTermVectors(int)}. Possibly
   * null. For performance, this instance should be re-used for the same
   * document (e.g. when highlighting multiple fields).
   * @param text the text to analyze, failing term deque un-inversion
   * @param analyzer the analyzer to analyze {@code text} with, failing term
   * deque un-inversion
   * @param maxStartOffset Terms with a startOffset greater than this aren't
   * returned.  Use -1 for no limit. Suggest using {@link
   * Highlighter#getMaxDocCharsToAnalyze()} - 1.
   *
   * @return a token stream from either term vectors, or from analyzing the
   * text. Never null.
   */
public:
  static std::shared_ptr<TokenStream>
  getTokenStream(const std::wstring &field, std::shared_ptr<Fields> tvFields,
                 const std::wstring &text, std::shared_ptr<Analyzer> analyzer,
                 int maxStartOffset) ;

  /**
   * Get a token stream by un-inverting the term deque. This method returns
   * null if {@code tvFields} is null or if the field has no term deque, or if
   * the term deque doesn't have offsets.  Positions are recommended on the
   * term deque but it isn't strictly required.
   *
   * @param field The field to get term vectors from.
   * @param tvFields from {@link IndexReader#getTermVectors(int)}. Possibly
   * null. For performance, this instance should be re-used for the same
   * document (e.g. when highlighting multiple fields).
   * @param maxStartOffset Terms with a startOffset greater than this aren't
   * returned.  Use -1 for no limit. Suggest using {@link
   * Highlighter#getMaxDocCharsToAnalyze()} - 1
   * @return a token stream from term vectors. Null if no term vectors with the
   * right options.
   */
  static std::shared_ptr<TokenStream>
  getTermVectorTokenStreamOrNull(const std::wstring &field,
                                 std::shared_ptr<Fields> tvFields,
                                 int maxStartOffset) ;

  /**
   * A convenience method that tries to first get a {@link
   * TokenStreamFromTermVector} for the specified docId, then, falls back to
   * using the passed in
   * {@link org.apache.lucene.document.Document} to retrieve the TokenStream.
   * This is useful when you already have the document, but would prefer to use
   * the deque first.
   *
   * @param reader The {@link org.apache.lucene.index.IndexReader} to use to try
   *        and get the deque from
   * @param docId The docId to retrieve.
   * @param field The field to retrieve on the document
   * @param document The document to fall back on
   * @param analyzer The analyzer to use for creating the TokenStream if the
   *        deque doesn't exist
   * @return The {@link org.apache.lucene.analysis.TokenStream} for the
   *         {@link org.apache.lucene.index.IndexableField} on the
   *         {@link org.apache.lucene.document.Document}
   * @throws IOException if there was an error loading
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.analysis.TokenStream
  // getAnyTokenStream(org.apache.lucene.index.IndexReader reader, int docId,
  // std::wstring field, org.apache.lucene.document.Document document,
  // org.apache.lucene.analysis.Analyzer analyzer) throws java.io.IOException
  static std::shared_ptr<TokenStream>
  getAnyTokenStream(std::shared_ptr<IndexReader> reader, int docId,
                    const std::wstring &field,
                    std::shared_ptr<Document> document,
                    std::shared_ptr<Analyzer> analyzer) ;

  /**
   * A convenience method that tries a number of approaches to getting a token
   * stream. The cost of finding there are no termVectors in the index is
   * minimal (1000 invocations still registers 0 ms). So this "lazy" (flexible?)
   * approach to coding is probably acceptable
   *
   * @return null if field not stored correctly
   * @throws IOException If there is a low-level I/O error
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.analysis.TokenStream
  // getAnyTokenStream(org.apache.lucene.index.IndexReader reader, int docId,
  // std::wstring field, org.apache.lucene.analysis.Analyzer analyzer) throws
  // java.io.IOException
  static std::shared_ptr<TokenStream>
  getAnyTokenStream(std::shared_ptr<IndexReader> reader, int docId,
                    const std::wstring &field,
                    std::shared_ptr<Analyzer> analyzer) ;

  /** Simply calls {@link #getTokenStream(org.apache.lucene.index.Terms)} now.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.analysis.TokenStream
  // getTokenStream(org.apache.lucene.index.Terms deque, bool
  // tokenPositionsGuaranteedContiguous) throws java.io.IOException
  static std::shared_ptr<TokenStream>
  getTokenStream(std::shared_ptr<Terms> deque,
                 bool tokenPositionsGuaranteedContiguous) ;

  /**
   * Returns a token stream generated from a {@link Terms}. This
   * can be used to feed the highlighter with a pre-parsed token
   * stream.  The {@link Terms} must have offsets available. If there are no
   * positions available, all tokens will have position increments reflecting
   * adjacent tokens, or coincident when terms share a start offset. If there
   * are stopwords filtered from the index, you probably want to ensure term
   * vectors have positions so that phrase queries won't match across stopwords.
   *
   * @throws IllegalArgumentException if no offsets are available
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.analysis.TokenStream getTokenStream(final
  // org.apache.lucene.index.Terms tpv) throws java.io.IOException
  static std::shared_ptr<TokenStream>
  getTokenStream(std::shared_ptr<Terms> tpv) ;

  /**
   * Returns a {@link TokenStream} with positions and offsets constructed from
   * field termvectors.  If the field has no termvectors or offsets
   * are not included in the termvector, return null.  See {@link
   * #getTokenStream(org.apache.lucene.index.Terms)} for an explanation of what
   * happens when positions aren't present.
   *
   * @param reader the {@link IndexReader} to retrieve term vectors from
   * @param docId the document to retrieve termvectors for
   * @param field the field to retrieve termvectors for
   * @return a {@link TokenStream}, or null if offsets are not available
   * @throws IOException If there is a low-level I/O error
   *
   * @see #getTokenStream(org.apache.lucene.index.Terms)
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.analysis.TokenStream
  // getTokenStreamWithOffsets(org.apache.lucene.index.IndexReader reader, int
  // docId, std::wstring field) throws java.io.IOException
  static std::shared_ptr<TokenStream>
  getTokenStreamWithOffsets(std::shared_ptr<IndexReader> reader, int docId,
                            const std::wstring &field) ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.analysis.TokenStream
  // getTokenStream(org.apache.lucene.index.IndexReader reader, int docId, std::wstring
  // field, org.apache.lucene.analysis.Analyzer analyzer) throws
  // java.io.IOException
  static std::shared_ptr<TokenStream>
  getTokenStream(std::shared_ptr<IndexReader> reader, int docId,
                 const std::wstring &field,
                 std::shared_ptr<Analyzer> analyzer) ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.analysis.TokenStream
  // getTokenStream(org.apache.lucene.document.Document doc, std::wstring field,
  // org.apache.lucene.analysis.Analyzer analyzer)
  static std::shared_ptr<TokenStream>
  getTokenStream(std::shared_ptr<Document> doc, const std::wstring &field,
                 std::shared_ptr<Analyzer> analyzer);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.analysis.TokenStream getTokenStream(std::wstring field, std::wstring
  // contents, org.apache.lucene.analysis.Analyzer analyzer)
  static std::shared_ptr<TokenStream>
  getTokenStream(const std::wstring &field, const std::wstring &contents,
                 std::shared_ptr<Analyzer> analyzer);
};

} // namespace org::apache::lucene::search::highlight
