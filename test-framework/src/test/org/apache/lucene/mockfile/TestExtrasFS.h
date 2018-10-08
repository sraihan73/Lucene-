#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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

/** Basic tests for ExtrasFS */
class TestExtrasFS : public MockFileSystemTestCase
{
  GET_CLASS_NAME(TestExtrasFS)

protected:
  std::shared_ptr<Path> wrap(std::shared_ptr<Path> path) override;

public:
  virtual std::shared_ptr<Path> wrap(std::shared_ptr<Path> path, bool active,
                                     bool createDirectory);

  /** test where extra file is created */
  virtual void testExtraFile() ;

  /** test where extra directory is created */
  virtual void testExtraDirectory() ;

  /** test where no extras are created: its a no-op */
  virtual void testNoExtras() ;

protected:
  std::shared_ptr<TestExtrasFS> shared_from_this()
  {
    return std::static_pointer_cast<TestExtrasFS>(
        MockFileSystemTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::mockfile
