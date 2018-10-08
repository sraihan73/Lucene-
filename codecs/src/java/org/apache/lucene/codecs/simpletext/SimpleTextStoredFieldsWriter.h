#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/StoredFieldsWriter.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class IndexableField;
}
namespace org::apache::lucene::index
{
class FieldInfos;
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

using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexableField = org::apache::lucene::index::IndexableField;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * Writes plain-text stored fields.
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 * @lucene.experimental
 */
class SimpleTextStoredFieldsWriter : public StoredFieldsWriter
{
  GET_CLASS_NAME(SimpleTextStoredFieldsWriter)
private:
  int numDocsWritten = 0;
  const std::shared_ptr<Directory> directory;
  const std::wstring segment;
  std::shared_ptr<IndexOutput> out;

public:
  static const std::wstring FIELDS_EXTENSION;

  static const std::shared_ptr<BytesRef> TYPE_STRING;
  static const std::shared_ptr<BytesRef> TYPE_BINARY;
  static const std::shared_ptr<BytesRef> TYPE_INT;
  static const std::shared_ptr<BytesRef> TYPE_LONG;
  static const std::shared_ptr<BytesRef> TYPE_FLOAT;
  static const std::shared_ptr<BytesRef> TYPE_DOUBLE;

  static const std::shared_ptr<BytesRef> END;
  static const std::shared_ptr<BytesRef> DOC;
  static const std::shared_ptr<BytesRef> FIELD;
  static const std::shared_ptr<BytesRef> NAME;
  static const std::shared_ptr<BytesRef> TYPE;
  static const std::shared_ptr<BytesRef> VALUE;

private:
  const std::shared_ptr<BytesRefBuilder> scratch =
      std::make_shared<BytesRefBuilder>();

public:
  SimpleTextStoredFieldsWriter(
      std::shared_ptr<Directory> directory, const std::wstring &segment,
      std::shared_ptr<IOContext> context) ;

  void startDocument()  override;

  void
  writeField(std::shared_ptr<FieldInfo> info,
             std::shared_ptr<IndexableField> field)  override;

  void finish(std::shared_ptr<FieldInfos> fis,
              int numDocs)  override;

  virtual ~SimpleTextStoredFieldsWriter();

private:
  void write(const std::wstring &s) ;

  void write(std::shared_ptr<BytesRef> bytes) ;

  void newLine() ;

protected:
  std::shared_ptr<SimpleTextStoredFieldsWriter> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextStoredFieldsWriter>(
        org.apache.lucene.codecs.StoredFieldsWriter::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
