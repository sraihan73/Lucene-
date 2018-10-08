#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"

#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"

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
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;

/**
 * A collector that collects all terms from a specified field matching the
 * query.
 *
 * @lucene.experimental
 */
template <typename DV>
class TermsCollector : public DocValuesTermsCollector<DV>
{
  GET_CLASS_NAME(TermsCollector)

public:
  TermsCollector(Function<DV> docValuesCall)
      : DocValuesTermsCollector<DV>(docValuesCall)
  {
  }

  const std::shared_ptr<BytesRefHash> collectorTerms =
      std::make_shared<BytesRefHash>();

  virtual std::shared_ptr<BytesRefHash> getCollectorTerms()
  {
    return collectorTerms;
  }

  /**
   * Chooses the right {@link TermsCollector} implementation.
   *
   * @param field                     The field to collect terms for
   * @param multipleValuesPerDocument Whether the field to collect terms for has
   * multiple values per document.
   * @return a {@link TermsCollector} instance
   */
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: static TermsCollector<?> create(std::wstring field, bool
  // multipleValuesPerDocument)
  static std::shared_ptr < TermsCollector <
      ? >> create(const std::wstring &field, bool multipleValuesPerDocument)
  {
    return multipleValuesPerDocument
               ? std::make_shared<MV>(sortedSetDocValues(field))
               : std::make_shared<SV>(binaryDocValues(field));
  }

  // impl that works with multiple values per document
public:
  class MV : public TermsCollector<std::shared_ptr<SortedSetDocValues>>
  {
    GET_CLASS_NAME(MV)

  public:
    MV(Function<std::shared_ptr<SortedSetDocValues>> docValuesCall);

    void collect(int doc)  override;

  protected:
    std::shared_ptr<MV> shared_from_this()
    {
      return std::static_pointer_cast<MV>(
          TermsCollector<
              org.apache.lucene.index.SortedSetDocValues>::shared_from_this());
    }
  };

  // impl that works with single value per document
public:
  class SV : public TermsCollector<std::shared_ptr<BinaryDocValues>>
  {
    GET_CLASS_NAME(SV)

  public:
    SV(Function<std::shared_ptr<BinaryDocValues>> docValuesCall);

    void collect(int doc)  override;

  protected:
    std::shared_ptr<SV> shared_from_this()
    {
      return std::static_pointer_cast<SV>(
          TermsCollector<
              org.apache.lucene.index.BinaryDocValues>::shared_from_this());
    }
  };

public:
  bool needsScores() override { return false; }

protected:
  std::shared_ptr<TermsCollector> shared_from_this()
  {
    return std::static_pointer_cast<TermsCollector>(
        DocValuesTermsCollector<DV>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
