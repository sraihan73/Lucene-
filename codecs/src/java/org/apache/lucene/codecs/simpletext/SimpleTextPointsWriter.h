#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PointsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PointValues.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/codecs/simpletext/SimpleTextBKDWriter.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"

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

using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

class SimpleTextPointsWriter : public PointsWriter
{
  GET_CLASS_NAME(SimpleTextPointsWriter)

public:
  static const std::shared_ptr<BytesRef> NUM_DIMS;
  static const std::shared_ptr<BytesRef> BYTES_PER_DIM;
  static const std::shared_ptr<BytesRef> MAX_LEAF_POINTS;
  static const std::shared_ptr<BytesRef> INDEX_COUNT;
  static const std::shared_ptr<BytesRef> BLOCK_COUNT;
  static const std::shared_ptr<BytesRef> BLOCK_DOC_ID;
  static const std::shared_ptr<BytesRef> BLOCK_FP;
  static const std::shared_ptr<BytesRef> BLOCK_VALUE;
  static const std::shared_ptr<BytesRef> SPLIT_COUNT;
  static const std::shared_ptr<BytesRef> SPLIT_DIM;
  static const std::shared_ptr<BytesRef> SPLIT_VALUE;
  static const std::shared_ptr<BytesRef> FIELD_COUNT;
  static const std::shared_ptr<BytesRef> FIELD_FP_NAME;
  static const std::shared_ptr<BytesRef> FIELD_FP;
  static const std::shared_ptr<BytesRef> MIN_VALUE;
  static const std::shared_ptr<BytesRef> MAX_VALUE;
  static const std::shared_ptr<BytesRef> POINT_COUNT;
  static const std::shared_ptr<BytesRef> DOC_COUNT;
  static const std::shared_ptr<BytesRef> END;

private:
  std::shared_ptr<IndexOutput> dataOut;

public:
  const std::shared_ptr<BytesRefBuilder> scratch =
      std::make_shared<BytesRefBuilder>();
  const std::shared_ptr<SegmentWriteState> writeState;
  const std::unordered_map<std::wstring, int64_t> indexFPs =
      std::unordered_map<std::wstring, int64_t>();

  SimpleTextPointsWriter(std::shared_ptr<SegmentWriteState> writeState) throw(
      IOException);

  void
  writeField(std::shared_ptr<FieldInfo> fieldInfo,
             std::shared_ptr<PointsReader> reader)  override;

private:
  class IntersectVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextPointsWriter> outerInstance;

    std::shared_ptr<
        org::apache::lucene::codecs::simpletext::SimpleTextBKDWriter>
        writer;

  public:
    IntersectVisitorAnonymousInnerClass(
        std::shared_ptr<SimpleTextPointsWriter> outerInstance,
        std::shared_ptr<
            org::apache::lucene::codecs::simpletext::SimpleTextBKDWriter>
            writer);

    void visit(int docID) override;

    void visit(int docID,
               std::deque<char> &packedValue)  override;

    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue);
  };

public:
  void finish()  override;

  virtual ~SimpleTextPointsWriter();

private:
  void write(std::shared_ptr<IndexOutput> out,
             const std::wstring &s) ;

  void writeInt(std::shared_ptr<IndexOutput> out, int x) ;

  void writeLong(std::shared_ptr<IndexOutput> out,
                 int64_t x) ;

  void write(std::shared_ptr<IndexOutput> out,
             std::shared_ptr<BytesRef> b) ;

  void newline(std::shared_ptr<IndexOutput> out) ;

protected:
  std::shared_ptr<SimpleTextPointsWriter> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextPointsWriter>(
        org.apache.lucene.codecs.PointsWriter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/simpletext/
