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

/**
 * Contains statistics for a collection (field)
 * @lucene.experimental
 */
class CollectionStatistics
    : public std::enable_shared_from_this<CollectionStatistics>
{
  GET_CLASS_NAME(CollectionStatistics)
private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::wstring field_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t maxDoc_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t docCount_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t sumTotalTermFreq_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t sumDocFreq_;

public:
  CollectionStatistics(const std::wstring &field, int64_t maxDoc,
                       int64_t docCount, int64_t sumTotalTermFreq,
                       int64_t sumDocFreq);

  /** returns the field name */
  std::wstring field();

  /** returns the total number of documents, regardless of
   * whether they all contain values for this field.
   * @see IndexReader#maxDoc() */
  int64_t maxDoc();

  /** returns the total number of documents that
   * have at least one term for this field.
   * @see Terms#getDocCount() */
  int64_t docCount();

  /** returns the total number of tokens for this field
   * @see Terms#getSumTotalTermFreq() */
  int64_t sumTotalTermFreq();

  /** returns the total number of postings for this field
   * @see Terms#getSumDocFreq() */
  int64_t sumDocFreq();
};

} // #include  "core/src/java/org/apache/lucene/search/
