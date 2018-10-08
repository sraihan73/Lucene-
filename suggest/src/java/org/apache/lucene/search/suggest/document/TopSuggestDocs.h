#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

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

using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;

/**
 * {@link org.apache.lucene.search.TopDocs} wrapper with
 * an additional std::wstring key per {@link org.apache.lucene.search.ScoreDoc}
 *
 * @lucene.experimental
 */
class TopSuggestDocs : public TopDocs
{
  GET_CLASS_NAME(TopSuggestDocs)

  /**
   * Singleton for empty {@link TopSuggestDocs}
   */
public:
  static const std::shared_ptr<TopSuggestDocs> EMPTY;

  /**
   * {@link org.apache.lucene.search.ScoreDoc} with an
   * additional std::wstring key
   */
public:
  class SuggestScoreDoc : public ScoreDoc,
                          public Comparable<std::shared_ptr<SuggestScoreDoc>>
  {
    GET_CLASS_NAME(SuggestScoreDoc)

    /**
     * Matched completion key
     */
  public:
    const std::shared_ptr<std::wstring> key;

    /**
     * Context for the completion
     */
    const std::shared_ptr<std::wstring> context;

    /**
     * Creates a SuggestScoreDoc instance
     *
     * @param doc   document id (hit)
     * @param key   matched completion
     * @param score weight of the matched completion
     */
    SuggestScoreDoc(int doc, std::shared_ptr<std::wstring> key,
                    std::shared_ptr<std::wstring> context, float score);

    int compareTo(std::shared_ptr<SuggestScoreDoc> o) override;

    bool equals(std::any other) override;

    virtual int hashCode();

    virtual std::wstring toString();

  protected:
    std::shared_ptr<SuggestScoreDoc> shared_from_this()
    {
      return std::static_pointer_cast<SuggestScoreDoc>(
          org.apache.lucene.search.ScoreDoc::shared_from_this());
    }
  };

  /**
   * {@link org.apache.lucene.search.TopDocs} wrapper with
   * {@link TopSuggestDocs.SuggestScoreDoc}
   * instead of {@link org.apache.lucene.search.ScoreDoc}
   */
public:
  TopSuggestDocs(int totalHits,
                 std::deque<std::shared_ptr<SuggestScoreDoc>> &scoreDocs,
                 float maxScore);

  /**
   * Returns {@link TopSuggestDocs.SuggestScoreDoc}s
   * for this instance
   */
  virtual std::deque<std::shared_ptr<SuggestScoreDoc>> scoreLookupDocs();

  /**
   * Returns a new TopSuggestDocs, containing topN results across
   * the provided TopSuggestDocs, sorting by score. Each {@link TopSuggestDocs}
   * instance must be sorted.
   * Analogous to {@link org.apache.lucene.search.TopDocs#merge(int,
   * org.apache.lucene.search.TopDocs[])} for {@link TopSuggestDocs}
   *
   * NOTE: assumes every <code>shardHit</code> is already sorted by score
   */
  static std::shared_ptr<TopSuggestDocs>
  merge(int topN, std::deque<std::shared_ptr<TopSuggestDocs>> &shardHits);

protected:
  std::shared_ptr<TopSuggestDocs> shared_from_this()
  {
    return std::static_pointer_cast<TopSuggestDocs>(
        org.apache.lucene.search.TopDocs::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
