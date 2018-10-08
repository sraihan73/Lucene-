#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::replicator
{
class RevisionFile;
}

namespace org::apache::lucene::replicator
{
class Revision;
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

/**
 * Token for a replication session, for guaranteeing that source replicated
 * files will be kept safe until the replication completes.
 *
 * @see Replicator#checkForUpdate(std::wstring)
 * @see Replicator#release(std::wstring)
 * @see LocalReplicator#DEFAULT_SESSION_EXPIRATION_THRESHOLD
 *
 * @lucene.experimental
 */
class SessionToken final : public std::enable_shared_from_this<SessionToken>
{
  GET_CLASS_NAME(SessionToken)

  /**
   * ID of this session.
   * Should be passed when releasing the session, thereby acknowledging the
   * {@link Replicator Replicator} that this session is no longer in use.
   * @see Replicator#release(std::wstring)
   */
public:
  const std::wstring id;

  /**
   * @see Revision#getVersion()
   */
  const std::wstring version;

  /**
   * @see Revision#getSourceFiles()
   */
  const std::unordered_map<std::wstring,
                           std::deque<std::shared_ptr<RevisionFile>>>
      sourceFiles;

  /** Constructor which deserializes from the given {@link DataInput}. */
  SessionToken(std::shared_ptr<DataInput> in_) ;

  /** Constructor with the given id and revision. */
  SessionToken(const std::wstring &id, std::shared_ptr<Revision> revision);

  /** Serialize the token data for communication between server and client. */
  void serialize(std::shared_ptr<DataOutput> out) ;

  virtual std::wstring toString();
};
} // namespace org::apache::lucene::replicator
