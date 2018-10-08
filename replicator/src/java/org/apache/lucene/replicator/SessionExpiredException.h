#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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
 * Exception indicating that a revision update session was expired due to lack
 * of activity.
 *
 * @see LocalReplicator#DEFAULT_SESSION_EXPIRATION_THRESHOLD
 * @see LocalReplicator#setExpirationThreshold(long)
 *
 * @lucene.experimental
 */
class SessionExpiredException : public IOException
{
  GET_CLASS_NAME(SessionExpiredException)

  /**
   * @see IOException#IOException(std::wstring, Throwable)
   */
public:
  SessionExpiredException(const std::wstring &message,
                          std::runtime_error cause);

  /**
   * @see IOException#IOException(std::wstring)
   */
  SessionExpiredException(const std::wstring &message);

  /**
   * @see IOException#IOException(Throwable)
   */
  SessionExpiredException(std::runtime_error cause);

protected:
  std::shared_ptr<SessionExpiredException> shared_from_this()
  {
    return std::static_pointer_cast<SessionExpiredException>(
        java.io.IOException::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/replicator/
