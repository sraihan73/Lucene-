#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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
namespace org::apache::lucene::replicator
{

using SourceDirectoryFactory =
    org::apache::lucene::replicator::ReplicationClient::SourceDirectoryFactory;
using Directory = org::apache::lucene::store::Directory;

/**
 * A {@link SourceDirectoryFactory} which returns {@link FSDirectory} under a
 * dedicated session directory. When a session is over, the entire directory is
 * deleted.
 *
 * @lucene.experimental
 */
class PerSessionDirectoryFactory
    : public std::enable_shared_from_this<PerSessionDirectoryFactory>,
      public SourceDirectoryFactory
{
  GET_CLASS_NAME(PerSessionDirectoryFactory)

private:
  const std::shared_ptr<Path> workDir;

  /** Constructor with the given sources mapping. */
public:
  PerSessionDirectoryFactory(std::shared_ptr<Path> workDir);

  std::shared_ptr<Directory>
  getDirectory(const std::wstring &sessionID,
               const std::wstring &source)  override;

  void
  cleanupSession(const std::wstring &sessionID)  override;
};

} // namespace org::apache::lucene::replicator
