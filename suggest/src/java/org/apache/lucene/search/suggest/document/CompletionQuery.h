#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::search::suggest
{
class BitsProducer;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class Query;
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
namespace org::apache::lucene::search::suggest::document
{

using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;
using BitsProducer = org::apache::lucene::search::suggest::BitsProducer;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.CompletionAnalyzer.HOLE_CHARACTER;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.miscellaneous.ConcatenateGraphFilter.SEP_LABEL;

/**
 * Abstract {@link Query} that match documents containing terms with a specified
 * prefix filtered by {@link BitsProducer}. This should be used to query against
 * any {@link SuggestField}s or {@link ContextSuggestField}s of documents. <p>
 * Use {@link SuggestIndexSearcher#suggest(CompletionQuery, int, bool)} to
 * execute any query that provides a concrete implementation of this query.
 * Example below shows using this query to retrieve the top 5 documents.
 *
 * <pre class="prettyprint">
 *  SuggestIndexSearcher searcher = new SuggestIndexSearcher(reader);
 *  TopSuggestDocs suggestDocs = searcher.suggest(query, 5);
 * </pre>
 * This query rewrites to an appropriate {@link CompletionQuery} depending on
 * the type ({@link SuggestField} or {@link ContextSuggestField}) of the field
 * the query is run against.
 *
 * @lucene.experimental
 */
class CompletionQuery : public Query
{
  GET_CLASS_NAME(CompletionQuery)

  /**
   * Term to query against
   */
private:
  const std::shared_ptr<Term> term;

  /**
   * {@link BitsProducer} which is used to filter the document scope.
   */
  const std::shared_ptr<BitsProducer> filter;

  /**
   * Creates a base Completion query against a <code>term</code>
   * with a <code>filter</code> to scope the documents
   */
protected:
  CompletionQuery(std::shared_ptr<Term> term,
                  std::shared_ptr<BitsProducer> filter);

  /**
   * Returns a {@link BitsProducer}. Only suggestions matching the returned
   * bits will be returned.
   */
public:
  virtual std::shared_ptr<BitsProducer> getFilter();

  /**
   * Returns the field name this query should
   * be run against
   */
  virtual std::wstring getField();

  /**
   * Returns the term to be queried against
   */
  virtual std::shared_ptr<Term> getTerm();

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::wstring toString(const std::wstring &field) override;

private:
  void validate(const std::wstring &termText);

protected:
  std::shared_ptr<CompletionQuery> shared_from_this()
  {
    return std::static_pointer_cast<CompletionQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
