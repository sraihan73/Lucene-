#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"

#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/Sorter.h"
#include  "core/src/java/org/apache/lucene/index/DocMap.h"
#include  "core/src/java/org/apache/lucene/index/DocComparator.h"
#include  "core/src/java/org/apache/lucene/search/SortField.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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
namespace org::apache::lucene::index
{

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortField = org::apache::lucene::search::SortField;

class DocValuesWriter : public std::enable_shared_from_this<DocValuesWriter>
{
  GET_CLASS_NAME(DocValuesWriter)
public:
  virtual void finish(int numDoc) = 0;
  virtual void flush(std::shared_ptr<SegmentWriteState> state,
                     std::shared_ptr<Sorter::DocMap> sortMap,
                     std::shared_ptr<DocValuesConsumer> consumer) = 0;
  virtual std::shared_ptr<Sorter::DocComparator>
  getDocComparator(int numDoc, std::shared_ptr<SortField> sortField) = 0;
  virtual std::shared_ptr<DocIdSetIterator> getDocIdSet() = 0;
};

} // #include  "core/src/java/org/apache/lucene/index/
