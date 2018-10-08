#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <unordered_map>

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

/**
 * FileSystem that tracks open handles.
 * <p>
 * When {@link FileSystem#close()} is called, this class will throw
 * an exception if any file handles are still open.
 */
class LeakFS : public HandleTrackingFS
{
  GET_CLASS_NAME(LeakFS)
  // we explicitly use reference hashcode/equality in our keys
private:
  const std::unordered_map<std::any, std::runtime_error> openHandles =
      std::make_shared<ConcurrentHashMap<std::any, std::runtime_error>>();

  /**
   * Create a new instance, tracking file handle leaks for the
   * specified delegate filesystem.
   * @param delegate delegate filesystem to wrap.
   */
public:
  LeakFS(std::shared_ptr<FileSystem> delegate_);

protected:
  void onOpen(std::shared_ptr<Path> path, std::any stream) override;

  void onClose(std::shared_ptr<Path> path, std::any stream) override;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void onClose() override;

protected:
  std::shared_ptr<LeakFS> shared_from_this()
  {
    return std::static_pointer_cast<LeakFS>(
        HandleTrackingFS::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/mockfile/
