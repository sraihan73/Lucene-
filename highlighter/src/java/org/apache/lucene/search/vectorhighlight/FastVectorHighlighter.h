#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FragListBuilder.h"

#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FragmentsBuilder.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldQuery.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/highlight/Encoder.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldFragList.h"

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

using IndexReader = org::apache::lucene::index::IndexReader;
using Query = org::apache::lucene::search::Query;
using Encoder = org::apache::lucene::search::highlight::Encoder;

/**
 * Another highlighter implementation.
 *
 */
class FastVectorHighlighter
    : public std::enable_shared_from_this<FastVectorHighlighter>
{
  GET_CLASS_NAME(FastVectorHighlighter)
public:
  static constexpr bool DEFAULT_PHRASE_HIGHLIGHT = true;
  static constexpr bool DEFAULT_FIELD_MATCH = true;

private:
  const bool phraseHighlight;
  const bool fieldMatch;
  const std::shared_ptr<FragListBuilder> fragListBuilder;
  const std::shared_ptr<FragmentsBuilder> fragmentsBuilder;
  int phraseLimit = std::numeric_limits<int>::max();

  /**
   * the default constructor.
   */
public:
  FastVectorHighlighter();

  /**
   * a constructor. Using {@link SimpleFragListBuilder} and {@link
   * ScoreOrderFragmentsBuilder}.
   *
   * @param phraseHighlight true or false for phrase highlighting
   * @param fieldMatch true of false for field matching
   */
  FastVectorHighlighter(bool phraseHighlight, bool fieldMatch);

  /**
   * a constructor. A {@link FragListBuilder} and a {@link FragmentsBuilder} can
   * be specified (plugins).
   *
   * @param phraseHighlight true of false for phrase highlighting
   * @param fieldMatch true of false for field matching
   * @param fragListBuilder an instance of {@link FragListBuilder}
   * @param fragmentsBuilder an instance of {@link FragmentsBuilder}
   */
  FastVectorHighlighter(bool phraseHighlight, bool fieldMatch,
                        std::shared_ptr<FragListBuilder> fragListBuilder,
                        std::shared_ptr<FragmentsBuilder> fragmentsBuilder);

  /**
   * create a {@link FieldQuery} object.
   *
   * @param query a query
   * @return the created {@link FieldQuery} object
   */
  virtual std::shared_ptr<FieldQuery>
  getFieldQuery(std::shared_ptr<Query> query);

  /**
   * create a {@link FieldQuery} object.
   *
   * @param query a query
   * @return the created {@link FieldQuery} object
   */
  virtual std::shared_ptr<FieldQuery>
  getFieldQuery(std::shared_ptr<Query> query,
                std::shared_ptr<IndexReader> reader) ;

  /**
   * return the best fragment.
   *
   * @param fieldQuery {@link FieldQuery} object
   * @param reader {@link IndexReader} of the index
   * @param docId document id to be highlighted
   * @param fieldName field of the document to be highlighted
   * @param fragCharSize the length (number of chars) of a fragment
   * @return the best fragment (snippet) string
   * @throws IOException If there is a low-level I/O error
   */
  std::wstring getBestFragment(std::shared_ptr<FieldQuery> fieldQuery,
                               std::shared_ptr<IndexReader> reader, int docId,
                               const std::wstring &fieldName,
                               int fragCharSize) ;

  /**
   * return the best fragments.
   *
   * @param fieldQuery {@link FieldQuery} object
   * @param reader {@link IndexReader} of the index
   * @param docId document id to be highlighted
   * @param fieldName field of the document to be highlighted
   * @param fragCharSize the length (number of chars) of a fragment
   * @param maxNumFragments maximum number of fragments
   * @return created fragments or null when no fragments created.
   *         size of the array can be less than maxNumFragments
   * @throws IOException If there is a low-level I/O error
   */
  std::deque<std::wstring>
  getBestFragments(std::shared_ptr<FieldQuery> fieldQuery,
                   std::shared_ptr<IndexReader> reader, int docId,
                   const std::wstring &fieldName, int fragCharSize,
                   int maxNumFragments) ;

  /**
   * return the best fragment.
   *
   * @param fieldQuery {@link FieldQuery} object
   * @param reader {@link IndexReader} of the index
   * @param docId document id to be highlighted
   * @param fieldName field of the document to be highlighted
   * @param fragCharSize the length (number of chars) of a fragment
   * @param fragListBuilder {@link FragListBuilder} object
   * @param fragmentsBuilder {@link FragmentsBuilder} object
   * @param preTags pre-tags to be used to highlight terms
   * @param postTags post-tags to be used to highlight terms
   * @param encoder an encoder that generates encoded text
   * @return the best fragment (snippet) string
   * @throws IOException If there is a low-level I/O error
   */
  std::wstring
  getBestFragment(std::shared_ptr<FieldQuery> fieldQuery,
                  std::shared_ptr<IndexReader> reader, int docId,
                  const std::wstring &fieldName, int fragCharSize,
                  std::shared_ptr<FragListBuilder> fragListBuilder,
                  std::shared_ptr<FragmentsBuilder> fragmentsBuilder,
                  std::deque<std::wstring> &preTags,
                  std::deque<std::wstring> &postTags,
                  std::shared_ptr<Encoder> encoder) ;

  /**
   * return the best fragments.
   *
   * @param fieldQuery {@link FieldQuery} object
   * @param reader {@link IndexReader} of the index
   * @param docId document id to be highlighted
   * @param fieldName field of the document to be highlighted
   * @param fragCharSize the length (number of chars) of a fragment
   * @param maxNumFragments maximum number of fragments
   * @param fragListBuilder {@link FragListBuilder} object
   * @param fragmentsBuilder {@link FragmentsBuilder} object
   * @param preTags pre-tags to be used to highlight terms
   * @param postTags post-tags to be used to highlight terms
   * @param encoder an encoder that generates encoded text
   * @return created fragments or null when no fragments created.
   *         size of the array can be less than maxNumFragments
   * @throws IOException If there is a low-level I/O error
   */
  std::deque<std::wstring> getBestFragments(
      std::shared_ptr<FieldQuery> fieldQuery,
      std::shared_ptr<IndexReader> reader, int docId,
      const std::wstring &fieldName, int fragCharSize, int maxNumFragments,
      std::shared_ptr<FragListBuilder> fragListBuilder,
      std::shared_ptr<FragmentsBuilder> fragmentsBuilder,
      std::deque<std::wstring> &preTags, std::deque<std::wstring> &postTags,
      std::shared_ptr<Encoder> encoder) ;

  /**
   * Return the best fragments.  Matches are scanned from matchedFields and
   * turned into fragments against storedField.  The highlighting may not make
   * sense if matchedFields has matches with offsets that don't correspond
   * features in storedField.  It will outright throw a {@code
   * StringIndexOutOfBoundsException} if matchedFields produces offsets outside
   * of storedField.  As such it is advisable that all matchedFields share the
   * same source as storedField or are at least a prefix of it.
   *
   * @param fieldQuery {@link FieldQuery} object
   * @param reader {@link IndexReader} of the index
   * @param docId document id to be highlighted
   * @param storedField field of the document that stores the text
   * @param matchedFields fields of the document to scan for matches
   * @param fragCharSize the length (number of chars) of a fragment
   * @param maxNumFragments maximum number of fragments
   * @param fragListBuilder {@link FragListBuilder} object
   * @param fragmentsBuilder {@link FragmentsBuilder} object
   * @param preTags pre-tags to be used to highlight terms
   * @param postTags post-tags to be used to highlight terms
   * @param encoder an encoder that generates encoded text
   * @return created fragments or null when no fragments created.
   *         size of the array can be less than maxNumFragments
   * @throws IOException If there is a low-level I/O error
   */
  std::deque<std::wstring> getBestFragments(
      std::shared_ptr<FieldQuery> fieldQuery,
      std::shared_ptr<IndexReader> reader, int docId,
      const std::wstring &storedField,
      std::shared_ptr<Set<std::wstring>> matchedFields, int fragCharSize,
      int maxNumFragments, std::shared_ptr<FragListBuilder> fragListBuilder,
      std::shared_ptr<FragmentsBuilder> fragmentsBuilder,
      std::deque<std::wstring> &preTags, std::deque<std::wstring> &postTags,
      std::shared_ptr<Encoder> encoder) ;

  /**
   * Build a FieldFragList for one field.
   */
private:
  std::shared_ptr<FieldFragList>
  getFieldFragList(std::shared_ptr<FragListBuilder> fragListBuilder,
                   std::shared_ptr<FieldQuery> fieldQuery,
                   std::shared_ptr<IndexReader> reader, int docId,
                   const std::wstring &matchedField,
                   int fragCharSize) ;

  /**
   * Build a FieldFragList for more than one field.
   */
  std::shared_ptr<FieldFragList>
  getFieldFragList(std::shared_ptr<FragListBuilder> fragListBuilder,
                   std::shared_ptr<FieldQuery> fieldQuery,
                   std::shared_ptr<IndexReader> reader, int docId,
                   std::shared_ptr<Set<std::wstring>> matchedFields,
                   int fragCharSize) ;

  /**
   * return whether phraseHighlight or not.
   *
   * @return whether phraseHighlight or not
   */
public:
  virtual bool isPhraseHighlight();

  /**
   * return whether fieldMatch or not.
   *
   * @return whether fieldMatch or not
   */
  virtual bool isFieldMatch();

  /**
   * @return the maximum number of phrases to analyze when searching for the
   * highest-scoring phrase.
   */
  virtual int getPhraseLimit();

  /**
   * set the maximum number of phrases to analyze when searching for the
   * highest-scoring phrase. The default is unlimited (Integer.MAX_VALUE).
   */
  virtual void setPhraseLimit(int phraseLimit);
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
