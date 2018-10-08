#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class IndexWriterConfig;
}
namespace org::apache::lucene::index
{
class IndexWriter;
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
namespace org::apache::lucene::facet::taxonomy::directory
{

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;

class TestDirectoryTaxonomyReader : public FacetTestCase
{
  GET_CLASS_NAME(TestDirectoryTaxonomyReader)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCloseAfterIncRef() throws Exception
  virtual void testCloseAfterIncRef() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCloseTwice() throws Exception
  virtual void testCloseTwice() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOpenIfChangedResult() throws Exception
  virtual void testOpenIfChangedResult() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAlreadyClosed() throws Exception
  virtual void testAlreadyClosed() ;

  /**
   * recreating a taxonomy should work well with a freshly opened taxonomy
   * reader
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFreshReadRecreatedTaxonomy() throws
  // Exception
  virtual void testFreshReadRecreatedTaxonomy() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOpenIfChangedReadRecreatedTaxonomy()
  // throws Exception
  virtual void
  testOpenIfChangedReadRecreatedTaxonomy() ;

private:
  void doTestReadRecreatedTaxonomy(std::shared_ptr<Random> random,
                                   bool closeReader) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOpenIfChangedAndRefCount() throws
  // Exception
  virtual void testOpenIfChangedAndRefCount() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOpenIfChangedManySegments() throws
  // Exception
  virtual void testOpenIfChangedManySegments() ;

private:
  class DirectoryTaxonomyWriterAnonymousInnerClass
      : public DirectoryTaxonomyWriter
  {
    GET_CLASS_NAME(DirectoryTaxonomyWriterAnonymousInnerClass)
  private:
    std::shared_ptr<TestDirectoryTaxonomyReader> outerInstance;

  public:
    DirectoryTaxonomyWriterAnonymousInnerClass(
        std::shared_ptr<TestDirectoryTaxonomyReader> outerInstance,
        std::shared_ptr<Directory> dir);

  protected:
    std::shared_ptr<IndexWriterConfig>
    createIndexWriterConfig(OpenMode openMode) override;

  protected:
    std::shared_ptr<DirectoryTaxonomyWriterAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DirectoryTaxonomyWriterAnonymousInnerClass>(
          DirectoryTaxonomyWriter::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOpenIfChangedMergedSegment() throws
  // Exception
  virtual void testOpenIfChangedMergedSegment() ;

private:
  class DirectoryTaxonomyWriterAnonymousInnerClass2
      : public DirectoryTaxonomyWriter
  {
    GET_CLASS_NAME(DirectoryTaxonomyWriterAnonymousInnerClass2)
  private:
    std::shared_ptr<TestDirectoryTaxonomyReader> outerInstance;

    std::shared_ptr<IndexWriter> iw;

  public:
    DirectoryTaxonomyWriterAnonymousInnerClass2(
        std::shared_ptr<TestDirectoryTaxonomyReader> outerInstance,
        std::shared_ptr<Directory> dir, std::shared_ptr<IndexWriter> iw);

  protected:
    std::shared_ptr<IndexWriter> openIndexWriter(
        std::shared_ptr<Directory> directory,
        std::shared_ptr<IndexWriterConfig> config)  override;

  protected:
    std::shared_ptr<DirectoryTaxonomyWriterAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DirectoryTaxonomyWriterAnonymousInnerClass2>(
          DirectoryTaxonomyWriter::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void
  // testOpenIfChangedNoChangesButSegmentMerges() throws Exception
  virtual void
  testOpenIfChangedNoChangesButSegmentMerges() ;

private:
  class DirectoryTaxonomyWriterAnonymousInnerClass3
      : public DirectoryTaxonomyWriter
  {
    GET_CLASS_NAME(DirectoryTaxonomyWriterAnonymousInnerClass3)
  private:
    std::shared_ptr<TestDirectoryTaxonomyReader> outerInstance;

    std::shared_ptr<IndexWriter> iw;

  public:
    DirectoryTaxonomyWriterAnonymousInnerClass3(
        std::shared_ptr<TestDirectoryTaxonomyReader> outerInstance,
        std::shared_ptr<Directory> dir, std::shared_ptr<IndexWriter> iw);

  protected:
    std::shared_ptr<IndexWriter> openIndexWriter(
        std::shared_ptr<Directory> directory,
        std::shared_ptr<IndexWriterConfig> config)  override;

  protected:
    std::shared_ptr<DirectoryTaxonomyWriterAnonymousInnerClass3>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DirectoryTaxonomyWriterAnonymousInnerClass3>(
          DirectoryTaxonomyWriter::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOpenIfChangedReuseAfterRecreate()
  // throws Exception
  virtual void testOpenIfChangedReuseAfterRecreate() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOpenIfChangedReuse() throws Exception
  virtual void testOpenIfChangedReuse() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOpenIfChangedReplaceTaxonomy() throws
  // Exception
  virtual void testOpenIfChangedReplaceTaxonomy() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetChildren() throws Exception
  virtual void testGetChildren() ;

  virtual void testAccountable() ;

protected:
  std::shared_ptr<TestDirectoryTaxonomyReader> shared_from_this()
  {
    return std::static_pointer_cast<TestDirectoryTaxonomyReader>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy::directory
