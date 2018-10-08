#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>
#include "core/src/java/org/apache/lucene/search/TopDocs.h"
#include "core/src/java/org/apache/lucene/search/SortField.h"
#include "core/src/java/org/apache/lucene/search/ScoreDoc.h"

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
/** Represents hits returned by {@link
 * IndexSearcher#search(Query,int,Sort)}.
 */
class TopFieldDocs : public TopDocs
{
  GET_CLASS_NAME(TopFieldDocs)

  /** The fields which were used to sort results by. */
public:
  std::deque<std::shared_ptr<SortField>> fields;

  /** Creates one of these objects.
   * @param totalHits  Total number of hits for the query.
   * @param scoreDocs  The top hits for the query.
   * @param fields     The sort criteria used to find the top hits.
   * @param maxScore   The maximum score encountered.
   */
  TopFieldDocs(int64_t totalHits,
               std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs,
               std::deque<std::shared_ptr<SortField>> &fields, float maxScore);

protected:
  std::shared_ptr<TopFieldDocs> shared_from_this()
  {
    return std::static_pointer_cast<TopFieldDocs>(TopDocs::shared_from_this());
  }
};
} // namespace org::apache::lucene::search
