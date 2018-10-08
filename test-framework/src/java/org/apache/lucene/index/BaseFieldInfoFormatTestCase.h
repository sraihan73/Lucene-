#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"

#include  "core/src/java/org/apache/lucene/index/IndexableFieldType.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"

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
namespace org::apache::lucene::index
{

using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;

/**
 * Abstract class to do basic tests for fis format.
 * NOTE: This test focuses on the fis impl, nothing else.
 * The [stretch] goal is for this test to be
 * so thorough in testing a new fis format that if this
 * test passes, then all Lucene/Solr tests should also pass.  Ie,
 * if there is some bug in a given fis Format that this
 * test fails to catch then this test needs to be improved! */
class BaseFieldInfoFormatTestCase : public BaseIndexFileFormatTestCase
{
  GET_CLASS_NAME(BaseFieldInfoFormatTestCase)

  /** Test field infos read/write with a single field */
public:
  virtual void testOneField() ;

  /** Test field infos attributes coming back are not mutable */
  virtual void testImmutableAttributes() ;

  /**
   * Test field infos write that hits exception immediately on open.
   * make sure we get our exception back, no file handle leaks, etc.
   */
  virtual void testExceptionOnCreateOutput() ;

private:
  class FailureAnonymousInnerClass : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass)
  private:
    std::shared_ptr<BaseFieldInfoFormatTestCase> outerInstance;

  public:
    FailureAnonymousInnerClass(
        std::shared_ptr<BaseFieldInfoFormatTestCase> outerInstance);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

  /**
   * Test field infos write that hits exception on close.
   * make sure we get our exception back, no file handle leaks, etc.
   */
public:
  virtual void testExceptionOnCloseOutput() ;

private:
  class FailureAnonymousInnerClass2 : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass2)
  private:
    std::shared_ptr<BaseFieldInfoFormatTestCase> outerInstance;

  public:
    FailureAnonymousInnerClass2(
        std::shared_ptr<BaseFieldInfoFormatTestCase> outerInstance);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass2>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

  /**
   * Test field infos read that hits exception immediately on open.
   * make sure we get our exception back, no file handle leaks, etc.
   */
public:
  virtual void testExceptionOnOpenInput() ;

private:
  class FailureAnonymousInnerClass3 : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass3)
  private:
    std::shared_ptr<BaseFieldInfoFormatTestCase> outerInstance;

  public:
    FailureAnonymousInnerClass3(
        std::shared_ptr<BaseFieldInfoFormatTestCase> outerInstance);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass3>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

  /**
   * Test field infos read that hits exception on close.
   * make sure we get our exception back, no file handle leaks, etc.
   */
public:
  virtual void testExceptionOnCloseInput() ;

private:
  class FailureAnonymousInnerClass4 : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass4)
  private:
    std::shared_ptr<BaseFieldInfoFormatTestCase> outerInstance;

  public:
    FailureAnonymousInnerClass4(
        std::shared_ptr<BaseFieldInfoFormatTestCase> outerInstance);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass4>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

  // TODO: more tests

  /** Test field infos read/write with random fields, with different values. */
public:
  virtual void testRandom() ;

private:
  std::shared_ptr<IndexableFieldType>
  randomFieldType(std::shared_ptr<Random> r);

  /**
   * Hook to add any codec attributes to fieldinfo
   * instances added in this test.
   */
protected:
  virtual void addAttributes(std::shared_ptr<FieldInfo> fi);

  /**
   * Docvalues types to test.
   * @deprecated only for Only available to ancient codecs can
   * limit this to the subset of types they support.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated protected DocValuesType[] getDocValuesTypes()
  virtual std::deque<DocValuesType> getDocValuesTypes();

  /** equality for entirety of fieldinfos */
  virtual void assertEquals(std::shared_ptr<FieldInfos> expected,
                            std::shared_ptr<FieldInfos> actual);

  /** equality for two individual fieldinfo objects */
  virtual void assertEquals(std::shared_ptr<FieldInfo> expected,
                            std::shared_ptr<FieldInfo> actual);

  /** Returns a new fake segment */
  static std::shared_ptr<SegmentInfo>
  newSegmentInfo(std::shared_ptr<Directory> dir, const std::wstring &name);

  void addRandomFields(std::shared_ptr<Document> doc) override;

public:
  void testRamBytesUsed()  override;

protected:
  std::shared_ptr<BaseFieldInfoFormatTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseFieldInfoFormatTestCase>(
        BaseIndexFileFormatTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
