#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <unordered_map>

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

/**
 * Controls the health status of a {@link DocumentsWriter} sessions. This class
 * used to block incoming indexing threads if flushing significantly slower than
 * indexing to ensure the {@link DocumentsWriter}s healthiness. If flushing is
 * significantly slower than indexing the net memory used within an
 * {@link IndexWriter} session can increase very quickly and easily exceed the
 * JVM's available memory.
 * <p>
 * To prevent OOM Errors and ensure IndexWriter's stability this class blocks
 * incoming threads from indexing once 2 x number of available
 * {@link ThreadState}s in {@link DocumentsWriterPerThreadPool} is exceeded.
 * Once flushing catches up and the number of flushing DWPT is equal or lower
 * than the number of active {@link ThreadState}s threads are released and can
 * continue indexing.
 */
class DocumentsWriterStallControl final
    : public std::enable_shared_from_this<DocumentsWriterStallControl>
{
  GET_CLASS_NAME(DocumentsWriterStallControl)

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool stalled;
  bool stalled = false;
  int numWaiting = 0; // only with assert
                      // C++ NOTE: Fields cannot have the same name as methods:
  bool wasStalled_ = false; // only with assert
  const std::unordered_map<std::shared_ptr<Thread>, bool> waiting =
      std::make_shared<
          IdentityHashMap<std::shared_ptr<Thread>, bool>>(); // only with assert

  /**
   * Update the stalled flag status. This method will set the stalled flag to
   * <code>true</code> iff the number of flushing
   * {@link DocumentsWriterPerThread} is greater than the number of active
   * {@link DocumentsWriterPerThread}. Otherwise it will reset the
   * {@link DocumentsWriterStallControl} to healthy and release all threads
   * waiting on {@link #waitIfStalled()}
   */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void updateStalled(bool stalled);

  /**
   * Blocks if documents writing is currently in a stalled state.
   *
   */
  void waitIfStalled();

  bool anyStalledThreads();

private:
  void incWaiters();

  void decrWaiters();

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool hasBlocked();

  bool isHealthy();

  // C++ WARNING: The following method was originally marked 'synchronized':
  bool isThreadQueued(std::shared_ptr<Thread> t);

  // C++ WARNING: The following method was originally marked 'synchronized':
  bool wasStalled();
};

} // #include  "core/src/java/org/apache/lucene/index/
