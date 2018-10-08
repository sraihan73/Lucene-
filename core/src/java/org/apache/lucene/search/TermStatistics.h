#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;
/**
 * Contains statistics for a specific term
 * @lucene.experimental
 */
class TermStatistics : public std::enable_shared_from_this<TermStatistics>
{
  GET_CLASS_NAME(TermStatistics)
private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<BytesRef> term_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t docFreq_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t totalTermFreq_;

public:
  TermStatistics(std::shared_ptr<BytesRef> term, int64_t docFreq,
                 int64_t totalTermFreq);

  /** returns the term text */
  std::shared_ptr<BytesRef> term();

  /** returns the number of documents this term occurs in
   * @see TermsEnum#docFreq() */
  int64_t docFreq();

  /** returns the total number of occurrences of this term
   * @see TermsEnum#totalTermFreq() */
  int64_t totalTermFreq();
};

} // #include  "core/src/java/org/apache/lucene/search/
