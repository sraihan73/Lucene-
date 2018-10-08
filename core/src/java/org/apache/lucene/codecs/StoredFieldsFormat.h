#pragma once
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
namespace org::apache::lucene::codecs
{

using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/**
 * Controls the format of stored fields
 */
class StoredFieldsFormat
    : public std::enable_shared_from_this<StoredFieldsFormat>
{
  GET_CLASS_NAME(StoredFieldsFormat)
  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  StoredFieldsFormat();

  /** Returns a {@link StoredFieldsReader} to load stored
   *  fields. */
public:
  virtual std::shared_ptr<StoredFieldsReader>
  fieldsReader(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si, std::shared_ptr<FieldInfos> fn,
               std::shared_ptr<IOContext> context) = 0;

  /** Returns a {@link StoredFieldsWriter} to write stored
   *  fields. */
  virtual std::shared_ptr<StoredFieldsWriter>
  fieldsWriter(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si,
               std::shared_ptr<IOContext> context) = 0;
};

} // namespace org::apache::lucene::codecs
