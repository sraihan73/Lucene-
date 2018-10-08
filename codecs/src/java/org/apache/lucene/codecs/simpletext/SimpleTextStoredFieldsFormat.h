#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/StoredFieldsFormat.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class StoredFieldsReader;
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
namespace org::apache::lucene::codecs
{
class StoredFieldsWriter;
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

using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/**
 * plain text stored fields format.
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 * @lucene.experimental
 */
class SimpleTextStoredFieldsFormat : public StoredFieldsFormat
{
  GET_CLASS_NAME(SimpleTextStoredFieldsFormat)

public:
  std::shared_ptr<StoredFieldsReader>
  fieldsReader(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si, std::shared_ptr<FieldInfos> fn,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<StoredFieldsWriter>
  fieldsWriter(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si,
               std::shared_ptr<IOContext> context)  override;

protected:
  std::shared_ptr<SimpleTextStoredFieldsFormat> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextStoredFieldsFormat>(
        org.apache.lucene.codecs.StoredFieldsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
