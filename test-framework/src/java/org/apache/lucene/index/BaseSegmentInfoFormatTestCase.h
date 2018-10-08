#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class SortField;
}

namespace org::apache::lucene::store
{
class MockDirectoryWrapper;
}
namespace org::apache::lucene::index
{
class SegmentInfo;
}
namespace org::apache::lucene::util
{
class Version;
}
namespace org::apache::lucene::document
{
class Document;
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
namespace org::apache::lucene::index
{

using Document = org::apache::lucene::document::Document;
using SortField = org::apache::lucene::search::SortField;
using Version = org::apache::lucene::util::Version;

/**
 * Abstract class to do basic tests for si format.
 * NOTE: This test focuses on the si impl, nothing else.
 * The [stretch] goal is for this test to be
 * so thorough in testing a new si format that if this
 * test passes, then all Lucene/Solr tests should also pass.  Ie,
 * if there is some bug in a given si Format that this
 * test fails to catch then this test needs to be improved! */
class BaseSegmentInfoFormatTestCase : public BaseIndexFileFormatTestCase
{
  GET_CLASS_NAME(BaseSegmentInfoFormatTestCase)

  /** Whether this format records min versions. */
protected:
  virtual bool supportsMinVersion();

  /** Test files map_obj */
public:
  virtual void testFiles() ;

  /** Tests SI writer adds itself to files... */
  virtual void testAddsSelfToFiles() ;

  /** Test diagnostics map_obj */
  virtual void testDiagnostics() ;

  /** Test attributes map_obj */
  virtual void testAttributes() ;

  /** Test unique ID */
  virtual void testUniqueID() ;

  /** Test versions */
  virtual void testVersions() ;

protected:
  virtual bool supportsIndexSort();

private:
  std::shared_ptr<SortField> randomIndexSortField();

  /** Test sort */
public:
  virtual void testSort() ;

  /**
   * Test segment infos write that hits exception immediately on open.
   * make sure we get our exception back, no file handle leaks, etc.
   */
  virtual void testExceptionOnCreateOutput() ;

private:
  class FailureAnonymousInnerClass : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass)
  private:
    std::shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance;

  public:
    FailureAnonymousInnerClass(
        std::shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance);

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
   * Test segment infos write that hits exception on close.
   * make sure we get our exception back, no file handle leaks, etc.
   */
public:
  virtual void testExceptionOnCloseOutput() ;

private:
  class FailureAnonymousInnerClass2 : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass2)
  private:
    std::shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance;

  public:
    FailureAnonymousInnerClass2(
        std::shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance);

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
   * Test segment infos read that hits exception immediately on open.
   * make sure we get our exception back, no file handle leaks, etc.
   */
public:
  virtual void testExceptionOnOpenInput() ;

private:
  class FailureAnonymousInnerClass3 : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass3)
  private:
    std::shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance;

  public:
    FailureAnonymousInnerClass3(
        std::shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance);

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
   * Test segment infos read that hits exception on close
   * make sure we get our exception back, no file handle leaks, etc.
   */
public:
  virtual void testExceptionOnCloseInput() ;

private:
  class FailureAnonymousInnerClass4 : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass4)
  private:
    std::shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance;

  public:
    FailureAnonymousInnerClass4(
        std::shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance);

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

  /**
   * Sets some otherwise hard-to-test properties:
   * random segment names, ID values, document count, etc and round-trips
   */
public:
  virtual void testRandom() ;

protected:
  void assertEquals(std::shared_ptr<SegmentInfo> expected,
                    std::shared_ptr<SegmentInfo> actual);

  /** Returns the versions this SI should test */
  virtual std::deque<std::shared_ptr<Version>> getVersions() = 0;

  /**
   * assert that unique id is equal.
   * @deprecated only exists to be overridden by old codecs that didnt support
   * this
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated protected void assertIDEquals(byte expected[],
  // byte actual[])
  virtual void assertIDEquals(std::deque<char> &expected,
                              std::deque<char> &actual);

  void addRandomFields(std::shared_ptr<Document> doc) override;

public:
  void testRamBytesUsed()  override;

protected:
  std::shared_ptr<BaseSegmentInfoFormatTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseSegmentInfoFormatTestCase>(
        BaseIndexFileFormatTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
