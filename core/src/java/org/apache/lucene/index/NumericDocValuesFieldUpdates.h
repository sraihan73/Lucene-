#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/packed/PagedGrowableWriter.h"

#include  "core/src/java/org/apache/lucene/util/packed/PagedMutable.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/DocValuesFieldUpdates.h"
#include  "core/src/java/org/apache/lucene/index/Iterator.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;
using PagedGrowableWriter =
    org::apache::lucene::util::packed::PagedGrowableWriter;
using PagedMutable = org::apache::lucene::util::packed::PagedMutable;

/**
 * A {@link DocValuesFieldUpdates} which holds updates of documents, of a single
 * {@link NumericDocValuesField}.
 *
 * @lucene.experimental
 */
class NumericDocValuesFieldUpdates final : public DocValuesFieldUpdates
{
  GET_CLASS_NAME(NumericDocValuesFieldUpdates)

  // TODO: can't this just be NumericDocValues now?  avoid boxing the long
  // value...
public:
  class Iterator final : public DocValuesFieldUpdates::AbstractIterator
  {
    GET_CLASS_NAME(Iterator)
  private:
    const std::shared_ptr<PagedGrowableWriter> values;
    int64_t value = 0;

  public:
    Iterator(int size, std::shared_ptr<PagedGrowableWriter> values,
             std::shared_ptr<PagedMutable> docs, int64_t delGen);
    int64_t longValue() override;

    std::shared_ptr<BytesRef> binaryValue() override;

  protected:
    void set(int64_t idx) override;

  protected:
    std::shared_ptr<Iterator> shared_from_this()
    {
      return std::static_pointer_cast<Iterator>(
          DocValuesFieldUpdates.AbstractIterator::shared_from_this());
    }
  };

private:
  std::shared_ptr<PagedGrowableWriter> values;

public:
  NumericDocValuesFieldUpdates(int64_t delGen, const std::wstring &field,
                               int maxDoc);
  void add(int doc, std::shared_ptr<BytesRef> value) override;

  void add(int docId,
           std::shared_ptr<DocValuesFieldUpdates::Iterator> iterator) override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void add(int doc, int64_t value) override;

protected:
  void swap(int i, int j) override;

  void grow(int size) override;

  void resize(int size) override;

public:
  std::shared_ptr<Iterator> iterator() override;

  int64_t ramBytesUsed() override;

protected:
  std::shared_ptr<NumericDocValuesFieldUpdates> shared_from_this()
  {
    return std::static_pointer_cast<NumericDocValuesFieldUpdates>(
        DocValuesFieldUpdates::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
