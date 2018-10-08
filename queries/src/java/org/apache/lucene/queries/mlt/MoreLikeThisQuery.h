#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"

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
namespace org::apache::lucene::queries::mlt
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using Query = org::apache::lucene::search::Query;

/**
 * A simple wrapper for MoreLikeThis for use in scenarios where a Query object
 * is required eg in custom QueryParser extensions. At query.rewrite() time the
 * reader is used to construct the actual MoreLikeThis object and obtain the
 * real Query object.
 */
class MoreLikeThisQuery : public Query
{
  GET_CLASS_NAME(MoreLikeThisQuery)

private:
  std::wstring likeText;
  std::deque<std::wstring> moreLikeFields;
  std::shared_ptr<Analyzer> analyzer;
  const std::wstring fieldName;
  float percentTermsToMatch = 0.3f;
  int minTermFrequency = 1;
  int maxQueryTerms = 5;
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private std::unordered_set<?> stopWords = null;
  std::shared_ptr < Set < ? >> stopWords = nullptr;
  int minDocFreq = -1;

  /**
   * @param moreLikeFields fields used for similarity measure
   */
public:
  MoreLikeThisQuery(const std::wstring &likeText,
                    std::deque<std::wstring> &moreLikeFields,
                    std::shared_ptr<Analyzer> analyzer,
                    const std::wstring &fieldName);

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  /* (non-Javadoc)
   * @see org.apache.lucene.search.Query#toString(java.lang.std::wstring)
   */
  std::wstring toString(const std::wstring &field) override;

  virtual float getPercentTermsToMatch();

  virtual void setPercentTermsToMatch(float percentTermsToMatch);

  virtual std::shared_ptr<Analyzer> getAnalyzer();

  virtual void setAnalyzer(std::shared_ptr<Analyzer> analyzer);

  virtual std::wstring getLikeText();

  virtual void setLikeText(const std::wstring &likeText);

  virtual int getMaxQueryTerms();

  virtual void setMaxQueryTerms(int maxQueryTerms);

  virtual int getMinTermFrequency();

  virtual void setMinTermFrequency(int minTermFrequency);

  virtual std::deque<std::wstring> getMoreLikeFields();

  virtual void setMoreLikeFields(std::deque<std::wstring> &moreLikeFields);

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: public std::unordered_set<?> getStopWords()
  virtual std::shared_ptr < Set < ? >> getStopWords();

  template <typename T1>
  void setStopWords(std::shared_ptr<Set<T1>> stopWords);

  virtual int getMinDocFreq();

  virtual void setMinDocFreq(int minDocFreq);

  virtual int hashCode();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<MoreLikeThisQuery> other);

protected:
  std::shared_ptr<MoreLikeThisQuery> shared_from_this()
  {
    return std::static_pointer_cast<MoreLikeThisQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/mlt/
