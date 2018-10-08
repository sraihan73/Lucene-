#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Fields;
}

namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::index
{
class SortedNumericDocValues;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::index
{
class StoredFieldVisitor;
}
namespace org::apache::lucene::index
{
class LeafMetaData;
}

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
namespace org::apache::lucene::search::highlight
{

using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using LeafMetaData = org::apache::lucene::index::LeafMetaData;
using LeafReader = org::apache::lucene::index::LeafReader;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using PointValues = org::apache::lucene::index::PointValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Terms = org::apache::lucene::index::Terms;
using Bits = org::apache::lucene::util::Bits;

/**
 * Wraps a Terms with a {@link org.apache.lucene.index.LeafReader}, typically
 * from term vectors.
 *
 * @lucene.experimental
 */
class TermVectorLeafReader : public LeafReader
{
  GET_CLASS_NAME(TermVectorLeafReader)

private:
  const std::shared_ptr<Fields> fields;
  const std::shared_ptr<FieldInfos> fieldInfos;

public:
  TermVectorLeafReader(const std::wstring &field, std::shared_ptr<Terms> terms);

protected:
  void doClose()  override;

public:
  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  std::shared_ptr<NumericDocValues>
  getNumericDocValues(const std::wstring &field)  override;

  std::shared_ptr<BinaryDocValues>
  getBinaryDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedDocValues>
  getSortedDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedNumericDocValues> getSortedNumericDocValues(
      const std::wstring &field)  override;

  std::shared_ptr<SortedSetDocValues>
  getSortedSetDocValues(const std::wstring &field)  override;

  std::shared_ptr<NumericDocValues>
  getNormValues(const std::wstring &field)  override;

  std::shared_ptr<FieldInfos> getFieldInfos() override;

  std::shared_ptr<Bits> getLiveDocs() override;

  std::shared_ptr<PointValues>
  getPointValues(const std::wstring &fieldName) override;

  void checkIntegrity()  override;

  std::shared_ptr<Fields> getTermVectors(int docID)  override;

  int numDocs() override;

  int maxDoc() override;

  void document(int docID, std::shared_ptr<StoredFieldVisitor> visitor) throw(
      IOException) override;

  std::shared_ptr<LeafMetaData> getMetaData() override;

  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  std::shared_ptr<TermVectorLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<TermVectorLeafReader>(
        org.apache.lucene.index.LeafReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::highlight
