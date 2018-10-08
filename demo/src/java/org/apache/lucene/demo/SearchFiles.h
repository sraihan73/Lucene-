#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

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
namespace org::apache::lucene::demo
{

using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

/** Simple command-line based search demo. */
class SearchFiles : public std::enable_shared_from_this<SearchFiles>
{
  GET_CLASS_NAME(SearchFiles)

private:
  SearchFiles();

  /** Simple command-line based search demo. */
  static void main(std::deque<std::wstring> &args) ;

  /**
   * This demonstrates a typical paging search scenario, where the search engine
   * presents pages of size n to the user. The user can then go to the next page
   * if interested in the next hits.
   *
   * When the query is executed for the first time, then only enough results are
   * collected to fill 5 result pages. If the user wants to page beyond this
   * limit, then the query is executed another time and all hits are collected.
   *
   */
public:
  static void doPagingSearch(std::shared_ptr<BufferedReader> in_,
                             std::shared_ptr<IndexSearcher> searcher,
                             std::shared_ptr<Query> query, int hitsPerPage,
                             bool raw, bool interactive) ;
};

} // #include  "core/src/java/org/apache/lucene/demo/
