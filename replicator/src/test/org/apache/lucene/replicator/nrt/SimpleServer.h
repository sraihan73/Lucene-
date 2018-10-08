#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/replicator/nrt/Node.h"

#include  "core/src/java/org/apache/lucene/store/DataOutput.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/FileMetaData.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/CopyState.h"

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

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Child process with silly naive TCP socket server to handle
 *  between-node commands, launched for each node  by TestNRTReplication. */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({"MockRandom", "Memory", "Direct",
// "SimpleText"}) @SuppressSysoutChecks(bugUrl = "Stuff gets printed, important
// stuff for debugging a failure") @SuppressForbidden(reason = "We need Unsafe to
// actually crush :-)") public class SimpleServer extends
// org.apache.lucene.util.LuceneTestCase
class SimpleServer : public LuceneTestCase
{

public:
  static const std::shared_ptr<Set<std::shared_ptr<Thread>>> clientThreads;
  static const std::shared_ptr<AtomicBoolean> stop;

  /** Handles one client connection */
private:
  class ClientHandler : public Thread
  {
    GET_CLASS_NAME(ClientHandler)

    // We hold this just so we can close it to exit the process:
  private:
    const std::shared_ptr<ServerSocket> ss;
    const std::shared_ptr<Socket> socket;
    const std::shared_ptr<Node> node;
    const int bufferSize;

  public:
    ClientHandler(std::shared_ptr<ServerSocket> ss, std::shared_ptr<Node> node,
                  std::shared_ptr<Socket> socket);

    void run() override;

  protected:
    std::shared_ptr<ClientHandler> shared_from_this()
    {
      return std::static_pointer_cast<ClientHandler>(
          Thread::shared_from_this());
    }
  };

  /**
   * currently, this only works/tested on Sun and IBM.
   */

  // poached from TestIndexWriterOnJRECrash ... should we factor out to
  // TestUtil?  seems dangerous to give it such "publicity"?
private:
  static void crashJRE();

public:
  static void writeFilesMetaData(
      std::shared_ptr<DataOutput> out,
      std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>>
          &files) ;

  static std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>>
  readFilesMetaData(std::shared_ptr<DataInput> in_) ;

  /** Pulls CopyState off the wire */
  static std::shared_ptr<CopyState>
  readCopyState(std::shared_ptr<DataInput> in_) ;

  virtual void test() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleServer> outerInstance;

    std::shared_ptr<ServerSocket> ss;
    std::shared_ptr<org::apache::lucene::replicator::nrt::Node> node;
    int waitForMS = 0;
    bool doClose = false;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<SimpleServer> outerInstance,
        std::shared_ptr<ServerSocket> ss,
        std::shared_ptr<org::apache::lucene::replicator::nrt::Node> node,
        int waitForMS, bool doClose);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SimpleServer> shared_from_this()
  {
    return std::static_pointer_cast<SimpleServer>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/replicator/nrt/
