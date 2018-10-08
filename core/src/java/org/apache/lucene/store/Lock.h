#pragma once
#include "stringhelper.h"
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
namespace org::apache::lucene::store
{

/** An interprocess mutex lock.
 * <p>Typical use might look like:<pre class="prettyprint">
 *   try (final Lock lock = directory.obtainLock("my.lock")) {
GET_CLASS_NAME(="prettyprint">)
 *     // ... code to execute while locked ...
 *   }
 * </pre>
 *
 * @see Directory#obtainLock(std::wstring)
 *
 * @lucene.internal
 */
class Lock : public std::enable_shared_from_this<Lock>
{
  GET_CLASS_NAME(Lock)

  /**
   * Releases exclusive access.
   * <p>
   * Note that exceptions thrown from close may require
   * human intervention, as it may mean the lock was no
   * longer valid, or that fs permissions prevent removal
   * of the lock file, or other reasons.
   * <p>
   * {@inheritDoc}
   * @throws LockReleaseFailedException optional specific exception) if
   *         the lock could not be properly released.
   */
public:
  virtual void close() = 0;

  /**
   * Best effort check that this lock is still valid. Locks
   * could become invalidated externally for a number of reasons,
   * for example if a user deletes the lock file manually or
   * when a network filesystem is in use.
   * @throws IOException if the lock is no longer valid.
   */
  virtual void ensureValid() = 0;
};

} // #include  "core/src/java/org/apache/lucene/store/
