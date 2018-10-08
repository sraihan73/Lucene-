#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/StoredFieldsFormat.h"

#include  "core/src/java/org/apache/lucene/codecs/StoredFieldsReader.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/codecs/StoredFieldsWriter.h"
#include  "core/src/java/org/apache/lucene/index/StoredFieldVisitor.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/IndexableField.h"

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
namespace org::apache::lucene::codecs::asserting
{

using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexableField = org::apache::lucene::index::IndexableField;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Accountable = org::apache::lucene::util::Accountable;
using TestUtil = org::apache::lucene::util::TestUtil;

/**
 * Just like the default stored fields format but with additional asserts.
 */
class AssertingStoredFieldsFormat : public StoredFieldsFormat
{
  GET_CLASS_NAME(AssertingStoredFieldsFormat)
private:
  const std::shared_ptr<StoredFieldsFormat> in_ =
      TestUtil::getDefaultCodec()->storedFieldsFormat();

public:
  std::shared_ptr<StoredFieldsReader>
  fieldsReader(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si, std::shared_ptr<FieldInfos> fn,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<StoredFieldsWriter>
  fieldsWriter(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si,
               std::shared_ptr<IOContext> context)  override;

public:
  class AssertingStoredFieldsReader : public StoredFieldsReader
  {
    GET_CLASS_NAME(AssertingStoredFieldsReader)
  private:
    const std::shared_ptr<StoredFieldsReader> in_;
    const int maxDoc;

  public:
    AssertingStoredFieldsReader(std::shared_ptr<StoredFieldsReader> in_,
                                int maxDoc);

    virtual ~AssertingStoredFieldsReader();

    void
    visitDocument(int n, std::shared_ptr<StoredFieldVisitor> visitor) throw(
        IOException) override;

    std::shared_ptr<StoredFieldsReader> clone() override;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

    std::shared_ptr<StoredFieldsReader>
    getMergeInstance()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<AssertingStoredFieldsReader> shared_from_this()
    {
      return std::static_pointer_cast<AssertingStoredFieldsReader>(
          org.apache.lucene.codecs.StoredFieldsReader::shared_from_this());
    }
  };

public:
  enum class Status { GET_CLASS_NAME(Status) UNDEFINED, STARTED, FINISHED };

public:
  class AssertingStoredFieldsWriter : public StoredFieldsWriter
  {
    GET_CLASS_NAME(AssertingStoredFieldsWriter)
  private:
    const std::shared_ptr<StoredFieldsWriter> in_;
    int numWritten = 0;
    Status docStatus = static_cast<Status>(0);

  public:
    AssertingStoredFieldsWriter(std::shared_ptr<StoredFieldsWriter> in_);

    void startDocument()  override;

    void finishDocument()  override;

    void writeField(
        std::shared_ptr<FieldInfo> info,
        std::shared_ptr<IndexableField> field)  override;

    void finish(std::shared_ptr<FieldInfos> fis,
                int numDocs)  override;

    virtual ~AssertingStoredFieldsWriter();

  protected:
    std::shared_ptr<AssertingStoredFieldsWriter> shared_from_this()
    {
      return std::static_pointer_cast<AssertingStoredFieldsWriter>(
          org.apache.lucene.codecs.StoredFieldsWriter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<AssertingStoredFieldsFormat> shared_from_this()
  {
    return std::static_pointer_cast<AssertingStoredFieldsFormat>(
        org.apache.lucene.codecs.StoredFieldsFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/asserting/
