#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
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
 * Acts like a virus checker on Windows, where random programs may open the
 * files you just wrote in an unfriendly way preventing deletion (e.g. not
 * passing FILE_SHARE_DELETE) or renaming or overwriting etc.  This is more evil
 * than WindowsFS which just prevents deletion of files you still old open.
 */
class VirusCheckingFS : public FilterFileSystemProvider
{
  GET_CLASS_NAME(VirusCheckingFS)

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool enabled = true;
  bool enabled = true;

  const std::shared_ptr<AtomicLong> state;

  /**
   * Create a new instance, wrapping {@code delegate}.
   */
public:
  VirusCheckingFS(std::shared_ptr<FileSystem> delegate_, int64_t salt);

  virtual void enable();

  virtual bool isEnabled();

  virtual void disable();

  void delete_(std::shared_ptr<Path> path)  override;

  // TODO: we could be more evil here, e.g. rename, createOutput, deleteIfExists

protected:
  std::shared_ptr<VirusCheckingFS> shared_from_this()
  {
    return std::static_pointer_cast<VirusCheckingFS>(
        FilterFileSystemProvider::shared_from_this());
  }
};

} // namespace org::apache::lucene::mockfile
