#pragma once
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

/**
 * FileSystem that throws exception if file handles
 * in use exceeds a specified limit
 */
class HandleLimitFS : public HandleTrackingFS
{
  GET_CLASS_NAME(HandleLimitFS)
public:
  const int limit;
  const std::shared_ptr<AtomicInteger> count =
      std::make_shared<AtomicInteger>();

  /**
   * Create a new instance, limiting the maximum number
   * of open files to {@code limit}
   * @param delegate delegate filesystem to wrap.
   * @param limit maximum number of open files.
   */
  HandleLimitFS(std::shared_ptr<FileSystem> delegate_, int limit);

protected:
  void onOpen(std::shared_ptr<Path> path,
              std::any stream)  override;

  void onClose(std::shared_ptr<Path> path,
               std::any stream)  override;

protected:
  std::shared_ptr<HandleLimitFS> shared_from_this()
  {
    return std::static_pointer_cast<HandleLimitFS>(
        HandleTrackingFS::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/mockfile/
