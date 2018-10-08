#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

#include  "core/src/java/org/apache/lucene/search/suggest/document/CompletionQuery.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/TopSuggestDocs.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/TopSuggestDocsCollector.h"

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
namespace org::apache::lucene::search::suggest::document
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/**
 * Adds document suggest capabilities to IndexSearcher.
 * Any {@link CompletionQuery} can be used to suggest documents.
 *
 * Use {@link PrefixCompletionQuery} for analyzed prefix queries,
 * {@link RegexCompletionQuery} for regular expression prefix queries,
 * {@link FuzzyCompletionQuery} for analyzed prefix with typo tolerance
 * and {@link ContextQuery} to boost and/or filter suggestions by contexts
 *
 * @lucene.experimental
 */
class SuggestIndexSearcher : public IndexSearcher
{
  GET_CLASS_NAME(SuggestIndexSearcher)

  // NOTE: we do not accept an ExecutorService here, because at least the dedup
  // logic in TopSuggestDocsCollector/NRTSuggester would not be thread safe (and
  // maybe other things)

  /**
   * Creates a searcher with document suggest capabilities
   * for <code>reader</code>.
   */
public:
  SuggestIndexSearcher(std::shared_ptr<IndexReader> reader);

  /**
   * Returns top <code>n</code> completion hits for
   * <code>query</code>
   */
  virtual std::shared_ptr<TopSuggestDocs>
  suggest(std::shared_ptr<CompletionQuery> query, int n,
          bool skipDuplicates) ;

  /**
   * Lower-level suggest API.
   * Collects completion hits through <code>collector</code> for
   * <code>query</code>.
   *
   * <p>{@link TopSuggestDocsCollector#collect(int, std::wstring, std::wstring,
   * float)} is called for every matching completion hit.
   */
  virtual void suggest(
      std::shared_ptr<CompletionQuery> query,
      std::shared_ptr<TopSuggestDocsCollector> collector) ;

protected:
  std::shared_ptr<SuggestIndexSearcher> shared_from_this()
  {
    return std::static_pointer_cast<SuggestIndexSearcher>(
        org.apache.lucene.search.IndexSearcher::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
