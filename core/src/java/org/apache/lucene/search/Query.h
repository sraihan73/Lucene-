#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;

/** The abstract base class for queries.
    <p>Instantiable subclasses are:
    <ul>
    <li> {@link TermQuery}
    <li> {@link BooleanQuery}
    <li> {@link WildcardQuery}
    <li> {@link PhraseQuery}
    <li> {@link PrefixQuery}
    <li> {@link MultiPhraseQuery}
    <li> {@link FuzzyQuery}
    <li> {@link RegexpQuery}
    <li> {@link TermRangeQuery}
    <li> {@link PointRangeQuery}
    <li> {@link ConstantScoreQuery}
    <li> {@link DisjunctionMaxQuery}
    <li> {@link MatchAllDocsQuery}
    </ul>
    <p>See also the family of {@link org.apache.lucene.search.spans Span
   Queries} and additional queries available in the <a
   href="{@docRoot}/../queries/overview-summary.html">Queries module</a>
*/
class Query : public std::enable_shared_from_this<Query>
{
  GET_CLASS_NAME(Query)

  /** Prints a query to a string, with <code>field</code> assumed to be the
   * default field and omitted.
   */
public:
  virtual std::wstring toString(const std::wstring &field) = 0;

  /** Prints a query to a string. */
  virtual std::wstring toString();

  /**
   * Expert: Constructs an appropriate Weight implementation for this query.
   * <p>
   * Only implemented by primitive queries, which re-write to themselves.
   *
   * @param needsScores   True if document scores ({@link Scorer#score}) are
   * needed.
   * @param boost         The boost that is propagated by the parent queries.
   */
  virtual std::shared_ptr<Weight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost) ;

  /** Expert: called to re-write queries into primitive queries. For example,
   * a PrefixQuery will be rewritten into a BooleanQuery that consists
   * of TermQuerys.
   */
  virtual std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader) ;

  /**
   * Override and implement query instance equivalence properly in a subclass.
   * This is required so that {@link QueryCache} works properly.
GET_CLASS_NAME(.)
   *
   * Typically a query will be equal to another only if it's an instance of
   * the same class and its document-filtering properties are identical that
other
   * instance. Utility methods are provided for certain repetitive code.
   *
   * @see #sameClassAs(Object)
   * @see #classHash()
   */
  virtual bool equals(std::any obj) = 0;
  

      /**
       * Override and implement query hash code properly in a subclass.
       * This is required so that {@link QueryCache} works properly.
GET_CLASS_NAME(.)
       *
       * @see #equals(Object)
       */
  virtual int hashCode() = 0;
  

      /**
       * Utility method to check whether <code>other</code> is not null and is
exactly
       * of the same class as this object's class.
       *
       * When this method is used in an implementation of {@link
#equals(Object)},
       * consider using {@link #classHash()} in the implementation
       * of {@link #hashCode} to differentiate different class
GET_CLASS_NAME(Hash()})
       */
      protected : 
        bool sameClassAs(std::any other);

private:
  const size_t CLASS_NAME_HASH = std::hash<std::wstring>()(_getClassName());

  /**
   * Provides a constant integer for a given class, derived from the name of the
class.
   * The rationale for not using just {@link Class#hashCode()} is that classes
may be GET_CLASS_NAME(,)
   * assigned different hash codes for each execution and we want hashes to be
possibly
   * consistent to facilitate debugging.
   */
protected:
  int classHash();
};

} // namespace org::apache::lucene::search
