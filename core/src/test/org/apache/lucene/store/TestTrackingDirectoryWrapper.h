#pragma once
#include "stringhelper.h"
#include <memory>

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

class TestTrackingDirectoryWrapper : public BaseDirectoryTestCase
{
  GET_CLASS_NAME(TestTrackingDirectoryWrapper)

protected:
  std::shared_ptr<Directory>
  getDirectory(std::shared_ptr<Path> path)  override;

public:
  virtual void testTrackEmpty() ;

  virtual void testTrackCreate() ;

  virtual void testTrackDelete() ;

  virtual void testTrackRename() ;

  virtual void testTrackCopyFrom() ;

protected:
  std::shared_ptr<TestTrackingDirectoryWrapper> shared_from_this()
  {
    return std::static_pointer_cast<TestTrackingDirectoryWrapper>(
        BaseDirectoryTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
