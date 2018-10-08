#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyWriter.h"

#include  "core/src/java/org/apache/lucene/facet/taxonomy/FacetLabel.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/directory/DirectoryTaxonomyReader.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/ParallelTaxonomyArrays.h"

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
namespace org::apache::lucene::facet::taxonomy
{

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs("SimpleText") public class
// TestTaxonomyCombined extends org.apache.lucene.facet.FacetTestCase
class TestTaxonomyCombined : public FacetTestCase
{

  /**  The following categories will be added to the taxonomy by
    fillTaxonomy(), and tested by all tests below:
  */
private:
  static std::deque<std::deque<std::wstring>> const categories;

  /**  When adding the above categories with TaxonomyWriter.addCategory(),
    the following paths are expected to be returned:
    (note that currently the full path is not returned, and therefore
    not tested - rather, just the last component, the ordinal, is returned
    and tested.
  */
  static std::deque<std::deque<int>> const expectedPaths;

  /**  The taxonomy index is expected to then contain the following
    generated categories, with increasing ordinals (note how parent
    categories are be added automatically when subcategories are added).
   */
  static std::deque<std::deque<std::wstring>> const expectedCategories;

  /**  fillTaxonomy adds the categories in the categories[] array, and asserts
    that the additions return exactly the ordinals (in the past - paths)
    specified in expectedPaths[].
    Note that this assumes that fillTaxonomy() is called on an empty taxonomy
    index. Calling it after something else was already added to the taxonomy
    index will surely have this method fail.
   */
public:
  static void
  fillTaxonomy(std::shared_ptr<TaxonomyWriter> tw) ;

  static std::wstring showcat(std::deque<std::wstring> &path);

private:
  std::wstring showcat(std::shared_ptr<FacetLabel> path);

  /**  Basic tests for TaxonomyWriter. Basically, we test that
    IndexWriter.addCategory works, i.e. returns the expected ordinals
    (this is tested by calling the fillTaxonomy() method above).
    We do not test here that after writing the index can be read -
    this will be done in more tests below.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWriter() throws Exception
  virtual void testWriter() ;

  /**  testWriterTwice is exactly like testWriter, except that after adding
    all the categories, we add them again, and see that we get the same
    old ids again - not new categories.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWriterTwice() throws Exception
  virtual void testWriterTwice() ;

  /**  testWriterTwice2 is similar to testWriterTwice, except that the index
    is closed and reopened before attempting to write to it the same
    categories again. While testWriterTwice can get along with writing
    and reading correctly just to the cache, testWriterTwice2 checks also
    the actual disk read part of the writer:
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWriterTwice2() throws Exception
  virtual void testWriterTwice2() ;

  /**
   * testWriterTwice3 is yet another test which tests creating a taxonomy
   * in two separate writing sessions. This test used to fail because of
   * a bug involving commit(), explained below, and now should succeed.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWriterTwice3() throws Exception
  virtual void testWriterTwice3() ;

  /**  Another set of tests for the writer, which don't use an array and
   *  try to distill the different cases, and therefore may be more helpful
   *  for debugging a problem than testWriter() which is hard to know why
   *  or where it failed.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWriterSimpler() throws Exception
  virtual void testWriterSimpler() ;

  /**  Test writing an empty index, and seeing that a reader finds in it
    the root category, and only it. We check all the methods on that
    root category return the expected results.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRootOnly() throws Exception
  virtual void testRootOnly() ;

  /**  The following test is exactly the same as testRootOnly, except we
   *  do not close the writer before opening the reader. We want to see
   *  that the root is visible to the reader not only after the writer is
   *  closed, but immediately after it is created.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRootOnly2() throws Exception
  virtual void testRootOnly2() ;

  /**  Basic tests for TaxonomyReader's category &lt;=&gt; ordinal
    transformations (getSize(), getCategory() and getOrdinal()). We test that
    after writing the index, it can be read and all the categories and ordinals
    are there just as we expected them to be.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testReaderBasic() throws Exception
  virtual void testReaderBasic() ;

  /**  Tests for TaxonomyReader's getParent() method.
    We check it by comparing its results to those we could have gotten by
    looking at the category string paths (where the parentage is obvious).
    Note that after testReaderBasic(), we already know we can trust the
    ordinal &lt;=&gt; category conversions.

    Note: At the moment, the parent methods in the reader are deprecated,
    but this does not mean they should not be tested! Until they are
    removed (*if* they are removed), these tests should remain to see
    that they still work correctly.
   */

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testReaderParent() throws Exception
  virtual void testReaderParent() ;

  /**
   * Tests for TaxonomyWriter's getParent() method. We check it by comparing
   * its results to those we could have gotten by looking at the category
   * string paths using a TaxonomyReader (where the parentage is obvious).
   * Note that after testReaderBasic(), we already know we can trust the
   * ordinal &lt;=&gt; category conversions from TaxonomyReader.
   *
   * The difference between testWriterParent1 and testWriterParent2 is that
   * the former closes the taxonomy writer before reopening it, while the
   * latter does not.
   *
   * This test code is virtually identical to that of testReaderParent().
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWriterParent1() throws Exception
  virtual void testWriterParent1() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWriterParent2() throws Exception
  virtual void testWriterParent2() ;

private:
  void checkWriterParent(
      std::shared_ptr<TaxonomyReader> tr,
      std::shared_ptr<TaxonomyWriter> tw) ;

  /**
   * Test TaxonomyReader's child browsing method, getChildrenArrays()
   * This only tests for correctness of the data on one example - we have
   * below further tests on data refresh etc.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testChildrenArrays() throws Exception
  virtual void testChildrenArrays() ;

  /**
   * Similar to testChildrenArrays, except rather than look at
   * expected results, we test for several "invariants" that the results
   * should uphold, e.g., that a child of a category indeed has this category
   * as its parent. This sort of test can more easily be extended to larger
   * example taxonomies, because we do not need to build the expected deque
   * of categories like we did in the above test.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testChildrenArraysInvariants() throws
  // Exception
  virtual void testChildrenArraysInvariants() ;

  /**
   * Test how getChildrenArrays() deals with the taxonomy's growth:
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testChildrenArraysGrowth() throws
  // Exception
  virtual void testChildrenArraysGrowth() ;

  // Test that getParentArrays is valid when retrieved during refresh
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTaxonomyReaderRefreshRaces() throws
  // Exception
  virtual void testTaxonomyReaderRefreshRaces() ;

private:
  void
  assertConsistentYoungestChild(std::shared_ptr<FacetLabel> abPath,
                                int const abOrd, int const abYoungChildBase1,
                                int const abYoungChildBase2, int const retry,
                                int numCategories) ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestTaxonomyCombined> outerInstance;

    std::shared_ptr<org::apache::lucene::facet::taxonomy::FacetLabel> abPath;
    int abOrd = 0;
    int abYoungChildBase1 = 0;
    int abYoungChildBase2 = 0;
    int retry = 0;
    std::shared_ptr<DirectoryTaxonomyReader> tr;
    std::shared_ptr<AtomicBoolean> stop;
    std::deque<std::runtime_error> error;
    std::deque<int> retrieval;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestTaxonomyCombined> outerInstance,
        std::shared_ptr<org::apache::lucene::facet::taxonomy::FacetLabel>
            abPath,
        int abOrd, int abYoungChildBase1, int abYoungChildBase2, int retry,
        std::shared_ptr<DirectoryTaxonomyReader> tr,
        std::shared_ptr<AtomicBoolean> stop,
        std::deque<std::runtime_error> &error, std::deque<int> &retrieval);

    void run() override;

  private:
    void assertChildrenArrays(std::shared_ptr<ParallelTaxonomyArrays> ca,
                              int retry, int retrieval);

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  /** Grab the stack trace into a string since the exception was thrown in a
   * thread and we want the assert
   * outside the thread to show the stack trace in case of failure.   */
private:
  std::wstring stackTraceStr(std::runtime_error const error);

  /**  Test that if separate reader and writer objects are opened, new
    categories written into the writer are available to a reader only
    after a commit().
    Note that this test obviously doesn't cover all the different
    concurrency scenarios, all different methods, and so on. We may
    want to write more tests of this sort.

    This test simulates what would happen when there are two separate
    processes, one doing indexing, and the other searching, and each opens
    its own object (with obviously no connection between the objects) using
    the same disk files. Note, though, that this test does not test what
    happens when the two processes do their actual work at exactly the same
    time.
    It also doesn't test multi-threading.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSeparateReaderAndWriter() throws
  // Exception
  virtual void testSeparateReaderAndWriter() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSeparateReaderAndWriter2() throws
  // Exception
  virtual void testSeparateReaderAndWriter2() ;

  /**
   * fillTaxonomyCheckPaths adds the categories in the categories[] array,
   * and asserts that the additions return exactly paths specified in
   * expectedPaths[]. This is the same add fillTaxonomy() but also checks
   * the correctness of getParent(), not just addCategory().
   * Note that this assumes that fillTaxonomyCheckPaths() is called on an empty
   * taxonomy index. Calling it after something else was already added to the
   * taxonomy index will surely have this method fail.
   */
  static void
  fillTaxonomyCheckPaths(std::shared_ptr<TaxonomyWriter> tw) ;

  // After fillTaxonomy returned successfully, checkPaths() checks that
  // the getParent() calls return as expected, from the table
  static void checkPaths(std::shared_ptr<TaxonomyWriter> tw) ;

  /**
   * Basic test for TaxonomyWriter.getParent(). This is similar to testWriter
   * above, except we also check the parents of the added categories, not just
   * the categories themselves.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWriterCheckPaths() throws Exception
  virtual void testWriterCheckPaths() ;

  /**
   * testWriterCheckPaths2 is the path-checking variant of testWriterTwice
   * and testWriterTwice2. After adding all the categories, we add them again,
   * and see that we get the same old ids and paths. We repeat the path checking
   * yet again after closing and opening the index for writing again - to see
   * that the reading of existing data from disk works as well.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWriterCheckPaths2() throws Exception
  virtual void testWriterCheckPaths2() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNRT() throws Exception
  virtual void testNRT() ;

  //  TODO (Facet): test multiple readers, one writer. Have the multiple readers
  //  using the same object (simulating threads) or different objects
  //  (simulating processes).

protected:
  std::shared_ptr<TestTaxonomyCombined> shared_from_this()
  {
    return std::static_pointer_cast<TestTaxonomyCombined>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
