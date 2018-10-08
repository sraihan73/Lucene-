#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::util
{

/**
 * A default {@link ThreadFactory} implementation that accepts the name prefix
 * of the created threads as a constructor argument. Otherwise, this factory
 * yields the same semantics as the thread factory returned by
 * {@link Executors#defaultThreadFactory()}.
 */
class NamedThreadFactory
    : public std::enable_shared_from_this<NamedThreadFactory>,
      public ThreadFactory
{
  GET_CLASS_NAME(NamedThreadFactory)
private:
  static const std::shared_ptr<AtomicInteger> threadPoolNumber;
  const std::shared_ptr<ThreadGroup> group;
  const std::shared_ptr<AtomicInteger> threadNumber =
      std::make_shared<AtomicInteger>(1);
  static const std::wstring NAME_PATTERN;
  const std::wstring threadNamePrefix;

  /**
   * Creates a new {@link NamedThreadFactory} instance
   *
   * @param threadNamePrefix the name prefix assigned to each thread created.
   */
public:
  NamedThreadFactory(const std::wstring &threadNamePrefix);

private:
  static std::wstring checkPrefix(const std::wstring &prefix);

  /**
   * Creates a new {@link Thread}
   *
   * @see java.util.concurrent.ThreadFactory#newThread(java.lang.Runnable)
   */
public:
  std::shared_ptr<Thread> newThread(Runnable r) override;
};

} // namespace org::apache::lucene::util
