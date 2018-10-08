#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriterConfig;
}

namespace org::apache::lucene::store
{
class Directory;
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Tests indexwriter's infostream */
class TestInfoStream : public LuceneTestCase
{
  GET_CLASS_NAME(TestInfoStream)

  /** we shouldn't have test points unless we ask */
public:
  virtual void testTestPointsOff() ;

private:
  class InfoStreamAnonymousInnerClass : public InfoStream
  {
    GET_CLASS_NAME(InfoStreamAnonymousInnerClass)
  private:
    std::shared_ptr<TestInfoStream> outerInstance;

  public:
    InfoStreamAnonymousInnerClass(
        std::shared_ptr<TestInfoStream> outerInstance);

    virtual ~InfoStreamAnonymousInnerClass();
    void message(const std::wstring &component,
                 const std::wstring &message) override;

    bool isEnabled(const std::wstring &component) override;

  protected:
    std::shared_ptr<InfoStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<InfoStreamAnonymousInnerClass>(
          org.apache.lucene.util.InfoStream::shared_from_this());
    }
  };

  /** but they should work when we need */
public:
  virtual void testTestPointsOn() ;

private:
  class InfoStreamAnonymousInnerClass2 : public InfoStream
  {
    GET_CLASS_NAME(InfoStreamAnonymousInnerClass2)
  private:
    std::shared_ptr<TestInfoStream> outerInstance;

    std::shared_ptr<AtomicBoolean> seenTestPoint;

  public:
    InfoStreamAnonymousInnerClass2(
        std::shared_ptr<TestInfoStream> outerInstance,
        std::shared_ptr<AtomicBoolean> seenTestPoint);

    virtual ~InfoStreamAnonymousInnerClass2();
    void message(const std::wstring &component,
                 const std::wstring &message) override;

    bool isEnabled(const std::wstring &component) override;

  protected:
    std::shared_ptr<InfoStreamAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<InfoStreamAnonymousInnerClass2>(
          org.apache.lucene.util.InfoStream::shared_from_this());
    }
  };

private:
  class IndexWriterAnonymousInnerClass : public IndexWriter
  {
    GET_CLASS_NAME(IndexWriterAnonymousInnerClass)
  private:
    std::shared_ptr<TestInfoStream> outerInstance;

  public:
    IndexWriterAnonymousInnerClass(
        std::shared_ptr<TestInfoStream> outerInstance,
        std::shared_ptr<Directory> dir,
        std::shared_ptr<org::apache::lucene::index::IndexWriterConfig> iwc);

  protected:
    bool isEnableTestPoints() override;

  protected:
    std::shared_ptr<IndexWriterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IndexWriterAnonymousInnerClass>(
          IndexWriter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestInfoStream> shared_from_this()
  {
    return std::static_pointer_cast<TestInfoStream>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
