#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
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
namespace org::apache::lucene::index
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static System.nanoTime;

/**
 * An implementation of {@link QueryTimeout} that can be used by
 * the {@link ExitableDirectoryReader} class to time out and exit out
 * when a query takes a long time to rewrite.
 */
class QueryTimeoutImpl : public std::enable_shared_from_this<QueryTimeoutImpl>,
                         public QueryTimeout
{
  GET_CLASS_NAME(QueryTimeoutImpl)

  /**
   * The local variable to store the time beyond which, the processing should
   * exit.
   */
private:
  std::optional<int64_t> timeoutAt;

  /**
   * Sets the time at which to time out by adding the given timeAllowed to the
   * current time.
   *
   * @param timeAllowed Number of milliseconds after which to time out. Use
   * {@code Long.MAX_VALUE} to effectively never time out.
   */
public:
  QueryTimeoutImpl(int64_t timeAllowed);

  /**
   * Returns time at which to time out, in nanoseconds relative to the
   * (JVM-specific) epoch for {@link System#nanoTime()}, to compare with the
   * value returned by
   * {@code nanoTime()}.
   */
  virtual std::optional<int64_t> getTimeoutAt();

  /**
   * Return true if {@link #reset()} has not been called
   * and the elapsed time has exceeded the time allowed.
   */
  bool shouldExit() override;

  /**
   * Reset the timeout value.
   */
  virtual void reset();

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/index/
