#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldInfosFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/DocValuesType.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class SegmentInfo;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
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
namespace org::apache::lucene::codecs::simpletext
{

using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * plaintext field infos format
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 * @lucene.experimental
 */
class SimpleTextFieldInfosFormat : public FieldInfosFormat
{
  GET_CLASS_NAME(SimpleTextFieldInfosFormat)

  /** Extension of field infos */
public:
  static const std::wstring FIELD_INFOS_EXTENSION;

  static const std::shared_ptr<BytesRef> NUMFIELDS;
  static const std::shared_ptr<BytesRef> NAME;
  static const std::shared_ptr<BytesRef> NUMBER;
  static const std::shared_ptr<BytesRef> STORETV;
  static const std::shared_ptr<BytesRef> STORETVPOS;
  static const std::shared_ptr<BytesRef> STORETVOFF;
  static const std::shared_ptr<BytesRef> PAYLOADS;
  static const std::shared_ptr<BytesRef> NORMS;
  static const std::shared_ptr<BytesRef> DOCVALUES;
  static const std::shared_ptr<BytesRef> DOCVALUES_GEN;
  static const std::shared_ptr<BytesRef> INDEXOPTIONS;
  static const std::shared_ptr<BytesRef> NUM_ATTS;
  static const std::shared_ptr<BytesRef> ATT_KEY;
  static const std::shared_ptr<BytesRef> ATT_VALUE;
  static const std::shared_ptr<BytesRef> DIM_COUNT;
  static const std::shared_ptr<BytesRef> DIM_NUM_BYTES;
  static const std::shared_ptr<BytesRef> SOFT_DELETES;

  std::shared_ptr<FieldInfos>
  read(std::shared_ptr<Directory> directory,
       std::shared_ptr<SegmentInfo> segmentInfo,
       const std::wstring &segmentSuffix,
       std::shared_ptr<IOContext> iocontext)  override;

  virtual DocValuesType docValuesType(const std::wstring &dvType);

private:
  std::wstring readString(int offset, std::shared_ptr<BytesRefBuilder> scratch);

public:
  void write(std::shared_ptr<Directory> directory,
             std::shared_ptr<SegmentInfo> segmentInfo,
             const std::wstring &segmentSuffix,
             std::shared_ptr<FieldInfos> infos,
             std::shared_ptr<IOContext> context)  override;

private:
  static std::wstring getDocValuesType(DocValuesType type);

protected:
  std::shared_ptr<SimpleTextFieldInfosFormat> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextFieldInfosFormat>(
        org.apache.lucene.codecs.FieldInfosFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
