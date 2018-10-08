#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriterConfig;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class MergePolicy;
}
namespace org::apache::lucene::index
{
class OneMerge;
}
namespace org::apache::lucene::util
{
class LineFileDocs;
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
namespace org::apache::lucene::index
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestForceMergeForever : public LuceneTestCase
{
  GET_CLASS_NAME(TestForceMergeForever)

  // Just counts how many merges are done
private:
  class MyIndexWriter : public IndexWriter
  {
    GET_CLASS_NAME(MyIndexWriter)

  public:
    std::shared_ptr<AtomicInteger> mergeCount =
        std::make_shared<AtomicInteger>();

  private:
    bool first = false;

  public:
    MyIndexWriter(
        std::shared_ptr<Directory> dir,
        std::shared_ptr<IndexWriterConfig> conf) ;

    void merge(std::shared_ptr<MergePolicy::OneMerge> merge) throw(
        IOException) override;

  protected:
    std::shared_ptr<MyIndexWriter> shared_from_this()
    {
      return std::static_pointer_cast<MyIndexWriter>(
          IndexWriter::shared_from_this());
    }
  };

public:
  virtual void test() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestForceMergeForever> outerInstance;

    std::shared_ptr<
        org::apache::lucene::index::TestForceMergeForever::MyIndexWriter>
        w;
    int numStartDocs = 0;
    std::shared_ptr<LineFileDocs> docs;
    std::shared_ptr<AtomicBoolean> doStop;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestForceMergeForever> outerInstance,
        std::shared_ptr<
            org::apache::lucene::index::TestForceMergeForever::MyIndexWriter>
            w,
        int numStartDocs, std::shared_ptr<LineFileDocs> docs,
        std::shared_ptr<AtomicBoolean> doStop);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestForceMergeForever> shared_from_this()
  {
    return std::static_pointer_cast<TestForceMergeForever>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
