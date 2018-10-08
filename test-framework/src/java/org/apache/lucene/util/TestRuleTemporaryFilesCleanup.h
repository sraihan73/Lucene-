#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class TestRuleMarkFailure;
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
namespace org::apache::lucene::util
{

using com::carrotsearch::randomizedtesting::rules::TestRuleAdapter;

/**
 * Checks and cleans up temporary files.
 *
 * @see LuceneTestCase#createTempDir()
 * @see LuceneTestCase#createTempFile()
 */
class TestRuleTemporaryFilesCleanup final : public TestRuleAdapter
{
  GET_CLASS_NAME(TestRuleTemporaryFilesCleanup)
  /**
   * Retry to create temporary file name this many times.
   */
private:
  static constexpr int TEMP_NAME_RETRY_THRESHOLD = 9999;

  /**
   * Writeable temporary base folder.
   */
  std::shared_ptr<Path> javaTempDir;

  /**
   * Per-test class temporary folder.
   */
  std::shared_ptr<Path> tempDirBase;

  /**
   * Per-test filesystem
   */
  std::shared_ptr<FileSystem> fileSystem;

  /**
   * Suite failure marker.
   */
  const std::shared_ptr<TestRuleMarkFailure> failureMarker;

  /**
   * A queue of temporary resources to be removed after the
   * suite completes.
   * @see #registerToRemoveAfterSuite(Path)
   */
  static const std::deque<std::shared_ptr<Path>> cleanupQueue;

public:
  TestRuleTemporaryFilesCleanup(
      std::shared_ptr<TestRuleMarkFailure> failureMarker);

  /**
   * Register temporary folder for removal after the suite completes.
   */
  void registerToRemoveAfterSuite(std::shared_ptr<Path> f);

protected:
  void before()  override;

  // os/config-independent limit for too many open files
  // TODO: can we make this lower?
private:
  static constexpr int MAX_OPEN_FILES = 2048;

  bool allowed(std::shared_ptr<Set<std::wstring>> avoid, std::type_info clazz);

  std::shared_ptr<FileSystem> initializeFileSystem();

  std::shared_ptr<Path> initializeJavaTempDir() ;

protected:
  void afterAlways(std::deque<std::runtime_error> &errors) throw(
      std::runtime_error) override;

public:
  std::shared_ptr<Path> getPerTestClassTempDir();

  /**
   * @see LuceneTestCase#createTempDir()
   */
  std::shared_ptr<Path> createTempDir(const std::wstring &prefix);

  /**
   * @see LuceneTestCase#createTempFile()
   */
  std::shared_ptr<Path>
  createTempFile(const std::wstring &prefix,
                 const std::wstring &suffix) ;

protected:
  std::shared_ptr<TestRuleTemporaryFilesCleanup> shared_from_this()
  {
    return std::static_pointer_cast<TestRuleTemporaryFilesCleanup>(
        com.carrotsearch.randomizedtesting.rules
            .TestRuleAdapter::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
