#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class StoredFieldsFormat;
}

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
namespace org::apache::lucene::index
{
class MergeState;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class IndexableField;
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
namespace org::apache::lucene::codecs::cranky
{

using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexableField = org::apache::lucene::index::IndexableField;
using MergeState = org::apache::lucene::index::MergeState;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

class CrankyStoredFieldsFormat : public StoredFieldsFormat
{
  GET_CLASS_NAME(CrankyStoredFieldsFormat)
public:
  const std::shared_ptr<StoredFieldsFormat> delegate_;
  const std::shared_ptr<Random> random;

  CrankyStoredFieldsFormat(std::shared_ptr<StoredFieldsFormat> delegate_,
                           std::shared_ptr<Random> random);

  std::shared_ptr<StoredFieldsReader>
  fieldsReader(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si, std::shared_ptr<FieldInfos> fn,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<StoredFieldsWriter>
  fieldsWriter(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si,
               std::shared_ptr<IOContext> context)  override;

public:
  class CrankyStoredFieldsWriter : public StoredFieldsWriter
  {
    GET_CLASS_NAME(CrankyStoredFieldsWriter)

  public:
    const std::shared_ptr<StoredFieldsWriter> delegate_;
    const std::shared_ptr<Random> random;

    CrankyStoredFieldsWriter(std::shared_ptr<StoredFieldsWriter> delegate_,
                             std::shared_ptr<Random> random);

    void finish(std::shared_ptr<FieldInfos> fis,
                int numDocs)  override;

    int
    merge(std::shared_ptr<MergeState> mergeState)  override;

    virtual ~CrankyStoredFieldsWriter();

    // per doc/field methods: lower probability since they are invoked so many
    // times.

    void startDocument()  override;

    void finishDocument()  override;

    void writeField(
        std::shared_ptr<FieldInfo> info,
        std::shared_ptr<IndexableField> field)  override;

  protected:
    std::shared_ptr<CrankyStoredFieldsWriter> shared_from_this()
    {
      return std::static_pointer_cast<CrankyStoredFieldsWriter>(
          org.apache.lucene.codecs.StoredFieldsWriter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CrankyStoredFieldsFormat> shared_from_this()
  {
    return std::static_pointer_cast<CrankyStoredFieldsFormat>(
        org.apache.lucene.codecs.StoredFieldsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::cranky
