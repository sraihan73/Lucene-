#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

#include  "core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"
#include  "core/src/java/org/apache/lucene/codecs/perfield/MergeRecordingPostingsFormatWrapper.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/MergeState.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
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
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using DirectPostingsFormat =
    org::apache::lucene::codecs::memory::DirectPostingsFormat;
using MemoryPostingsFormat =
    org::apache::lucene::codecs::memory::MemoryPostingsFormat;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

/**
 *
 *
 */
// TODO: would be better in this test to pull termsenums and instanceof or
// something?
// this way we can verify PFPF is doing the right thing.
// for now we do termqueries.
class TestPerFieldPostingsFormat2 : public LuceneTestCase
{
  GET_CLASS_NAME(TestPerFieldPostingsFormat2)

private:
  std::shared_ptr<IndexWriter>
  newWriter(std::shared_ptr<Directory> dir,
            std::shared_ptr<IndexWriterConfig> conf) ;

  void addDocs(std::shared_ptr<IndexWriter> writer,
               int numDocs) ;

  void addDocs2(std::shared_ptr<IndexWriter> writer,
                int numDocs) ;

  void addDocs3(std::shared_ptr<IndexWriter> writer,
                int numDocs) ;

  /*
   * Test that heterogeneous index segments are merge successfully
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMergeUnusedPerFieldCodec() throws
  // java.io.IOException
  virtual void testMergeUnusedPerFieldCodec() ;

  /*
   * Test that heterogeneous index segments are merged sucessfully
   */
  // TODO: not sure this test is that great, we should probably peek inside
  // PerFieldPostingsFormat or something?!
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testChangeCodecAndMerge() throws
  // java.io.IOException
  virtual void testChangeCodecAndMerge() ;

  virtual void assertQuery(std::shared_ptr<Term> t,
                           std::shared_ptr<Directory> dir,
                           int num) ;

public:
  class MockCodec : public AssertingCodec
  {
    GET_CLASS_NAME(MockCodec)
  public:
    const std::shared_ptr<PostingsFormat> luceneDefault =
        TestUtil::getDefaultPostingsFormat();
    const std::shared_ptr<PostingsFormat> direct =
        std::make_shared<DirectPostingsFormat>();
    const std::shared_ptr<PostingsFormat> memory =
        std::make_shared<MemoryPostingsFormat>();

    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<MockCodec> shared_from_this()
    {
      return std::static_pointer_cast<MockCodec>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

public:
  class MockCodec2 : public AssertingCodec
  {
    GET_CLASS_NAME(MockCodec2)
  public:
    const std::shared_ptr<PostingsFormat> luceneDefault =
        TestUtil::getDefaultPostingsFormat();
    const std::shared_ptr<PostingsFormat> direct =
        std::make_shared<DirectPostingsFormat>();

    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<MockCodec2> shared_from_this()
    {
      return std::static_pointer_cast<MockCodec2>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

  /*
   * Test per field codec support - adding fields with random codecs
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testStressPerFieldCodec() throws
  // java.io.IOException
  virtual void testStressPerFieldCodec() ;

  virtual void testSameCodecDifferentInstance() ;

private:
  class AssertingCodecAnonymousInnerClass : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass)
  private:
    std::shared_ptr<TestPerFieldPostingsFormat2> outerInstance;

  public:
    AssertingCodecAnonymousInnerClass(
        std::shared_ptr<TestPerFieldPostingsFormat2> outerInstance);

    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

public:
  virtual void testSameCodecDifferentParams() ;

private:
  class AssertingCodecAnonymousInnerClass2 : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass2)
  private:
    std::shared_ptr<TestPerFieldPostingsFormat2> outerInstance;

  public:
    AssertingCodecAnonymousInnerClass2(
        std::shared_ptr<TestPerFieldPostingsFormat2> outerInstance);

    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass2>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

private:
  void
  doTestMixedPostings(std::shared_ptr<Codec> codec) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("deprecation") public void
  // testMergeCalledOnTwoFormats() throws java.io.IOException
  virtual void testMergeCalledOnTwoFormats() ;

private:
  class AssertingCodecAnonymousInnerClass3 : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass3)
  private:
    std::shared_ptr<TestPerFieldPostingsFormat2> outerInstance;

    std::shared_ptr<
        org::apache::lucene::codecs::perfield::TestPerFieldPostingsFormat2::
            MergeRecordingPostingsFormatWrapper>
        pf1;
    std::shared_ptr<
        org::apache::lucene::codecs::perfield::TestPerFieldPostingsFormat2::
            MergeRecordingPostingsFormatWrapper>
        pf2;

  public:
    AssertingCodecAnonymousInnerClass3(
        std::shared_ptr<TestPerFieldPostingsFormat2> outerInstance,
        std::shared_ptr<
            org::apache::lucene::codecs::perfield::TestPerFieldPostingsFormat2::
                MergeRecordingPostingsFormatWrapper>
            pf1,
        std::shared_ptr<
            org::apache::lucene::codecs::perfield::TestPerFieldPostingsFormat2::
                MergeRecordingPostingsFormatWrapper>
            pf2);

    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass3>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

private:
  class MergeRecordingPostingsFormatWrapper final : public PostingsFormat
  {
    GET_CLASS_NAME(MergeRecordingPostingsFormatWrapper)
  private:
    const std::shared_ptr<PostingsFormat> delegate_;

  public:
    const std::deque<std::wstring> fieldNames = std::deque<std::wstring>();
    int nbMergeCalls = 0;

    MergeRecordingPostingsFormatWrapper(
        std::shared_ptr<PostingsFormat> delegate_);

    std::shared_ptr<FieldsConsumer> fieldsConsumer(
        std::shared_ptr<SegmentWriteState> state)  override;

  private:
    class FieldsConsumerAnonymousInnerClass : public FieldsConsumer
    {
      GET_CLASS_NAME(FieldsConsumerAnonymousInnerClass)
    private:
      std::shared_ptr<MergeRecordingPostingsFormatWrapper> outerInstance;

      std::shared_ptr<FieldsConsumer> consumer;

    public:
      FieldsConsumerAnonymousInnerClass(
          std::shared_ptr<MergeRecordingPostingsFormatWrapper> outerInstance,
          std::shared_ptr<FieldsConsumer> consumer);

      void write(std::shared_ptr<Fields> fields)  override;

      void
      merge(std::shared_ptr<MergeState> mergeState)  override;

      virtual ~FieldsConsumerAnonymousInnerClass();

    protected:
      std::shared_ptr<FieldsConsumerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FieldsConsumerAnonymousInnerClass>(
            org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
      }
    };

  public:
    std::shared_ptr<FieldsProducer> fieldsProducer(
        std::shared_ptr<SegmentReadState> state)  override;

  protected:
    std::shared_ptr<MergeRecordingPostingsFormatWrapper> shared_from_this()
    {
      return std::static_pointer_cast<MergeRecordingPostingsFormatWrapper>(
          org.apache.lucene.codecs.PostingsFormat::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestPerFieldPostingsFormat2> shared_from_this()
  {
    return std::static_pointer_cast<TestPerFieldPostingsFormat2>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/perfield/
