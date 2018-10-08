#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
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

// See: https://issues.apache.org/jira/browse/SOLR-12028 Tests cannot remove
// files on Windows machines occasionally
class TestMockDirectoryWrapper : public BaseDirectoryTestCase
{
  GET_CLASS_NAME(TestMockDirectoryWrapper)

protected:
  std::shared_ptr<Directory>
  getDirectory(std::shared_ptr<Path> path)  override;

  // we wrap the directory in slow stuff, so only run nightly
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Nightly public void testThreadSafety() throws
  // Exception
  void testThreadSafety()  override;

  virtual void testDiskFull() ;

  virtual void testMDWinsideOfMDW() ;

  // just shields the wrapped directory from being closed
private:
  class PreventCloseDirectoryWrapper : public FilterDirectory
  {
    GET_CLASS_NAME(PreventCloseDirectoryWrapper)
  public:
    PreventCloseDirectoryWrapper(std::shared_ptr<Directory> in_);

    virtual ~PreventCloseDirectoryWrapper();

  protected:
    std::shared_ptr<PreventCloseDirectoryWrapper> shared_from_this()
    {
      return std::static_pointer_cast<PreventCloseDirectoryWrapper>(
          FilterDirectory::shared_from_this());
    }
  };

public:
  virtual void testCorruptOnCloseIsWorkingFSDir() ;

  virtual void testCorruptOnCloseIsWorkingRAMDir() ;

private:
  void testCorruptOnCloseIsWorking(std::shared_ptr<Directory> dir) throw(
      std::runtime_error);

public:
  virtual void testAbuseClosedIndexInput() ;

  virtual void testAbuseCloneAfterParentClosed() ;

  virtual void
  testAbuseCloneOfCloneAfterParentClosed() ;

protected:
  std::shared_ptr<TestMockDirectoryWrapper> shared_from_this()
  {
    return std::static_pointer_cast<TestMockDirectoryWrapper>(
        BaseDirectoryTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
