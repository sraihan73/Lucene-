#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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
class TestFileSwitchDirectory : public BaseDirectoryTestCase
{
  GET_CLASS_NAME(TestFileSwitchDirectory)

  /**
   * Test if writing doc stores to disk and everything else to ram works.
   */
public:
  virtual void testBasic() ;

private:
  std::shared_ptr<Directory> newFSSwitchDirectory(
      std::shared_ptr<Set<std::wstring>> primaryExtensions) ;

  std::shared_ptr<Directory> newFSSwitchDirectory(
      std::shared_ptr<Path> aDir, std::shared_ptr<Path> bDir,
      std::shared_ptr<Set<std::wstring>> primaryExtensions) ;

  // LUCENE-3380 -- make sure we get exception if the directory really does not
  // exist.
public:
  void testNoDir()  override;

protected:
  std::shared_ptr<Directory>
  getDirectory(std::shared_ptr<Path> path)  override;

protected:
  std::shared_ptr<TestFileSwitchDirectory> shared_from_this()
  {
    return std::static_pointer_cast<TestFileSwitchDirectory>(
        BaseDirectoryTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
