#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
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

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

class SimpleTextFieldsWriter : public FieldsConsumer
{
  GET_CLASS_NAME(SimpleTextFieldsWriter)

private:
  std::shared_ptr<IndexOutput> out;
  const std::shared_ptr<BytesRefBuilder> scratch =
      std::make_shared<BytesRefBuilder>();
  const std::shared_ptr<SegmentWriteState> writeState;

public:
  const std::wstring segment;

  static const std::shared_ptr<BytesRef> END;
  static const std::shared_ptr<BytesRef> FIELD;
  static const std::shared_ptr<BytesRef> TERM;
  static const std::shared_ptr<BytesRef> DOC;
  static const std::shared_ptr<BytesRef> FREQ;
  static const std::shared_ptr<BytesRef> POS;
  static const std::shared_ptr<BytesRef> START_OFFSET;
  static const std::shared_ptr<BytesRef> END_OFFSET;
  static const std::shared_ptr<BytesRef> PAYLOAD;

  SimpleTextFieldsWriter(std::shared_ptr<SegmentWriteState> writeState) throw(
      IOException);

  void write(std::shared_ptr<Fields> fields)  override;

  virtual void write(std::shared_ptr<FieldInfos> fieldInfos,
                     std::shared_ptr<Fields> fields) ;

private:
  void write(const std::wstring &s) ;

  void write(std::shared_ptr<BytesRef> b) ;

  void newline() ;

public:
  virtual ~SimpleTextFieldsWriter();

protected:
  std::shared_ptr<SimpleTextFieldsWriter> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextFieldsWriter>(
        org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/simpletext/
