#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"

#include  "core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/perfield/MergeRecordingDocValueFormatWrapper.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/MergeState.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"

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
namespace org::apache::lucene::codecs::perfield
{

using Codec = org::apache::lucene::codecs::Codec;
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using BaseDocValuesFormatTestCase =
    org::apache::lucene::index::BaseDocValuesFormatTestCase;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/**
 * Basic tests of PerFieldDocValuesFormat
 */
class TestPerFieldDocValuesFormat : public BaseDocValuesFormatTestCase
{
  GET_CLASS_NAME(TestPerFieldDocValuesFormat)
private:
  std::shared_ptr<Codec> codec;

public:
  void setUp()  override;

protected:
  std::shared_ptr<Codec> getCodec() override;

  bool codecAcceptsHugeBinaryValues(const std::wstring &field) override;

  // just a simple trivial test
  // TODO: we should come up with a test that somehow checks that segment suffix
  // is respected by all codec apis (not just docvalues and postings)
public:
  virtual void testTwoFieldsTwoFormats() ;

private:
  class AssertingCodecAnonymousInnerClass : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass)
  private:
    std::shared_ptr<TestPerFieldDocValuesFormat> outerInstance;

    std::shared_ptr<DocValuesFormat> fast;
    std::shared_ptr<DocValuesFormat> slow;

  public:
    AssertingCodecAnonymousInnerClass(
        std::shared_ptr<TestPerFieldDocValuesFormat> outerInstance,
        std::shared_ptr<DocValuesFormat> fast,
        std::shared_ptr<DocValuesFormat> slow);

    std::shared_ptr<DocValuesFormat>
    getDocValuesFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

public:
  virtual void testMergeCalledOnTwoFormats() ;

private:
  class AssertingCodecAnonymousInnerClass2 : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass2)
  private:
    std::shared_ptr<TestPerFieldDocValuesFormat> outerInstance;

    std::shared_ptr<
        org::apache::lucene::codecs::perfield::TestPerFieldDocValuesFormat::
            MergeRecordingDocValueFormatWrapper>
        dvf1;
    std::shared_ptr<
        org::apache::lucene::codecs::perfield::TestPerFieldDocValuesFormat::
            MergeRecordingDocValueFormatWrapper>
        dvf2;

  public:
    AssertingCodecAnonymousInnerClass2(
        std::shared_ptr<TestPerFieldDocValuesFormat> outerInstance,
        std::shared_ptr<
            org::apache::lucene::codecs::perfield::TestPerFieldDocValuesFormat::
                MergeRecordingDocValueFormatWrapper>
            dvf1,
        std::shared_ptr<
            org::apache::lucene::codecs::perfield::TestPerFieldDocValuesFormat::
                MergeRecordingDocValueFormatWrapper>
            dvf2);

    std::shared_ptr<DocValuesFormat>
    getDocValuesFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass2>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

private:
  class MergeRecordingDocValueFormatWrapper final : public DocValuesFormat
  {
    GET_CLASS_NAME(MergeRecordingDocValueFormatWrapper)
  private:
    const std::shared_ptr<DocValuesFormat> delegate_;

  public:
    const std::deque<std::wstring> fieldNames = std::deque<std::wstring>();
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile int nbMergeCalls = 0;
    int nbMergeCalls = 0;

    MergeRecordingDocValueFormatWrapper(
        std::shared_ptr<DocValuesFormat> delegate_);

    std::shared_ptr<DocValuesConsumer> fieldsConsumer(
        std::shared_ptr<SegmentWriteState> state)  override;

  private:
    class DocValuesConsumerAnonymousInnerClass : public DocValuesConsumer
    {
      GET_CLASS_NAME(DocValuesConsumerAnonymousInnerClass)
    private:
      std::shared_ptr<MergeRecordingDocValueFormatWrapper> outerInstance;

      std::shared_ptr<DocValuesConsumer> consumer;

    public:
      DocValuesConsumerAnonymousInnerClass(
          std::shared_ptr<MergeRecordingDocValueFormatWrapper> outerInstance,
          std::shared_ptr<DocValuesConsumer> consumer);

      void addNumericField(std::shared_ptr<FieldInfo> field,
                           std::shared_ptr<DocValuesProducer>
                               values)  override;

      void addBinaryField(std::shared_ptr<FieldInfo> field,
                          std::shared_ptr<DocValuesProducer>
                              values)  override;

      void addSortedField(std::shared_ptr<FieldInfo> field,
                          std::shared_ptr<DocValuesProducer>
                              values)  override;

      void addSortedNumericField(std::shared_ptr<FieldInfo> field,
                                 std::shared_ptr<DocValuesProducer>
                                     values)  override;

      void addSortedSetField(std::shared_ptr<FieldInfo> field,
                             std::shared_ptr<DocValuesProducer>
                                 values)  override;

      void
      merge(std::shared_ptr<MergeState> mergeState)  override;

      virtual ~DocValuesConsumerAnonymousInnerClass();

    protected:
      std::shared_ptr<DocValuesConsumerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DocValuesConsumerAnonymousInnerClass>(
            org.apache.lucene.codecs.DocValuesConsumer::shared_from_this());
      }
    };

  public:
    std::shared_ptr<DocValuesProducer> fieldsProducer(
        std::shared_ptr<SegmentReadState> state)  override;

  protected:
    std::shared_ptr<MergeRecordingDocValueFormatWrapper> shared_from_this()
    {
      return std::static_pointer_cast<MergeRecordingDocValueFormatWrapper>(
          org.apache.lucene.codecs.DocValuesFormat::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestPerFieldDocValuesFormat> shared_from_this()
  {
    return std::static_pointer_cast<TestPerFieldDocValuesFormat>(
        org.apache.lucene.index
            .BaseDocValuesFormatTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/perfield/
