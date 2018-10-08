#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Sort.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"

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

/**
 * A {@link Rescorer} that re-sorts according to a provided
 * Sort.
 */

class SortRescorer : public Rescorer
{
  GET_CLASS_NAME(SortRescorer)

private:
  const std::shared_ptr<Sort> sort;

  /** Sole constructor. */
public:
  SortRescorer(std::shared_ptr<Sort> sort);

  std::shared_ptr<TopDocs> rescore(std::shared_ptr<IndexSearcher> searcher,
                                   std::shared_ptr<TopDocs> firstPassTopDocs,
                                   int topN)  override;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<ScoreDoc>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<SortRescorer> outerInstance;

  public:
    ComparatorAnonymousInnerClass(std::shared_ptr<SortRescorer> outerInstance);

    int compare(std::shared_ptr<ScoreDoc> a, std::shared_ptr<ScoreDoc> b);
  };

public:
  std::shared_ptr<Explanation>
  explain(std::shared_ptr<IndexSearcher> searcher,
          std::shared_ptr<Explanation> firstPassExplanation,
          int docID)  override;

protected:
  std::shared_ptr<SortRescorer> shared_from_this()
  {
    return std::static_pointer_cast<SortRescorer>(Rescorer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
