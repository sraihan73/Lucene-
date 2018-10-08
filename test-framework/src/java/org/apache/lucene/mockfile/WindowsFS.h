#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>
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

/**
 * FileSystem that (imperfectly) acts like windows.
 * <p>
 * Currently this filesystem only prevents deletion of open files.
 */
class WindowsFS : public HandleTrackingFS
{
  GET_CLASS_NAME(WindowsFS)
  // This map_obj also supports fileKey -> Path -> counts
  // which is important to effectively support renames etc.
  // in the rename case we have to transfer ownership but need to make sure we
  // only transfer ownership for the path we rename ie. hardlinks will still
  // resolve to the same key
public:
  const std::unordered_map<std::any,
                           std::unordered_map<std::shared_ptr<Path>, int>>
      openFiles =
          std::unordered_map<std::any,
                             std::unordered_map<std::shared_ptr<Path>, int>>();
  // TODO: try to make this as realistic as possible... it depends e.g. how you
  // open files, if you map_obj them, etc, if you can delete them (Uwe knows the
  // rules)

  // TODO: add case-insensitivity

  /**
   * Create a new instance, wrapping the delegate filesystem to
   * act like Windows.
   * @param delegate delegate filesystem to wrap.
   */
  WindowsFS(std::shared_ptr<FileSystem> delegate_);

  /**
   * Returns file "key" (e.g. inode) for the specified path
   */
private:
  std::any getKey(std::shared_ptr<Path> existing) ;

protected:
  void onOpen(std::shared_ptr<Path> path,
              std::any stream)  override;

  void onClose(std::shared_ptr<Path> path,
               std::any stream)  override;

private:
  std::any getKeyOrNull(std::shared_ptr<Path> path);

  /**
   * Checks that it's ok to delete {@code Path}. If the file
   * is still open, it throws IOException("access denied").
   */
  void checkDeleteAccess(std::shared_ptr<Path> path) ;

public:
  void delete_(std::shared_ptr<Path> path)  override;

  void move(std::shared_ptr<Path> source, std::shared_ptr<Path> target,
            std::deque<CopyOption> &options)  override;

  bool deleteIfExists(std::shared_ptr<Path> path)  override;

protected:
  std::shared_ptr<WindowsFS> shared_from_this()
  {
    return std::static_pointer_cast<WindowsFS>(
        HandleTrackingFS::shared_from_this());
  }
};

} // namespace org::apache::lucene::mockfile
