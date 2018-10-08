#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_set>
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

/** Basic tests for ShuffleFS */
class TestShuffleFS : public MockFileSystemTestCase
{
  GET_CLASS_NAME(TestShuffleFS)

protected:
  std::shared_ptr<Path> wrap(std::shared_ptr<Path> path) override;

public:
  virtual std::shared_ptr<Path> wrap(std::shared_ptr<Path> path,
                                     int64_t seed);

  /** test that we return directory listings correctly */
  virtual void testShuffleWorks() ;

  /** test that we change order of directory listings */
  virtual void testActuallyShuffles() ;

  /**
   * shuffle underlying contents randomly with different seeds,
   * and ensure shuffling that again with the same seed is consistent.
   */
  virtual void testConsistentOrder() ;

  /**
   * test that we give a consistent order
   * for the same file names within different directories
   */
  virtual void testFileNameOnly() ;

protected:
  std::shared_ptr<TestShuffleFS> shared_from_this()
  {
    return std::static_pointer_cast<TestShuffleFS>(
        MockFileSystemTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::mockfile
