#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/replicator/RevisionFile.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/replicator/Replicator.h"
#include  "core/src/java/org/apache/lucene/util/InfoStream.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

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

using Directory = org::apache::lucene::store::Directory;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * A client which monitors and obtains new revisions from a {@link Replicator}.
 * It can be used to either periodically check for updates by invoking
 * {@link #startUpdateThread}, or manually by calling {@link #updateNow()}.
 * <p>
 * Whenever a new revision is available, the {@link #requiredFiles(Map)} are
 * copied to the {@link Directory} specified by {@link
 * PerSessionDirectoryFactory} and a handler is notified.
 *
 * @lucene.experimental
 */
class ReplicationClient : public std::enable_shared_from_this<ReplicationClient>
{
  GET_CLASS_NAME(ReplicationClient)

private:
  class ReplicationThread : public Thread
  {
    GET_CLASS_NAME(ReplicationThread)
  private:
    std::shared_ptr<ReplicationClient> outerInstance;

    const int64_t interval;

    // client uses this to stop us
  public:
    const std::shared_ptr<CountDownLatch> stop =
        std::make_shared<CountDownLatch>(1);

    ReplicationThread(std::shared_ptr<ReplicationClient> outerInstance,
                      int64_t interval);

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override public
    // void run()
    void run() override;

  protected:
    std::shared_ptr<ReplicationThread> shared_from_this()
    {
      return std::static_pointer_cast<ReplicationThread>(
          Thread::shared_from_this());
    }
  };

  /** Handler for revisions obtained by the client. */
public:
  class ReplicationHandler
  {
    GET_CLASS_NAME(ReplicationHandler)

    /** Returns the current revision files held by the handler. */
  public:
    virtual std::unordered_map<std::wstring,
                               std::deque<std::shared_ptr<RevisionFile>>>
    currentRevisionFiles() = 0;

    /** Returns the current revision version held by the handler. */
    virtual std::wstring currentVersion() = 0;

    /**
     * Called when a new revision was obtained and is available (i.e. all needed
     * files were successfully copied).
     *
     * @param version
     *          the version of the {@link Revision} that was copied
     * @param revisionFiles
     *          the files contained by this {@link Revision}
     * @param copiedFiles
     *          the files that were actually copied
     * @param sourceDirectory
     *          a mapping from a source of files to the {@link Directory} they
     *          were copied into
     */
    virtual void
    revisionReady(const std::wstring &version,
                  std::unordered_map<std::wstring,
                                     std::deque<std::shared_ptr<RevisionFile>>>
                      &revisionFiles,
                  std::unordered_map<std::wstring, std::deque<std::wstring>>
                      &copiedFiles,
                  std::unordered_map<std::wstring, std::shared_ptr<Directory>>
                      &sourceDirectory) = 0;
  };

  /**
   * Resolves a session and source into a {@link Directory} to use for copying
   * the session files to.
   */
public:
  class SourceDirectoryFactory
  {
    GET_CLASS_NAME(SourceDirectoryFactory)

    /**
     * Called to denote that the replication actions for this session were
     * finished and the directory is no longer needed.
     */
  public:
    virtual void cleanupSession(const std::wstring &sessionID) = 0;

    /**
     * Returns the {@link Directory} to use for the given session and source.
     * Implementations may e.g. return different directories for different
     * sessions, or the same directory for all sessions. In that case, it is
     * advised to clean the directory before it is used for a new session.
     *
     * @see #cleanupSession(std::wstring)
     */
    virtual std::shared_ptr<Directory>
    getDirectory(const std::wstring &sessionID, const std::wstring &source) = 0;
  };

  /** The component name to use with {@link InfoStream#isEnabled(std::wstring)}. */
public:
  static const std::wstring INFO_STREAM_COMPONENT;

private:
  const std::shared_ptr<Replicator> replicator;
  const std::shared_ptr<ReplicationHandler> handler;
  const std::shared_ptr<SourceDirectoryFactory> factory;
  std::deque<char> const copyBuffer = std::deque<char>(16384);
  const std::shared_ptr<Lock> updateLock = std::make_shared<ReentrantLock>();

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile ReplicationThread updateThread;
  std::shared_ptr<ReplicationThread> updateThread;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool closed = false;
  bool closed = false;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile org.apache.lucene.util.InfoStream
  // infoStream = org.apache.lucene.util.InfoStream.getDefault();
  std::shared_ptr<InfoStream> infoStream = InfoStream::getDefault();

  /**
   * Constructor.
   *
   * @param replicator the {@link Replicator} used for checking for updates
   * @param handler notified when new revisions are ready
   * @param factory returns a {@link Directory} for a given source and session
   */
public:
  ReplicationClient(std::shared_ptr<Replicator> replicator,
                    std::shared_ptr<ReplicationHandler> handler,
                    std::shared_ptr<SourceDirectoryFactory> factory);

private:
  void copyBytes(std::shared_ptr<IndexOutput> out,
                 std::shared_ptr<InputStream> in_) ;

  void doUpdate() ;

  /** Throws {@link AlreadyClosedException} if the client has already been
   * closed. */
protected:
  void ensureOpen();

  /**
   * Called when an exception is hit by the replication thread. The default
   * implementation prints the full stacktrace to the {@link InfoStream} set in
   * {@link #setInfoStream(InfoStream)}, or the {@link InfoStream#getDefault()
   * default} one. You can override to log the exception elswhere.
   * <p>
   * <b>NOTE:</b> if you override this method to throw the exception further,
   * the replication thread will be terminated. The only way to restart it is to
   * call {@link #stopUpdateThread()} followed by
   * {@link #startUpdateThread(long, std::wstring)}.
   */
  virtual void handleUpdateException(std::runtime_error t);

  /**
   * Returns the files required for replication. By default, this method returns
   * all files that exist in the new revision, but not in the handler.
   */
  virtual std::unordered_map<std::wstring,
                             std::deque<std::shared_ptr<RevisionFile>>>
  requiredFiles(std::unordered_map<std::wstring,
                                   std::deque<std::shared_ptr<RevisionFile>>>
                    &newRevisionFiles);

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~ReplicationClient();

  /**
   * Start the update thread with the specified interval in milliseconds. For
   * debugging purposes, you can optionally set the name to set on
   * {@link Thread#setName(std::wstring)}. If you pass {@code null}, a default name
   * will be set.
   *
   * @throws IllegalStateException if the thread has already been started
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void startUpdateThread(int64_t intervalMillis,
                                 const std::wstring &threadName);

  /**
   * Stop the update thread. If the update thread is not running, silently does
   * nothing. This method returns after the update thread has stopped.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void stopUpdateThread();

  /**
   * Returns true if the update thread is alive. The update thread is alive if
   * it has been {@link #startUpdateThread(long, std::wstring) started} and not
   * {@link #stopUpdateThread() stopped}, as well as didn't hit an error which
   * caused it to terminate (i.e. {@link #handleUpdateException(Throwable)}
   * threw the exception further).
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual bool isUpdateThreadAlive();

  virtual std::wstring toString();

  /**
   * Executes the update operation immediately, irregardess if an update thread
   * is running or not.
   */
  virtual void updateNow() ;

  /** Sets the {@link InfoStream} to use for logging messages. */
  virtual void setInfoStream(std::shared_ptr<InfoStream> infoStream);
};

} // #include  "core/src/java/org/apache/lucene/replicator/
