#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::store
{
class IOContext;
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
namespace org::apache::lucene::store
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestBufferedIndexInput : public LuceneTestCase
{
  GET_CLASS_NAME(TestBufferedIndexInput)

private:
  static constexpr int64_t TEST_FILE_LENGTH = 100 * 1024;

  // Call readByte() repeatedly, past the buffer boundary, and see that it
  // is working as expected.
  // Our input comes from a dynamically generated/ "file" - see
  // MyBufferedIndexInput below.
public:
  virtual void testReadByte() ;

  // Call readBytes() repeatedly, with various chunk sizes (from 1 byte to
  // larger than the buffer size), and see that it returns the bytes we expect.
  // Our input comes from a dynamically generated "file" -
  // see MyBufferedIndexInput below.
  virtual void testReadBytes() ;

private:
  void runReadBytesAndClose(std::shared_ptr<IndexInput> input, int bufferSize,
                            std::shared_ptr<Random> r) ;

  void runReadBytes(std::shared_ptr<IndexInput> input, int bufferSize,
                    std::shared_ptr<Random> r) ;

  std::deque<char> buffer = std::deque<char>(10);

  void checkReadBytes(std::shared_ptr<IndexInput> input, int size,
                      int pos) ;

  // This tests that attempts to readBytes() past an EOF will fail, while
  // reads up to the EOF will succeed. The EOF is determined by the
  // BufferedIndexInput's arbitrary length() value.
public:
  virtual void testEOF() ;

  // byten emulates a file - byten(n) returns the n'th byte in that file.
  // MyBufferedIndexInput reads this "file".
private:
  static char byten(int64_t n);

private:
  class MyBufferedIndexInput : public BufferedIndexInput
  {
    GET_CLASS_NAME(MyBufferedIndexInput)
  private:
    int64_t pos = 0;
    int64_t len = 0;

  public:
    MyBufferedIndexInput(int64_t len);
    MyBufferedIndexInput();

  protected:
    void readInternal(std::deque<char> &b, int offset,
                      int length)  override;

    void seekInternal(int64_t pos)  override;

  public:
    virtual ~MyBufferedIndexInput();

    int64_t length() override;

    std::shared_ptr<IndexInput>
    slice(const std::wstring &sliceDescription, int64_t offset,
          int64_t length)  override;

  protected:
    std::shared_ptr<MyBufferedIndexInput> shared_from_this()
    {
      return std::static_pointer_cast<MyBufferedIndexInput>(
          BufferedIndexInput::shared_from_this());
    }
  };

public:
  virtual void testSetBufferSize() ;

private:
  class MockFSDirectory : public FilterDirectory
  {
    GET_CLASS_NAME(MockFSDirectory)

  public:
    const std::deque<std::shared_ptr<IndexInput>> allIndexInputs =
        std::deque<std::shared_ptr<IndexInput>>();
    const std::shared_ptr<Random> rand;

    MockFSDirectory(std::shared_ptr<Path> path,
                    std::shared_ptr<Random> rand) ;

    virtual void tweakBufferSizes();

    std::shared_ptr<IndexInput>
    openInput(const std::wstring &name,
              std::shared_ptr<IOContext> context)  override;

  protected:
    std::shared_ptr<MockFSDirectory> shared_from_this()
    {
      return std::static_pointer_cast<MockFSDirectory>(
          FilterDirectory::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestBufferedIndexInput> shared_from_this()
  {
    return std::static_pointer_cast<TestBufferedIndexInput>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
