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
namespace org::apache::lucene::search::join
{

using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;

template <typename DV>
class DocValuesTermsCollector : public SimpleCollector
{
  GET_CLASS_NAME(DocValuesTermsCollector)

  template <typename R>
  using Function = std::function<R(LeafReader t)>;

protected:
  DV docValues;

private:
  const Function<DV> docValuesCall;

public:
  DocValuesTermsCollector(Function<DV> docValuesCall)
      : docValuesCall(docValuesCall)
  {
  }

protected:
  void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
      IOException) override final
  {
    docValues = docValuesCall(context->reader());
  }

public:
  static Function<std::shared_ptr<BinaryDocValues>>
  binaryDocValues(const std::wstring &field)
  {
    return [&](ctx) {
      org::apache::lucene::index::DocValues::getBinary(ctx, field);
    };
  }

  static Function<std::shared_ptr<SortedSetDocValues>>
  sortedSetDocValues(const std::wstring &field)
  {
    return [&](ctx) {
      org::apache::lucene::index::DocValues::getSortedSet(ctx, field);
    };
  }

protected:
  std::shared_ptr<DocValuesTermsCollector> shared_from_this()
  {
    return std::static_pointer_cast<DocValuesTermsCollector>(
        org.apache.lucene.search.SimpleCollector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
