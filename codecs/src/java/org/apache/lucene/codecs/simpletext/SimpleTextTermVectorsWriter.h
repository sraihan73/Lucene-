#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/TermVectorsWriter.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"

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

using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * Writes plain-text term vectors.
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 * @lucene.experimental
 */
class SimpleTextTermVectorsWriter : public TermVectorsWriter
{
  GET_CLASS_NAME(SimpleTextTermVectorsWriter)

public:
  static const std::shared_ptr<BytesRef> END;
  static const std::shared_ptr<BytesRef> DOC;
  static const std::shared_ptr<BytesRef> NUMFIELDS;
  static const std::shared_ptr<BytesRef> FIELD;
  static const std::shared_ptr<BytesRef> FIELDNAME;
  static const std::shared_ptr<BytesRef> FIELDPOSITIONS;
  static const std::shared_ptr<BytesRef> FIELDOFFSETS;
  static const std::shared_ptr<BytesRef> FIELDPAYLOADS;
  static const std::shared_ptr<BytesRef> FIELDTERMCOUNT;
  static const std::shared_ptr<BytesRef> TERMTEXT;
  static const std::shared_ptr<BytesRef> TERMFREQ;
  static const std::shared_ptr<BytesRef> POSITION;
  static const std::shared_ptr<BytesRef> PAYLOAD;
  static const std::shared_ptr<BytesRef> STARTOFFSET;
  static const std::shared_ptr<BytesRef> ENDOFFSET;

  static const std::wstring VECTORS_EXTENSION;

private:
  const std::shared_ptr<Directory> directory;
  const std::wstring segment;
  std::shared_ptr<IndexOutput> out;
  int numDocsWritten = 0;
  const std::shared_ptr<BytesRefBuilder> scratch =
      std::make_shared<BytesRefBuilder>();
  bool offsets = false;
  bool positions = false;
  bool payloads = false;

public:
  SimpleTextTermVectorsWriter(
      std::shared_ptr<Directory> directory, const std::wstring &segment,
      std::shared_ptr<IOContext> context) ;

  void startDocument(int numVectorFields)  override;

  void startField(std::shared_ptr<FieldInfo> info, int numTerms, bool positions,
                  bool offsets, bool payloads)  override;

  void startTerm(std::shared_ptr<BytesRef> term,
                 int freq)  override;

  void
  addPosition(int position, int startOffset, int endOffset,
              std::shared_ptr<BytesRef> payload)  override;

  void finish(std::shared_ptr<FieldInfos> fis,
              int numDocs)  override;

  virtual ~SimpleTextTermVectorsWriter();

private:
  void write(const std::wstring &s) ;

  void write(std::shared_ptr<BytesRef> bytes) ;

  void newLine() ;

protected:
  std::shared_ptr<SimpleTextTermVectorsWriter> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextTermVectorsWriter>(
        org.apache.lucene.codecs.TermVectorsWriter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/simpletext/
