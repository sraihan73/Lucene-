#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

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

/** Holds one hit in {@link TopDocs}. */

class ScoreDoc : public std::enable_shared_from_this<ScoreDoc>
{
  GET_CLASS_NAME(ScoreDoc)

  /** The score of this document for the query. */
public:
  float score = 0;

  /** A hit document's number.
   * @see IndexSearcher#doc(int) */
  int doc = 0;

  /** Only set by {@link TopDocs#merge}*/
  int shardIndex = 0;

  ScoreDoc()
  {
  }

  /** Constructs a ScoreDoc. */
  ScoreDoc(int doc, float score);

  /** Constructs a ScoreDoc. */
  ScoreDoc(int doc, float score, int shardIndex);

  // A convenience method for debugging.
  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/search/
