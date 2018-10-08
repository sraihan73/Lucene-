#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
 * Adds extra files/subdirectories when directories are created.
 * <p>
 * Lucene shouldn't care about these, but sometimes operating systems
 * create special files themselves (.DS_Store, thumbs.db, .nfsXXX, ...),
 * so we add them and see what breaks.
 * <p>
 * When a directory is created, sometimes a file or directory named
 * "extra0" will be included with it.
 * All other filesystem operations are passed thru as normal.
 */
class ExtrasFS : public FilterFileSystemProvider
{
  GET_CLASS_NAME(ExtrasFS)
public:
  const bool active;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool createDirectory_;

  /**
   * Create a new instance, wrapping {@code delegate}.
   * @param active {@code true} if we should create extra files
   * @param createDirectory {@code true} if we should create directories instead
   * of files. Ignored if {@code active} is {@code false}.
   */
  ExtrasFS(std::shared_ptr<FileSystem> delegate_, bool active,
           bool createDirectory);

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: @Override public void createDirectory(java.nio.file.Path
  // dir, java.nio.file.attribute.FileAttribute<?>... attrs) throws
  // java.io.IOException
      void createDirectory(std::shared_ptr<Path> dir, std::deque<FileAttribute<?>> &attrs)  override;

      // TODO: would be great if we overrode attributes, so file size was always
      // zero for our fake files. But this is tricky because its hooked into
      // several places. Currently MDW has a hack so we don't break disk full
      // tests.

    protected:
      std::shared_ptr<ExtrasFS> shared_from_this()
      {
        return std::static_pointer_cast<ExtrasFS>(
            FilterFileSystemProvider::shared_from_this());
      }
};

} // #include  "core/src/java/org/apache/lucene/mockfile/
