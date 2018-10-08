#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/Builder.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"

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
using BufferedIndexInput = org::apache::lucene::store::BufferedIndexInput;
using Directory = org::apache::lucene::store::Directory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestFieldsReader : public LuceneTestCase
{
  GET_CLASS_NAME(TestFieldsReader)
private:
  static std::shared_ptr<Directory> dir;
  static std::shared_ptr<Document> testDoc;
  static std::shared_ptr<FieldInfos::Builder> fieldInfos;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  virtual void test() ;

public:
  class FaultyFSDirectory : public FilterDirectory
  {
    GET_CLASS_NAME(FaultyFSDirectory)
  private:
    std::shared_ptr<TestFieldsReader> outerInstance;

  public:
    std::shared_ptr<AtomicBoolean> doFail = std::make_shared<AtomicBoolean>();

    FaultyFSDirectory(std::shared_ptr<TestFieldsReader> outerInstance,
                      std::shared_ptr<Directory> fsDir);

    std::shared_ptr<IndexInput>
    openInput(const std::wstring &name,
              std::shared_ptr<IOContext> context)  override;

    virtual void startFailing();

  protected:
    std::shared_ptr<FaultyFSDirectory> shared_from_this()
    {
      return std::static_pointer_cast<FaultyFSDirectory>(
          org.apache.lucene.store.FilterDirectory::shared_from_this());
    }
  };

private:
  class FaultyIndexInput : public BufferedIndexInput
  {
    GET_CLASS_NAME(FaultyIndexInput)
  private:
    std::shared_ptr<TestFieldsReader> outerInstance;

    const std::shared_ptr<AtomicBoolean> doFail;

  public:
    std::shared_ptr<IndexInput> delegate_;
    int count = 0;

  private:
    FaultyIndexInput(std::shared_ptr<TestFieldsReader> outerInstance,
                     std::shared_ptr<AtomicBoolean> doFail,
                     std::shared_ptr<IndexInput> delegate_);

    void simOutage() ;

  public:
    void readInternal(std::deque<char> &b, int offset,
                      int length)  override;

    void seekInternal(int64_t pos)  override;

    int64_t length() override;

    virtual ~FaultyIndexInput();

    std::shared_ptr<FaultyIndexInput> clone() override;

    std::shared_ptr<IndexInput>
    slice(const std::wstring &sliceDescription, int64_t offset,
          int64_t length)  override;

  protected:
    std::shared_ptr<FaultyIndexInput> shared_from_this()
    {
      return std::static_pointer_cast<FaultyIndexInput>(
          org.apache.lucene.store.BufferedIndexInput::shared_from_this());
    }
  };

  // LUCENE-1262
public:
  virtual void testExceptions() ;

protected:
  std::shared_ptr<TestFieldsReader> shared_from_this()
  {
    return std::static_pointer_cast<TestFieldsReader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
