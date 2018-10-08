using namespace std;

#include "TestTaxonomyCombined.h"

namespace org::apache::lucene::facet::taxonomy
{
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using SlowRAMDirectory = org::apache::lucene::facet::SlowRAMDirectory;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using Directory = org::apache::lucene::store::Directory;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using org::junit::Test;
std::deque<std::deque<wstring>> const TestTaxonomyCombined::categories = {
    {L"Author", L"Tom Clancy"},
    {L"Author", L"Richard Dawkins"},
    {L"Author", L"Richard Adams"},
    {L"Price", L"10", L"11"},
    {L"Price", L"10", L"12"},
    {L"Price", L"20", L"27"},
    {L"Date", L"2006", L"05"},
    {L"Date", L"2005"},
    {L"Date", L"2006"},
    {L"Subject", L"Nonfiction", L"Children", L"Animals"},
    {L"Author", L"Stephen Jay Gould"},
    {L"Author", L"\u05e0\u05d3\u05d1\u3042\u0628"}};
std::deque<std::deque<int>> const TestTaxonomyCombined::expectedPaths = {
    {1, 2},       {1, 3},   {1, 4},   {5, 6, 7},        {5, 6, 8}, {5, 9, 10},
    {11, 12, 13}, {11, 14}, {11, 12}, {15, 16, 17, 18}, {1, 19},   {1, 20}};
std::deque<std::deque<wstring>> const
    TestTaxonomyCombined::expectedCategories = {
        {},
        {L"Author"},
        {L"Author", L"Tom Clancy"},
        {L"Author", L"Richard Dawkins"},
        {L"Author", L"Richard Adams"},
        {L"Price"},
        {L"Price", L"10"},
        {L"Price", L"10", L"11"},
        {L"Price", L"10", L"12"},
        {L"Price", L"20"},
        {L"Price", L"20", L"27"},
        {L"Date"},
        {L"Date", L"2006"},
        {L"Date", L"2006", L"05"},
        {L"Date", L"2005"},
        {L"Subject"},
        {L"Subject", L"Nonfiction"},
        {L"Subject", L"Nonfiction", L"Children"},
        {L"Subject", L"Nonfiction", L"Children", L"Animals"},
        {L"Author", L"Stephen Jay Gould"},
        {L"Author", L"\u05e0\u05d3\u05d1\u3042\u0628"}};

void TestTaxonomyCombined::fillTaxonomy(shared_ptr<TaxonomyWriter> tw) throw(
    IOException)
{
  for (int i = 0; i < categories.size(); i++) {
    int ordinal = tw->addCategory(make_shared<FacetLabel>(categories[i]));
    int expectedOrdinal = expectedPaths[i][expectedPaths[i].length - 1];
    if (ordinal != expectedOrdinal) {
      fail(L"For category " + showcat(categories[i]) + L" expected ordinal " +
           to_wstring(expectedOrdinal) + L", but got " + to_wstring(ordinal));
    }
  }
}

wstring TestTaxonomyCombined::showcat(std::deque<wstring> &path)
{
  if (path.empty()) {
    return L"<null>";
  }
  if (path.empty()) {
    return L"<empty>";
  }
  if (path.size() == 1 && path[0].length() == 0) {
    return L"<\"\">";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>(path[0]);
  for (int i = 1; i < path.size(); i++) {
    sb->append(L'/');
    sb->append(path[i]);
  }
  return sb->toString();
}

wstring TestTaxonomyCombined::showcat(shared_ptr<FacetLabel> path)
{
  if (path == nullptr) {
    return L"<null>";
  }
  if (path->length == 0) {
    return L"<empty>";
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"<" + path->toString() + L">";
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWriter() throws Exception
void TestTaxonomyCombined::testWriter() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  // Also check TaxonomyWriter.getSize() - see that the taxonomy's size
  // is what we expect it to be.
  assertEquals(expectedCategories.size(), tw->getSize());
  delete tw;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWriterTwice() throws Exception
void TestTaxonomyCombined::testWriterTwice() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  // run fillTaxonomy again - this will try to add the same categories
  // again, and check that we see the same ordinal paths again, not
  // different ones.
  fillTaxonomy(tw);
  // Let's check the number of categories again, to see that no
  // extraneous categories were created:
  assertEquals(expectedCategories.size(), tw->getSize());
  delete tw;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWriterTwice2() throws Exception
void TestTaxonomyCombined::testWriterTwice2() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  delete tw;
  tw = make_shared<DirectoryTaxonomyWriter>(indexDir);
  // run fillTaxonomy again - this will try to add the same categories
  // again, and check that we see the same ordinals again, not different
  // ones, and that the number of categories hasn't grown by the new
  // additions
  fillTaxonomy(tw);
  assertEquals(expectedCategories.size(), tw->getSize());
  delete tw;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWriterTwice3() throws Exception
void TestTaxonomyCombined::testWriterTwice3() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  // First, create and fill the taxonomy
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  delete tw;
  // Now, open the same taxonomy and add the same categories again.
  // After a few categories, the LuceneTaxonomyWriter implementation
  // will stop looking for each category on disk, and rather read them
  // all into memory and close its reader. The bug was that it closed
  // the reader, but forgot that it did (because it didn't set the reader
  // reference to null).
  tw = make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  // Add one new category, just to make commit() do something:
  tw->addCategory(make_shared<FacetLabel>(L"hi"));
  // Do a commit(). Here was a bug - if tw had a reader open, it should
  // be reopened after the commit. However, in our case the reader should
  // not be open (as explained above) but because it was not set to null,
  // we forgot that, tried to reopen it, and got an AlreadyClosedException.
  tw->commit();
  assertEquals(expectedCategories.size() + 1, tw->getSize());
  delete tw;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWriterSimpler() throws Exception
void TestTaxonomyCombined::testWriterSimpler() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  assertEquals(1, tw->getSize()); // the root only
  // Test that adding a new top-level category works
  assertEquals(1, tw->addCategory(make_shared<FacetLabel>(L"a")));
  assertEquals(2, tw->getSize());
  // Test that adding the same category again is noticed, and the
  // same ordinal (and not a new one) is returned.
  assertEquals(1, tw->addCategory(make_shared<FacetLabel>(L"a")));
  assertEquals(2, tw->getSize());
  // Test that adding another top-level category returns a new ordinal,
  // not the same one
  assertEquals(2, tw->addCategory(make_shared<FacetLabel>(L"b")));
  assertEquals(3, tw->getSize());
  // Test that adding a category inside one of the above adds just one
  // new ordinal:
  assertEquals(3, tw->addCategory(make_shared<FacetLabel>(L"a", L"c")));
  assertEquals(4, tw->getSize());
  // Test that adding the same second-level category doesn't do anything:
  assertEquals(3, tw->addCategory(make_shared<FacetLabel>(L"a", L"c")));
  assertEquals(4, tw->getSize());
  // Test that adding a second-level category with two new components
  // indeed adds two categories
  assertEquals(5, tw->addCategory(make_shared<FacetLabel>(L"d", L"e")));
  assertEquals(6, tw->getSize());
  // Verify that the parents were added above in the order we expected
  assertEquals(4, tw->addCategory(make_shared<FacetLabel>(L"d")));
  // Similar, but inside a category that already exists:
  assertEquals(7, tw->addCategory(make_shared<FacetLabel>(L"b", L"d", L"e")));
  assertEquals(8, tw->getSize());
  // And now inside two levels of categories that already exist:
  assertEquals(8, tw->addCategory(make_shared<FacetLabel>(L"b", L"d", L"f")));
  assertEquals(9, tw->getSize());

  delete tw;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRootOnly() throws Exception
void TestTaxonomyCombined::testRootOnly() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  // right after opening the index, it should already contain the
  // root, so have size 1:
  assertEquals(1, tw->getSize());
  delete tw;
  shared_ptr<TaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);
  assertEquals(1, tr->getSize());
  assertEquals(0, tr->getPath(0)->length);
  assertEquals(TaxonomyReader::INVALID_ORDINAL,
               tr->getParallelTaxonomyArrays()->parents()[0]);
  assertEquals(0, tr->getOrdinal(make_shared<FacetLabel>()));
  delete tr;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRootOnly2() throws Exception
void TestTaxonomyCombined::testRootOnly2() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  tw->commit();
  shared_ptr<TaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);
  assertEquals(1, tr->getSize());
  assertEquals(0, tr->getPath(0)->length);
  assertEquals(TaxonomyReader::INVALID_ORDINAL,
               tr->getParallelTaxonomyArrays()->parents()[0]);
  assertEquals(0, tr->getOrdinal(make_shared<FacetLabel>()));
  delete tw;
  delete tr;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testReaderBasic() throws Exception
void TestTaxonomyCombined::testReaderBasic() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  delete tw;
  shared_ptr<TaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);

  // test TaxonomyReader.getSize():
  assertEquals(expectedCategories.size(), tr->getSize());

  // test round trips of ordinal => category => ordinal
  for (int i = 0; i < tr->getSize(); i++) {
    assertEquals(i, tr->getOrdinal(tr->getPath(i)));
  }

  // test TaxonomyReader.getCategory():
  for (int i = 1; i < tr->getSize(); i++) {
    shared_ptr<FacetLabel> expectedCategory =
        make_shared<FacetLabel>(expectedCategories[i]);
    shared_ptr<FacetLabel> category = tr->getPath(i);
    if (!expectedCategory->equals(category)) {
      fail(L"For ordinal " + to_wstring(i) + L" expected category " +
           showcat(expectedCategory) + L", but got " + showcat(category));
    }
  }
  //  (also test invalid ordinals:)
  assertNull(tr->getPath(-1));
  assertNull(tr->getPath(tr->getSize()));
  assertNull(tr->getPath(TaxonomyReader::INVALID_ORDINAL));

  // test TaxonomyReader.getOrdinal():
  for (int i = 1; i < expectedCategories.size(); i++) {
    int expectedOrdinal = i;
    int ordinal =
        tr->getOrdinal(make_shared<FacetLabel>(expectedCategories[i]));
    if (expectedOrdinal != ordinal) {
      fail(L"For category " + showcat(expectedCategories[i]) +
           L" expected ordinal " + to_wstring(expectedOrdinal) + L", but got " +
           to_wstring(ordinal));
    }
  }
  // (also test invalid categories:)
  assertEquals(TaxonomyReader::INVALID_ORDINAL,
               tr->getOrdinal(make_shared<FacetLabel>(L"non-existant")));
  assertEquals(
      TaxonomyReader::INVALID_ORDINAL,
      tr->getOrdinal(make_shared<FacetLabel>(L"Author", L"Jules Verne")));

  delete tr;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testReaderParent() throws Exception
void TestTaxonomyCombined::testReaderParent() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  delete tw;
  shared_ptr<TaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);

  // check that the parent of the root ordinal is the invalid ordinal:
  std::deque<int> parents = tr->getParallelTaxonomyArrays()->parents();
  assertEquals(TaxonomyReader::INVALID_ORDINAL, parents[0]);

  // check parent of non-root ordinals:
  for (int ordinal = 1; ordinal < tr->getSize(); ordinal++) {
    shared_ptr<FacetLabel> me = tr->getPath(ordinal);
    int parentOrdinal = parents[ordinal];
    shared_ptr<FacetLabel> parent = tr->getPath(parentOrdinal);
    if (parent == nullptr) {
      fail(L"Parent of " + to_wstring(ordinal) + L" is " +
           to_wstring(parentOrdinal) + L", but this is not a valid category.");
    }
    // verify that the parent is indeed my parent, according to the strings
    if (!me->subpath(me->length - 1)->equals(parent)) {
      fail(L"Got parent " + to_wstring(parentOrdinal) + L" for ordinal " +
           to_wstring(ordinal) + L" but categories are " + showcat(parent) +
           L" and " + showcat(me) + L" respectively.");
    }
  }

  delete tr;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWriterParent1() throws Exception
void TestTaxonomyCombined::testWriterParent1() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  delete tw;
  tw = make_shared<DirectoryTaxonomyWriter>(indexDir);
  shared_ptr<TaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);

  checkWriterParent(tr, tw);

  delete tw;
  delete tr;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWriterParent2() throws Exception
void TestTaxonomyCombined::testWriterParent2() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  tw->commit();
  shared_ptr<TaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);

  checkWriterParent(tr, tw);

  delete tw;
  delete tr;
  delete indexDir;
}

void TestTaxonomyCombined::checkWriterParent(
    shared_ptr<TaxonomyReader> tr,
    shared_ptr<TaxonomyWriter> tw) 
{
  // check that the parent of the root ordinal is the invalid ordinal:
  assertEquals(TaxonomyReader::INVALID_ORDINAL, tw->getParent(0));

  // check parent of non-root ordinals:
  for (int ordinal = 1; ordinal < tr->getSize(); ordinal++) {
    shared_ptr<FacetLabel> me = tr->getPath(ordinal);
    int parentOrdinal = tw->getParent(ordinal);
    shared_ptr<FacetLabel> parent = tr->getPath(parentOrdinal);
    if (parent == nullptr) {
      fail(L"Parent of " + to_wstring(ordinal) + L" is " +
           to_wstring(parentOrdinal) + L", but this is not a valid category.");
    }
    // verify that the parent is indeed my parent, according to the
    // strings
    if (!me->subpath(me->length - 1)->equals(parent)) {
      fail(L"Got parent " + to_wstring(parentOrdinal) + L" for ordinal " +
           to_wstring(ordinal) + L" but categories are " + showcat(parent) +
           L" and " + showcat(me) + L" respectively.");
    }
  }

  // check parent of of invalid ordinals:
  expectThrows(out_of_range::typeid, [&]() { tw->getParent(-1); });
  expectThrows(out_of_range::typeid,
               [&]() { tw->getParent(TaxonomyReader::INVALID_ORDINAL); });
  expectThrows(out_of_range::typeid, [&]() { tw->getParent(tr->getSize()); });
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testChildrenArrays() throws Exception
void TestTaxonomyCombined::testChildrenArrays() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  delete tw;
  shared_ptr<TaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);
  shared_ptr<ParallelTaxonomyArrays> ca = tr->getParallelTaxonomyArrays();
  std::deque<int> youngestChildArray = ca->children();
  assertEquals(tr->getSize(), youngestChildArray.size());
  std::deque<int> olderSiblingArray = ca->siblings();
  assertEquals(tr->getSize(), olderSiblingArray.size());
  for (int i = 0; i < expectedCategories.size(); i++) {
    // find expected children by looking at all expectedCategories
    // for children
    deque<int> expectedChildren = deque<int>();
    for (int j = expectedCategories.size() - 1; j >= 0; j--) {
      if (expectedCategories[j].length != expectedCategories[i].length + 1) {
        continue; // not longer by 1, so can't be a child
      }
      bool ischild = true;
      for (int k = 0; k < expectedCategories[i].length; k++) {
        if (expectedCategories[j][k] != expectedCategories[i][k]) {
          ischild = false;
          break;
        }
      }
      if (ischild) {
        expectedChildren.push_back(j);
      }
    }
    // check that children and expectedChildren are the same, with the
    // correct reverse (youngest to oldest) order:
    if (expectedChildren.empty()) {
      assertEquals(TaxonomyReader::INVALID_ORDINAL, youngestChildArray[i]);
    } else {
      int child = youngestChildArray[i];
      assertEquals(expectedChildren[0], child);
      for (int j = 1; j < expectedChildren.size(); j++) {
        child = olderSiblingArray[child];
        assertEquals(expectedChildren[j], child);
        // if child is INVALID_ORDINAL we should stop, but
        // assertEquals would fail in this case anyway.
      }
      // When we're done comparing, olderSiblingArray should now point
      // to INVALID_ORDINAL, saying there are no more children. If it
      // doesn't, we found too many children...
      assertEquals(-1, olderSiblingArray[child]);
    }
  }
  delete tr;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testChildrenArraysInvariants() throws
// Exception
void TestTaxonomyCombined::testChildrenArraysInvariants() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  delete tw;
  shared_ptr<TaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);
  shared_ptr<ParallelTaxonomyArrays> ca = tr->getParallelTaxonomyArrays();
  std::deque<int> children = ca->children();
  assertEquals(tr->getSize(), children.size());
  std::deque<int> olderSiblingArray = ca->siblings();
  assertEquals(tr->getSize(), olderSiblingArray.size());

  // test that the "youngest child" of every category is indeed a child:
  std::deque<int> parents = tr->getParallelTaxonomyArrays()->parents();
  for (int i = 0; i < tr->getSize(); i++) {
    int youngestChild = children[i];
    if (youngestChild != TaxonomyReader::INVALID_ORDINAL) {
      assertEquals(i, parents[youngestChild]);
    }
  }

  // test that the "older sibling" of every category is indeed older (lower)
  // (it can also be INVALID_ORDINAL, which is lower than any ordinal)
  for (int i = 0; i < tr->getSize(); i++) {
    assertTrue(L"olderSiblingArray[" + to_wstring(i) + L"] should be <" +
                   to_wstring(i),
               olderSiblingArray[i] < i);
  }

  // test that the "older sibling" of every category is indeed a sibling
  // (they share the same parent)
  for (int i = 0; i < tr->getSize(); i++) {
    int sibling = olderSiblingArray[i];
    if (sibling == TaxonomyReader::INVALID_ORDINAL) {
      continue;
    }
    assertEquals(parents[i], parents[sibling]);
  }

  // And now for slightly more complex (and less "invariant-like"...)
  // tests:

  // test that the "youngest child" is indeed the youngest (so we don't
  // miss the first children in the chain)
  for (int i = 0; i < tr->getSize(); i++) {
    // Find the really youngest child:
    int j;
    for (j = tr->getSize() - 1; j > i; j--) {
      if (parents[j] == i) {
        break; // found youngest child
      }
    }
    if (j == i) { // no child found
      j = TaxonomyReader::INVALID_ORDINAL;
    }
    assertEquals(j, children[i]);
  }

  // test that the "older sibling" is indeed the least oldest one - and
  // not a too old one or -1 (so we didn't miss some children in the
  // middle or the end of the chain).
  for (int i = 0; i < tr->getSize(); i++) {
    // Find the youngest older sibling:
    int j;
    for (j = i - 1; j >= 0; j--) {
      if (parents[j] == parents[i]) {
        break; // found youngest older sibling
      }
    }
    if (j < 0) { // no sibling found
      j = TaxonomyReader::INVALID_ORDINAL;
    }
    assertEquals(j, olderSiblingArray[i]);
  }

  delete tr;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testChildrenArraysGrowth() throws Exception
void TestTaxonomyCombined::testChildrenArraysGrowth() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  tw->addCategory(make_shared<FacetLabel>(L"hi", L"there"));
  tw->commit();
  shared_ptr<TaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);
  shared_ptr<ParallelTaxonomyArrays> ca = tr->getParallelTaxonomyArrays();
  assertEquals(3, tr->getSize());
  assertEquals(3, ca->siblings().size());
  assertEquals(3, ca->children().size());
  assertTrue(Arrays::equals(std::deque<int>{1, 2, -1}, ca->children()));
  assertTrue(Arrays::equals(std::deque<int>{-1, -1, -1}, ca->siblings()));
  tw->addCategory(make_shared<FacetLabel>(L"hi", L"ho"));
  tw->addCategory(make_shared<FacetLabel>(L"hello"));
  tw->commit();
  // Before refresh, nothing changed..
  shared_ptr<ParallelTaxonomyArrays> newca = tr->getParallelTaxonomyArrays();
  assertSame(newca, ca); // we got exactly the same object
  assertEquals(3, tr->getSize());
  assertEquals(3, ca->siblings().size());
  assertEquals(3, ca->children().size());
  // After the refresh, things change:
  shared_ptr<TaxonomyReader> newtr = TaxonomyReader::openIfChanged(tr);
  assertNotNull(newtr);
  delete tr;
  tr = newtr;
  ca = tr->getParallelTaxonomyArrays();
  assertEquals(5, tr->getSize());
  assertEquals(5, ca->siblings().size());
  assertEquals(5, ca->children().size());
  assertTrue(
      Arrays::equals(std::deque<int>{4, 3, -1, -1, -1}, ca->children()));
  assertTrue(
      Arrays::equals(std::deque<int>{-1, -1, -1, 2, 1}, ca->siblings()));
  delete tw;
  delete tr;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTaxonomyReaderRefreshRaces() throws
// Exception
void TestTaxonomyCombined::testTaxonomyReaderRefreshRaces() 
{
  // compute base child arrays - after first chunk, and after the other
  shared_ptr<Directory> indexDirBase = newDirectory();
  shared_ptr<TaxonomyWriter> twBase =
      make_shared<DirectoryTaxonomyWriter>(indexDirBase);
  twBase->addCategory(make_shared<FacetLabel>(L"a", L"0"));
  shared_ptr<FacetLabel> *const abPath = make_shared<FacetLabel>(L"a", L"b");
  twBase->addCategory(abPath);
  twBase->commit();
  shared_ptr<TaxonomyReader> trBase =
      make_shared<DirectoryTaxonomyReader>(indexDirBase);

  shared_ptr<ParallelTaxonomyArrays> *const ca1 =
      trBase->getParallelTaxonomyArrays();

  constexpr int abOrd = trBase->getOrdinal(abPath);
  constexpr int abYoungChildBase1 = ca1->children()[abOrd];

  constexpr int numCategories = atLeast(800);
  for (int i = 0; i < numCategories; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    twBase->addCategory(
        make_shared<FacetLabel>(L"a", L"b", Integer::toString(i)));
  }
  delete twBase;

  shared_ptr<TaxonomyReader> newTaxoReader =
      TaxonomyReader::openIfChanged(trBase);
  assertNotNull(newTaxoReader);
  delete trBase;
  trBase = newTaxoReader;

  shared_ptr<ParallelTaxonomyArrays> *const ca2 =
      trBase->getParallelTaxonomyArrays();
  constexpr int abYoungChildBase2 = ca2->children()[abOrd];

  int numRetries = atLeast(50);
  for (int retry = 0; retry < numRetries; retry++) {
    assertConsistentYoungestChild(abPath, abOrd, abYoungChildBase1,
                                  abYoungChildBase2, retry, numCategories);
  }

  delete trBase;
  delete indexDirBase;
}

void TestTaxonomyCombined::assertConsistentYoungestChild(
    shared_ptr<FacetLabel> abPath, int const abOrd, int const abYoungChildBase1,
    int const abYoungChildBase2, int const retry,
    int numCategories) 
{
  shared_ptr<SlowRAMDirectory> indexDir = make_shared<SlowRAMDirectory>(
      -1, nullptr); // no slowness for initialization
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  tw->addCategory(make_shared<FacetLabel>(L"a", L"0"));
  tw->addCategory(abPath);
  tw->commit();

  shared_ptr<DirectoryTaxonomyReader> *const tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);
  for (int i = 0; i < numCategories; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<FacetLabel> *const cp =
        make_shared<FacetLabel>(L"a", L"b", Integer::toString(i));
    tw->addCategory(cp);
    assertEquals(L"Ordinal of " + cp +
                     L" must be invalid until Taxonomy Reader was refreshed",
                 TaxonomyReader::INVALID_ORDINAL, tr->getOrdinal(cp));
  }
  delete tw;

  shared_ptr<AtomicBoolean> *const stop = make_shared<AtomicBoolean>(false);
  const std::deque<runtime_error> error = std::deque<runtime_error>{nullptr};
  const std::deque<int> retrieval = {0};

  shared_ptr<Thread> thread = make_shared<ThreadAnonymousInnerClass>(
      shared_from_this(), abPath, abOrd, abYoungChildBase1, abYoungChildBase2,
      retry, tr, stop, error, retrieval);
  thread->start();

  indexDir->setSleepMillis(1); // some delay for refresh
  shared_ptr<TaxonomyReader> newTaxoReader = TaxonomyReader::openIfChanged(tr);
  if (newTaxoReader != nullptr) {
    delete newTaxoReader;
  }

  stop->set(true);
  thread->join();
  assertNull(L"Unexpcted exception at retry " + to_wstring(retry) +
                 L" retrieval " + to_wstring(retrieval[0]) + L": \n" +
                 stackTraceStr(error[0]),
             error[0]);

  delete tr;
}

TestTaxonomyCombined::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestTaxonomyCombined> outerInstance,
    shared_ptr<org::apache::lucene::facet::taxonomy::FacetLabel> abPath,
    int abOrd, int abYoungChildBase1, int abYoungChildBase2, int retry,
    shared_ptr<DirectoryTaxonomyReader> tr, shared_ptr<AtomicBoolean> stop,
    deque<runtime_error> &error, deque<int> &retrieval)
    : Thread(L"Child Arrays Verifier")
{
  this->outerInstance = outerInstance;
  this->abPath = abPath;
  this->abOrd = abOrd;
  this->abYoungChildBase1 = abYoungChildBase1;
  this->abYoungChildBase2 = abYoungChildBase2;
  this->retry = retry;
  this->tr = tr;
  this->stop = stop;
  this->error = error;
  this->retrieval = retrieval;
}

void TestTaxonomyCombined::ThreadAnonymousInnerClass::run()
{
  setPriority(1 + getPriority());
  try {
    while (!stop->get()) {
      int lastOrd = tr->getParallelTaxonomyArrays()->parents().size() - 1;
      assertNotNull(L"path of last-ord " + to_wstring(lastOrd) +
                        L" is not found!",
                    tr->getPath(lastOrd));
      assertChildrenArrays(tr->getParallelTaxonomyArrays(), retry,
                           retrieval[0]++);
      sleep(10); // don't starve refresh()'s CPU, which sleeps every 50 bytes
                 // for 1 ms
    }
  } catch (const runtime_error &e) {
    error[0] = e;
    stop->set(true);
  }
}

void TestTaxonomyCombined::ThreadAnonymousInnerClass::assertChildrenArrays(
    shared_ptr<ParallelTaxonomyArrays> ca, int retry, int retrieval)
{
  constexpr int abYoungChild = ca->children()[abOrd];
  assertTrue(L"Retry " + to_wstring(retry) + L": retrieval: " +
                 to_wstring(retrieval) +
                 L": wrong youngest child for category " + abPath + L" (ord=" +
                 to_wstring(abOrd) + L") - must be either " +
                 to_wstring(abYoungChildBase1) + L" or " +
                 to_wstring(abYoungChildBase2) + L" but was: " +
                 to_wstring(abYoungChild),
             abYoungChildBase1 == abYoungChild ||
                 abYoungChildBase2 == ca->children()[abOrd]);
}

wstring TestTaxonomyCombined::stackTraceStr(runtime_error const error)
{
  if (error == nullptr) {
    return L"";
  }
  shared_ptr<StringWriter> sw = make_shared<StringWriter>();
  shared_ptr<PrintWriter> pw = make_shared<PrintWriter>(sw);
  error.printStackTrace(pw);
  pw->close();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return sw->toString();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSeparateReaderAndWriter() throws
// Exception
void TestTaxonomyCombined::testSeparateReaderAndWriter() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  tw->commit();
  shared_ptr<TaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);

  assertEquals(1, tr->getSize()); // the empty taxonomy has size 1 (the root)
  tw->addCategory(make_shared<FacetLabel>(L"Author"));
  assertEquals(1, tr->getSize()); // still root only...
  assertNull(TaxonomyReader::openIfChanged(
      tr)); // this is not enough, because tw.commit() hasn't been done yet
  assertEquals(1, tr->getSize()); // still root only...
  tw->commit();
  assertEquals(1, tr->getSize()); // still root only...
  shared_ptr<TaxonomyReader> newTaxoReader = TaxonomyReader::openIfChanged(tr);
  assertNotNull(newTaxoReader);
  delete tr;
  tr = newTaxoReader;

  int author = 1;
  try {
    assertEquals(TaxonomyReader::ROOT_ORDINAL,
                 tr->getParallelTaxonomyArrays()->parents()[author]);
    // ok
  } catch (const out_of_range &e) {
    fail(L"After category addition, commit() and refresh(), getParent for " +
         to_wstring(author) + L" should NOT throw exception");
  }
  assertEquals(2, tr->getSize()); // finally, see there are two categories

  // now, add another category, and verify that after commit and refresh
  // the parent of this category is correct (this requires the reader
  // to correctly update its prefetched parent deque), and that the
  // old information also wasn't ruined:
  tw->addCategory(make_shared<FacetLabel>(L"Author", L"Richard Dawkins"));
  int dawkins = 2;
  tw->commit();
  newTaxoReader = TaxonomyReader::openIfChanged(tr);
  assertNotNull(newTaxoReader);
  delete tr;
  tr = newTaxoReader;
  std::deque<int> parents = tr->getParallelTaxonomyArrays()->parents();
  assertEquals(author, parents[dawkins]);
  assertEquals(TaxonomyReader::ROOT_ORDINAL, parents[author]);
  assertEquals(TaxonomyReader::INVALID_ORDINAL,
               parents[TaxonomyReader::ROOT_ORDINAL]);
  assertEquals(3, tr->getSize());
  delete tw;
  delete tr;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSeparateReaderAndWriter2() throws
// Exception
void TestTaxonomyCombined::testSeparateReaderAndWriter2() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  tw->commit();
  shared_ptr<TaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(indexDir);

  // Test getOrdinal():
  shared_ptr<FacetLabel> author = make_shared<FacetLabel>(L"Author");

  assertEquals(1, tr->getSize()); // the empty taxonomy has size 1 (the root)
  assertEquals(TaxonomyReader::INVALID_ORDINAL, tr->getOrdinal(author));
  tw->addCategory(author);
  // before commit and refresh, no change:
  assertEquals(TaxonomyReader::INVALID_ORDINAL, tr->getOrdinal(author));
  assertEquals(1, tr->getSize()); // still root only...
  assertNull(TaxonomyReader::openIfChanged(
      tr)); // this is not enough, because tw.commit() hasn't been done yet
  assertEquals(TaxonomyReader::INVALID_ORDINAL, tr->getOrdinal(author));
  assertEquals(1, tr->getSize()); // still root only...
  tw->commit();
  // still not enough before refresh:
  assertEquals(TaxonomyReader::INVALID_ORDINAL, tr->getOrdinal(author));
  assertEquals(1, tr->getSize()); // still root only...
  shared_ptr<TaxonomyReader> newTaxoReader = TaxonomyReader::openIfChanged(tr);
  assertNotNull(newTaxoReader);
  delete tr;
  tr = newTaxoReader;
  assertEquals(1, tr->getOrdinal(author));
  assertEquals(2, tr->getSize());
  delete tw;
  delete tr;
  delete indexDir;
}

void TestTaxonomyCombined::fillTaxonomyCheckPaths(
    shared_ptr<TaxonomyWriter> tw) 
{
  for (int i = 0; i < categories.size(); i++) {
    int ordinal = tw->addCategory(make_shared<FacetLabel>(categories[i]));
    int expectedOrdinal = expectedPaths[i][expectedPaths[i].length - 1];
    if (ordinal != expectedOrdinal) {
      fail(L"For category " + showcat(categories[i]) + L" expected ordinal " +
           to_wstring(expectedOrdinal) + L", but got " + to_wstring(ordinal));
    }
    for (int j = expectedPaths[i].length - 2; j >= 0; j--) {
      ordinal = tw->getParent(ordinal);
      expectedOrdinal = expectedPaths[i][j];
      if (ordinal != expectedOrdinal) {
        fail(L"For category " + showcat(categories[i]) +
             L" expected ancestor level " + (expectedPaths[i].length - 1 - j) +
             L" was " + to_wstring(expectedOrdinal) + L", but got " +
             to_wstring(ordinal));
      }
    }
  }
}

void TestTaxonomyCombined::checkPaths(shared_ptr<TaxonomyWriter> tw) throw(
    IOException)
{
  for (int i = 0; i < categories.size(); i++) {
    int ordinal = expectedPaths[i][expectedPaths[i].length - 1];
    for (int j = expectedPaths[i].length - 2; j >= 0; j--) {
      ordinal = tw->getParent(ordinal);
      int expectedOrdinal = expectedPaths[i][j];
      if (ordinal != expectedOrdinal) {
        fail(L"For category " + showcat(categories[i]) +
             L" expected ancestor level " + (expectedPaths[i].length - 1 - j) +
             L" was " + to_wstring(expectedOrdinal) + L", but got " +
             to_wstring(ordinal));
      }
    }
    assertEquals(TaxonomyReader::ROOT_ORDINAL,
                 tw->getParent(expectedPaths[i][0]));
  }
  assertEquals(TaxonomyReader::INVALID_ORDINAL,
               tw->getParent(TaxonomyReader::ROOT_ORDINAL));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWriterCheckPaths() throws Exception
void TestTaxonomyCombined::testWriterCheckPaths() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomyCheckPaths(tw);
  // Also check TaxonomyWriter.getSize() - see that the taxonomy's size
  // is what we expect it to be.
  assertEquals(expectedCategories.size(), tw->getSize());
  delete tw;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWriterCheckPaths2() throws Exception
void TestTaxonomyCombined::testWriterCheckPaths2() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(indexDir);
  fillTaxonomy(tw);
  checkPaths(tw);
  fillTaxonomy(tw);
  checkPaths(tw);
  delete tw;

  tw = make_shared<DirectoryTaxonomyWriter>(indexDir);
  checkPaths(tw);
  fillTaxonomy(tw);
  checkPaths(tw);
  delete tw;
  delete indexDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNRT() throws Exception
void TestTaxonomyCombined::testNRT() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> writer =
      make_shared<DirectoryTaxonomyWriter>(dir);
  shared_ptr<TaxonomyReader> reader =
      make_shared<DirectoryTaxonomyReader>(writer);

  shared_ptr<FacetLabel> cp = make_shared<FacetLabel>(L"a");
  writer->addCategory(cp);
  shared_ptr<TaxonomyReader> newReader = TaxonomyReader::openIfChanged(reader);
  assertNotNull(L"expected a new instance", newReader);
  assertEquals(2, newReader->getSize());
  assertNotSame(TaxonomyReader::INVALID_ORDINAL, newReader->getOrdinal(cp));
  delete reader;
  reader = newReader;

  delete writer;
  delete reader;

  delete dir;
}
} // namespace org::apache::lucene::facet::taxonomy