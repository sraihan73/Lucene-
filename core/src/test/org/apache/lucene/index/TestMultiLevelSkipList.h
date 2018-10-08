#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
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

using namespace org::apache::lucene::analysis;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * This testcase tests whether multi-level skipping is being used
 * to reduce I/O while skipping through posting lists.
 *
 * Skipping in general is already covered by several other
 * testcases.
 *
 */
class TestMultiLevelSkipList : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiLevelSkipList)

public:
  class CountingRAMDirectory : public MockDirectoryWrapper
  {
    GET_CLASS_NAME(CountingRAMDirectory)
  private:
    std::shared_ptr<TestMultiLevelSkipList> outerInstance;

  public:
    CountingRAMDirectory(std::shared_ptr<TestMultiLevelSkipList> outerInstance,
                         std::shared_ptr<Directory> delegate_);

    std::shared_ptr<IndexInput>
    openInput(const std::wstring &fileName,
              std::shared_ptr<IOContext> context)  override;

  protected:
    std::shared_ptr<CountingRAMDirectory> shared_from_this()
    {
      return std::static_pointer_cast<CountingRAMDirectory>(
          org.apache.lucene.store.MockDirectoryWrapper::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Before public void setUp() throws Exception
  void setUp()  override;

  virtual void testSimpleSkip() ;

  virtual void checkSkipTo(std::shared_ptr<PostingsEnum> tp, int target,
                           int maxCounter) ;

private:
  class PayloadAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(PayloadAnalyzer)
  private:
    const std::shared_ptr<AtomicInteger> payloadCount =
        std::make_shared<AtomicInteger>(-1);

  public:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<PayloadAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<PayloadAnalyzer>(
          Analyzer::shared_from_this());
    }
  };

private:
  class PayloadFilter : public TokenFilter
  {
    GET_CLASS_NAME(PayloadFilter)

  public:
    std::shared_ptr<PayloadAttribute> payloadAtt;

  private:
    std::shared_ptr<AtomicInteger> payloadCount;

  protected:
    PayloadFilter(std::shared_ptr<AtomicInteger> payloadCount,
                  std::shared_ptr<TokenStream> input);

  public:
    bool incrementToken()  override;

  protected:
    std::shared_ptr<PayloadFilter> shared_from_this()
    {
      return std::static_pointer_cast<PayloadFilter>(
          TokenFilter::shared_from_this());
    }
  };

private:
  int counter = 0;

  // Simply extends IndexInput in a way that we are able to count the number
  // of bytes read
public:
  class CountingStream : public IndexInput
  {
    GET_CLASS_NAME(CountingStream)
  private:
    std::shared_ptr<TestMultiLevelSkipList> outerInstance;

    std::shared_ptr<IndexInput> input;

  public:
    CountingStream(std::shared_ptr<TestMultiLevelSkipList> outerInstance,
                   std::shared_ptr<IndexInput> input);

    char readByte()  override;

    void readBytes(std::deque<char> &b, int offset,
                   int len)  override;

    virtual ~CountingStream();

    int64_t getFilePointer() override;

    void seek(int64_t pos)  override;

    int64_t length() override;

    std::shared_ptr<CountingStream> clone() override;

    std::shared_ptr<IndexInput>
    slice(const std::wstring &sliceDescription, int64_t offset,
          int64_t length)  override;

  protected:
    std::shared_ptr<CountingStream> shared_from_this()
    {
      return std::static_pointer_cast<CountingStream>(
          org.apache.lucene.store.IndexInput::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestMultiLevelSkipList> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiLevelSkipList>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
