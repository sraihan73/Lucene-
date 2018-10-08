#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"

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

using namespace org::apache::lucene::analysis;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests lazy skipping on the proximity file.
 *
 */
class TestLazyProxSkipping : public LuceneTestCase
{
  GET_CLASS_NAME(TestLazyProxSkipping)
private:
  std::shared_ptr<IndexSearcher> searcher;
  int seeksCounter = 0;

  std::wstring field = L"tokens";
  std::wstring term1 = L"xx";
  std::wstring term2 = L"yy";
  std::wstring term3 = L"zz";

private:
  class SeekCountingDirectory : public MockDirectoryWrapper
  {
    GET_CLASS_NAME(SeekCountingDirectory)
  private:
    std::shared_ptr<TestLazyProxSkipping> outerInstance;

  public:
    SeekCountingDirectory(std::shared_ptr<TestLazyProxSkipping> outerInstance,
                          std::shared_ptr<Directory> delegate_);

    std::shared_ptr<IndexInput>
    openInput(const std::wstring &name,
              std::shared_ptr<IOContext> context)  override;

  protected:
    std::shared_ptr<SeekCountingDirectory> shared_from_this()
    {
      return std::static_pointer_cast<SeekCountingDirectory>(
          org.apache.lucene.store.MockDirectoryWrapper::shared_from_this());
    }
  };

private:
  void createIndex(int numHits) ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestLazyProxSkipping> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestLazyProxSkipping> outerInstance);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          Analyzer::shared_from_this());
    }
  };

private:
  std::deque<std::shared_ptr<ScoreDoc>> search() ;

  void performTest(int numHits) ;

public:
  virtual void testLazySkipping() ;

  virtual void testSeek() ;

  // Simply extends IndexInput in a way that we are able to count the number
  // of invocations of seek()
public:
  class SeeksCountingStream : public IndexInput
  {
    GET_CLASS_NAME(SeeksCountingStream)
  private:
    std::shared_ptr<TestLazyProxSkipping> outerInstance;

    std::shared_ptr<IndexInput> input;

  public:
    SeeksCountingStream(std::shared_ptr<TestLazyProxSkipping> outerInstance,
                        std::shared_ptr<IndexInput> input);

    char readByte()  override;

    void readBytes(std::deque<char> &b, int offset,
                   int len)  override;

    virtual ~SeeksCountingStream();

    int64_t getFilePointer() override;

    void seek(int64_t pos)  override;

    int64_t length() override;

    std::shared_ptr<SeeksCountingStream> clone() override;

    std::shared_ptr<IndexInput>
    slice(const std::wstring &sliceDescription, int64_t offset,
          int64_t length)  override;

  protected:
    std::shared_ptr<SeeksCountingStream> shared_from_this()
    {
      return std::static_pointer_cast<SeeksCountingStream>(
          org.apache.lucene.store.IndexInput::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestLazyProxSkipping> shared_from_this()
  {
    return std::static_pointer_cast<TestLazyProxSkipping>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
