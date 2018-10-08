#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class CodecReader;
}

namespace org::apache::lucene::codecs
{
class StoredFieldsReader;
}
namespace org::apache::lucene::codecs
{
class TermVectorsReader;
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
class FieldsProducer;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::codecs
{
class PointsReader;
}
namespace org::apache::lucene::index
{
class LeafMetaData;
}
namespace org::apache::lucene::util
{
class Accountable;
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
using PointsReader = org::apache::lucene::codecs::PointsReader;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Accountable = org::apache::lucene::util::Accountable;
using Bits = org::apache::lucene::util::Bits;

/**
 * A <code>FilterCodecReader</code> contains another CodecReader, which it
 * uses as its basic source of data, possibly transforming the data along the
 * way or providing additional functionality.
 * <p><b>NOTE</b>: If this {@link FilterCodecReader} does not change the
 * content the contained reader, you could consider delegating calls to
 * {@link #getCoreCacheHelper()} and {@link #getReaderCacheHelper()}.
 */
class FilterCodecReader : public CodecReader
{
  GET_CLASS_NAME(FilterCodecReader)
  /**
   * The underlying CodecReader instance.
   */
protected:
  const std::shared_ptr<CodecReader> in_;

  /**
   * Creates a new FilterCodecReader.
   * @param in the underlying CodecReader instance.
   */
public:
  FilterCodecReader(std::shared_ptr<CodecReader> in_);

  std::shared_ptr<StoredFieldsReader> getFieldsReader() override;

  std::shared_ptr<TermVectorsReader> getTermVectorsReader() override;

  std::shared_ptr<NormsProducer> getNormsReader() override;

  std::shared_ptr<DocValuesProducer> getDocValuesReader() override;

  std::shared_ptr<FieldsProducer> getPostingsReader() override;

  std::shared_ptr<Bits> getLiveDocs() override;

  std::shared_ptr<FieldInfos> getFieldInfos() override;

  std::shared_ptr<PointsReader> getPointsReader() override;

  int numDocs() override;

  int maxDoc() override;

  std::shared_ptr<LeafMetaData> getMetaData() override;

protected:
  void doClose()  override;

public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  void checkIntegrity()  override;

protected:
  std::shared_ptr<FilterCodecReader> shared_from_this()
  {
    return std::static_pointer_cast<FilterCodecReader>(
        CodecReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
