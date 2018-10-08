#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class InfoStream;
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
namespace org::apache::lucene::mockfile
{

using InfoStream = org::apache::lucene::util::InfoStream;

/** Basic tests for VerboseFS */
class TestVerboseFS : public MockFileSystemTestCase
{
  GET_CLASS_NAME(TestVerboseFS)

protected:
  std::shared_ptr<Path> wrap(std::shared_ptr<Path> path) override;

public:
  virtual std::shared_ptr<Path> wrap(std::shared_ptr<Path> path,
                                     std::shared_ptr<InfoStream> stream);

  /** InfoStream that looks for a substring and indicates if it saw it */
public:
  class InfoStreamListener : public InfoStream
  {
    GET_CLASS_NAME(InfoStreamListener)
    /** True if we saw the message */
  public:
    const std::shared_ptr<AtomicBoolean> seenMessage =
        std::make_shared<AtomicBoolean>(false);
    /** Expected message */
    const std::wstring messageStartsWith;

    InfoStreamListener(const std::wstring &messageStartsWith);

    virtual ~InfoStreamListener();
    void message(const std::wstring &component,
                 const std::wstring &message) override;

    bool isEnabled(const std::wstring &component) override;

    virtual bool sawMessage();

  protected:
    std::shared_ptr<InfoStreamListener> shared_from_this()
    {
      return std::static_pointer_cast<InfoStreamListener>(
          org.apache.lucene.util.InfoStream::shared_from_this());
    }
  };

  /** Test createDirectory */
public:
  virtual void testCreateDirectory() ;

  /** Test delete */
  virtual void testDelete() ;

  /** Test deleteIfExists */
  virtual void testDeleteIfExists() ;

  /** Test copy */
  virtual void testCopy() ;

  /** Test move */
  virtual void testMove() ;

  /** Test newOutputStream */
  virtual void testNewOutputStream() ;

  /** Test FileChannel.open */
  virtual void testFileChannel() ;

  /** Test AsynchronousFileChannel.open */
  virtual void testAsyncFileChannel() ;

  /** Test newByteChannel */
  virtual void testByteChannel() ;

  /** Test that verbose does not corrumpt file not found exceptions */
  virtual void testVerboseFSNoSuchFileException() ;

protected:
  std::shared_ptr<TestVerboseFS> shared_from_this()
  {
    return std::static_pointer_cast<TestVerboseFS>(
        MockFileSystemTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::mockfile
