#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/PhraseQuery.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"

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
using Term = org::apache::lucene::index::Term;

/**
 * This is a {@link PhraseQuery} which is optimized for n-gram phrase query.
 * For example, when you query "ABCD" on a 2-gram field, you may want to use
 * NGramPhraseQuery rather than {@link PhraseQuery}, because NGramPhraseQuery
 * will {@link #rewrite(IndexReader)} the query to "AB/0 CD/2", while {@link
 * PhraseQuery} will query "AB/0 BC/1 CD/2" (where term/position).
 *
 */
class NGramPhraseQuery : public Query
{
  GET_CLASS_NAME(NGramPhraseQuery)

private:
  const int n;
  const std::shared_ptr<PhraseQuery> phraseQuery;

  /**
   * Constructor that takes gram size.
   * @param n n-gram size
   */
public:
  NGramPhraseQuery(int n, std::shared_ptr<PhraseQuery> query);

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<NGramPhraseQuery> other);

public:
  virtual int hashCode();

  /** Return the n in n-gram */
  virtual int getN();

  /** Return the deque of terms. */
  virtual std::deque<std::shared_ptr<Term>> getTerms();

  /** Return the deque of relative positions that each term should appear at. */
  virtual std::deque<int> getPositions();

  std::wstring toString(const std::wstring &field) override;

protected:
  std::shared_ptr<NGramPhraseQuery> shared_from_this()
  {
    return std::static_pointer_cast<NGramPhraseQuery>(
        Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
