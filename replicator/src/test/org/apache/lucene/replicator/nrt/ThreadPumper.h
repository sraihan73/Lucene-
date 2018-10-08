#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
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

namespace org::apache::lucene::replicator::nrt
{

/** A pipe thread. It'd be nice to reuse guava's implementation for this... */
class ThreadPumper : public std::enable_shared_from_this<ThreadPumper>
{
  GET_CLASS_NAME(ThreadPumper)
public:
  static std::shared_ptr<Thread>
  start(Runnable const onExit, std::shared_ptr<BufferedReader> from,
        std::shared_ptr<PrintStream> to, std::shared_ptr<Writer> toFile,
        std::shared_ptr<AtomicBoolean> nodeClosing);

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    Runnable onExit;
    std::shared_ptr<BufferedReader> from;
    std::shared_ptr<Writer> toFile;
    std::shared_ptr<AtomicBoolean> nodeClosing;

  public:
    ThreadAnonymousInnerClass(Runnable onExit,
                              std::shared_ptr<BufferedReader> from,
                              std::shared_ptr<Writer> toFile,
                              std::shared_ptr<AtomicBoolean> nodeClosing);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/replicator/nrt/
