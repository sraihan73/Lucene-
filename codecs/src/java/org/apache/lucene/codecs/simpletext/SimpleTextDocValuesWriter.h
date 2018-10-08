#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/DocValuesType.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/EmptyDocValuesProducer.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"

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

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

class SimpleTextDocValuesWriter : public DocValuesConsumer
{
  GET_CLASS_NAME(SimpleTextDocValuesWriter)
public:
  static const std::shared_ptr<BytesRef> END;
  static const std::shared_ptr<BytesRef> FIELD;
  static const std::shared_ptr<BytesRef> TYPE;
  // used for numerics
  static const std::shared_ptr<BytesRef> MINVALUE;
  static const std::shared_ptr<BytesRef> PATTERN;
  // used for bytes
  static const std::shared_ptr<BytesRef> LENGTH;
  static const std::shared_ptr<BytesRef> MAXLENGTH;
  // used for sorted bytes
  static const std::shared_ptr<BytesRef> NUMVALUES;
  static const std::shared_ptr<BytesRef> ORDPATTERN;

  std::shared_ptr<IndexOutput> data;
  const std::shared_ptr<BytesRefBuilder> scratch =
      std::make_shared<BytesRefBuilder>();
  const int numDocs;

private:
  const std::shared_ptr<Set<std::wstring>> fieldsSeen =
      std::unordered_set<std::wstring>(); // for asserting

public:
  SimpleTextDocValuesWriter(std::shared_ptr<SegmentWriteState> state,
                            const std::wstring &ext) ;

  // for asserting
private:
  bool fieldSeen(const std::wstring &field);

public:
  void addNumericField(std::shared_ptr<FieldInfo> field,
                       std::shared_ptr<DocValuesProducer>
                           valuesProducer)  override;

  void addBinaryField(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<DocValuesProducer>
                          valuesProducer)  override;

private:
  void doAddBinaryField(
      std::shared_ptr<FieldInfo> field,
      std::shared_ptr<DocValuesProducer> valuesProducer) ;

public:
  void addSortedField(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<DocValuesProducer>
                          valuesProducer)  override;

  void addSortedNumericField(std::shared_ptr<FieldInfo> field,
                             std::shared_ptr<DocValuesProducer>
                                 valuesProducer)  override;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextDocValuesWriter> outerInstance;

    std::shared_ptr<FieldInfo> field;
    std::shared_ptr<DocValuesProducer> valuesProducer;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<SimpleTextDocValuesWriter> outerInstance,
        std::shared_ptr<FieldInfo> field,
        std::shared_ptr<DocValuesProducer> valuesProducer);

    std::shared_ptr<BinaryDocValues>
    getBinary(std::shared_ptr<FieldInfo> field)  override;

  private:
    class BinaryDocValuesAnonymousInnerClass : public BinaryDocValues
    {
      GET_CLASS_NAME(BinaryDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance;

      std::shared_ptr<SortedNumericDocValues> values;

    public:
      BinaryDocValuesAnonymousInnerClass(
          std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
              outerInstance,
          std::shared_ptr<SortedNumericDocValues> values);

      int nextDoc()  override;

      int docID() override;

      int64_t cost() override;

      int advance(int target)  override;

      bool advanceExact(int target)  override;

      const std::shared_ptr<StringBuilder> builder;
      std::shared_ptr<BytesRef> binaryValue;

    private:
      void setCurrentDoc() ;

    public:
      std::shared_ptr<BytesRef> binaryValue()  override;

    protected:
      std::shared_ptr<BinaryDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<BinaryDocValuesAnonymousInnerClass>(
            org.apache.lucene.index.BinaryDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          org.apache.lucene.index.EmptyDocValuesProducer::shared_from_this());
    }
  };

public:
  void addSortedSetField(std::shared_ptr<FieldInfo> field,
                         std::shared_ptr<DocValuesProducer>
                             valuesProducer)  override;

  /** write the header for this field */
private:
  void writeFieldEntry(std::shared_ptr<FieldInfo> field,
                       DocValuesType type) ;

public:
  virtual ~SimpleTextDocValuesWriter();

protected:
  std::shared_ptr<SimpleTextDocValuesWriter> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextDocValuesWriter>(
        org.apache.lucene.codecs.DocValuesConsumer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/simpletext/
