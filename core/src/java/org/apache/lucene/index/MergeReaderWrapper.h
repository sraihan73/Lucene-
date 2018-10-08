#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class CodecReader;
}

namespace org::apache::lucene::codecs
{
class FieldsProducer;
}
namespace org::apache::lucene::codecs
{
class NormsProducer;
}
namespace org::apache::lucene::codecs
{
class DocValuesProducer;
}
namespace org::apache::lucene::codecs
{
class StoredFieldsReader;
}
namespace org::apache::lucene::codecs
{
class TermVectorsReader;
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
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::index
{
class Fields;
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

namespace org::apache::lucene::index
{

using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Bits = org::apache::lucene::util::Bits;

/** This is a hack to make index sorting fast, with a {@link LeafReader} that
 * always returns merge instances when you ask for the codec readers. */
class MergeReaderWrapper : public LeafReader
{
  GET_CLASS_NAME(MergeReaderWrapper)
public:
  const std::shared_ptr<CodecReader> in_;
  const std::shared_ptr<FieldsProducer> fields;
  const std::shared_ptr<NormsProducer> norms;
  const std::shared_ptr<DocValuesProducer> docValues;
  const std::shared_ptr<StoredFieldsReader> store;
  const std::shared_ptr<TermVectorsReader> vectors;

  MergeReaderWrapper(std::shared_ptr<CodecReader> in_) ;

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

  void checkIntegrity()  override;

  std::shared_ptr<Fields> getTermVectors(int docID)  override;

  std::shared_ptr<PointValues>
  getPointValues(const std::wstring &fieldName)  override;

  int numDocs() override;

  int maxDoc() override;

  void document(int docID, std::shared_ptr<StoredFieldVisitor> visitor) throw(
      IOException) override;

protected:
  void doClose()  override;

public:
  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

private:
  void checkBounds(int docID);

public:
  virtual std::wstring toString();

  std::shared_ptr<LeafMetaData> getMetaData() override;

protected:
  std::shared_ptr<MergeReaderWrapper> shared_from_this()
  {
    return std::static_pointer_cast<MergeReaderWrapper>(
        LeafReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
