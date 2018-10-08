#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::replicator
{
class Revision;
}

namespace org::apache::lucene::replicator
{
class SessionToken;
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
 * A {@link Replicator} implementation for use by the side that publishes
 * {@link Revision}s, as well for clients to {@link #checkForUpdate(std::wstring)
 * check for updates}. When a client needs to be updated, it is returned a
 * {@link SessionToken} through which it can
 * {@link #obtainFile(std::wstring, std::wstring, std::wstring) obtain} the files of that
 * revision. As long as a revision is being replicated, this replicator
 * guarantees that it will not be {@link Revision#release() released}.
 * <p>
 * Replication sessions expire by default after
 * {@link #DEFAULT_SESSION_EXPIRATION_THRESHOLD}, and the threshold can be
 * configured through {@link #setExpirationThreshold(long)}.
 *
 * @lucene.experimental
 */
class LocalReplicator : public std::enable_shared_from_this<LocalReplicator>,
                        public Replicator
{
  GET_CLASS_NAME(LocalReplicator)

private:
  class RefCountedRevision
      : public std::enable_shared_from_this<RefCountedRevision>
  {
    GET_CLASS_NAME(RefCountedRevision)
  private:
    const std::shared_ptr<AtomicInteger> refCount =
        std::make_shared<AtomicInteger>(1);

  public:
    const std::shared_ptr<Revision> revision;

    RefCountedRevision(std::shared_ptr<Revision> revision);

    virtual void decRef() ;

    virtual void incRef();
  };

private:
  class ReplicationSession
      : public std::enable_shared_from_this<ReplicationSession>
  {
    GET_CLASS_NAME(ReplicationSession)
  public:
    const std::shared_ptr<SessionToken> session;
    const std::shared_ptr<RefCountedRevision> revision;

  private:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: private volatile long lastAccessTime;
    int64_t lastAccessTime = 0;

  public:
    ReplicationSession(std::shared_ptr<SessionToken> session,
                       std::shared_ptr<RefCountedRevision> revision);

    virtual bool isExpired(int64_t expirationThreshold);

    virtual void markAccessed();
  };

  /** Threshold for expiring inactive sessions. Defaults to 30 minutes. */
public:
  static constexpr int64_t DEFAULT_SESSION_EXPIRATION_THRESHOLD =
      1000 * 60 * 30;

private:
  int64_t expirationThresholdMilllis =
      LocalReplicator::DEFAULT_SESSION_EXPIRATION_THRESHOLD;

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile RefCountedRevision currentRevision;
  std::shared_ptr<RefCountedRevision> currentRevision;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool closed = false;
  bool closed = false;

  const std::shared_ptr<AtomicInteger> sessionToken =
      std::make_shared<AtomicInteger>(0);
  const std::unordered_map<std::wstring, std::shared_ptr<ReplicationSession>>
      sessions = std::unordered_map<std::wstring,
                                    std::shared_ptr<ReplicationSession>>();

  void checkExpiredSessions() ;

  void releaseSession(const std::wstring &sessionID) ;

  /** Ensure that replicator is still open, or throw {@link
   * AlreadyClosedException} otherwise. */
protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void ensureOpen();

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<SessionToken>
  checkForUpdate(const std::wstring &currentVersion) override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~LocalReplicator();

  /**
   * Returns the expiration threshold.
   *
   * @see #setExpirationThreshold(long)
   */
  virtual int64_t getExpirationThreshold();

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<InputStream>
  obtainFile(const std::wstring &sessionID, const std::wstring &source,
             const std::wstring &fileName)  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void publish(std::shared_ptr<Revision> revision)  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void release(const std::wstring &sessionID)  override;

  /**
   * Modify session expiration time - if a replication session is inactive that
   * long it is automatically expired, and further attempts to operate within
   * this session will throw a {@link SessionExpiredException}.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void
  setExpirationThreshold(int64_t expirationThreshold) ;
};

} // namespace org::apache::lucene::replicator
