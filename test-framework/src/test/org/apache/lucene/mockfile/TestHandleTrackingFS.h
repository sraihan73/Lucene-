#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>

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

/** Basic tests for HandleTrackingFS */
class TestHandleTrackingFS : public MockFileSystemTestCase
{
  GET_CLASS_NAME(TestHandleTrackingFS)

protected:
  std::shared_ptr<Path> wrap(std::shared_ptr<Path> path) override;

  /** Test that the delegate gets closed on exception in
   * HandleTrackingFS#onClose */
public:
  virtual void testOnCloseThrowsException() ;

private:
  class HandleTrackingFSAnonymousInnerClass : public HandleTrackingFS
  {
    GET_CLASS_NAME(HandleTrackingFSAnonymousInnerClass)
  private:
    std::shared_ptr<TestHandleTrackingFS> outerInstance;

    std::shared_ptr<Path> path;

  public:
    HandleTrackingFSAnonymousInnerClass(
        std::shared_ptr<TestHandleTrackingFS> outerInstance,
        std::shared_ptr<UnknownType> getFileSystem, std::shared_ptr<Path> path);

  protected:
    void onClose(std::shared_ptr<Path> path,
                 std::any stream)  override;

    void onOpen(std::shared_ptr<Path> path,
                std::any stream)  override;

  protected:
    std::shared_ptr<HandleTrackingFSAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HandleTrackingFSAnonymousInnerClass>(
          org.apache.lucene.mockfile.HandleTrackingFS::shared_from_this());
    }
  };

  /** Test that the delegate gets closed on exception in HandleTrackingFS#onOpen
   */
public:
  virtual void testOnOpenThrowsException() ;

private:
  class HandleTrackingFSAnonymousInnerClass2 : public HandleTrackingFS
  {
    GET_CLASS_NAME(HandleTrackingFSAnonymousInnerClass2)
  private:
    std::shared_ptr<TestHandleTrackingFS> outerInstance;

    std::shared_ptr<Path> path;

  public:
    HandleTrackingFSAnonymousInnerClass2(
        std::shared_ptr<TestHandleTrackingFS> outerInstance,
        std::shared_ptr<UnknownType> getFileSystem, std::shared_ptr<Path> path);

  protected:
    void onClose(std::shared_ptr<Path> path,
                 std::any stream)  override;

    void onOpen(std::shared_ptr<Path> path,
                std::any stream)  override;

  protected:
    std::shared_ptr<HandleTrackingFSAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<HandleTrackingFSAnonymousInnerClass2>(
          org.apache.lucene.mockfile.HandleTrackingFS::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestHandleTrackingFS> shared_from_this()
  {
    return std::static_pointer_cast<TestHandleTrackingFS>(
        MockFileSystemTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/mockfile/
