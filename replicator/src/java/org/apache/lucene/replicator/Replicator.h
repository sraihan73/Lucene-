#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/replicator/Revision.h"

#include  "core/src/java/org/apache/lucene/replicator/SessionToken.h"

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
 * An interface for replicating files. Allows a producer to
 * {@link #publish(Revision) publish} {@link Revision}s and consumers to
 * {@link #checkForUpdate(std::wstring) check for updates}. When a client needs to be
 * updated, it is given a {@link SessionToken} through which it can
 * {@link #obtainFile(std::wstring, std::wstring, std::wstring) obtain} the files of that
 * revision. After the client has finished obtaining all the files, it should
 * {@link #release(std::wstring) release} the given session, so that the files can be
 * reclaimed if they are not needed anymore.
 * <p>
 * A client is always updated to the newest revision available. That is, if a
 * client is on revision <em>r1</em> and revisions <em>r2</em> and <em>r3</em>
 * were published, then when the cllient will next check for update, it will
 * receive <em>r3</em>.
 *
 * @lucene.experimental
 */
class Replicator : public Closeable
{
  GET_CLASS_NAME(Replicator)

  /**
   * Publish a new {@link Revision} for consumption by clients. It is the
   * caller's responsibility to verify that the revision files exist and can be
   * read by clients. When the revision is no longer needed, it will be
   * {@link Revision#release() released} by the replicator.
   */
public:
  virtual void publish(std::shared_ptr<Revision> revision) = 0;

  /**
   * Check whether the given version is up-to-date and returns a
   * {@link SessionToken} which can be used for fetching the revision files,
   * otherwise returns {@code null}.
   * <p>
   * <b>NOTE:</b> when the returned session token is no longer needed, you
   * should call {@link #release(std::wstring)} so that the session resources can be
   * reclaimed, including the revision files.
   */
  virtual std::shared_ptr<SessionToken>
  checkForUpdate(const std::wstring &currVersion) = 0;

  /**
   * Notify that the specified {@link SessionToken} is no longer needed by the
   * caller.
   */
  virtual void release(const std::wstring &sessionID) = 0;

  /**
   * Returns an {@link InputStream} for the requested file and source in the
   * context of the given {@link SessionToken#id session}.
   * <p>
   * <b>NOTE:</b> it is the caller's responsibility to close the returned
   * stream.
   *
   * @throws SessionExpiredException if the specified session has already
   *         expired
   */
  virtual std::shared_ptr<InputStream>
  obtainFile(const std::wstring &sessionID, const std::wstring &source,
             const std::wstring &fileName) = 0;
};

} // #include  "core/src/java/org/apache/lucene/replicator/
