using namespace std;

#include "TestCheckIndex.h"

namespace org::apache::lucene::index
{
using Directory = org::apache::lucene::store::Directory;
using org::junit::Test;

void TestCheckIndex::setUp() 
{
  BaseTestCheckIndex::setUp();
  directory = newDirectory();
}

void TestCheckIndex::tearDown() 
{
  delete directory;
  BaseTestCheckIndex::tearDown();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDeletedDocs() throws java.io.IOException
void TestCheckIndex::testDeletedDocs() 
{
  testDeletedDocs(directory);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testChecksumsOnly() throws
// java.io.IOException
void TestCheckIndex::testChecksumsOnly() 
{
  testChecksumsOnly(directory);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testChecksumsOnlyVerbose() throws
// java.io.IOException
void TestCheckIndex::testChecksumsOnlyVerbose() 
{
  testChecksumsOnlyVerbose(directory);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testObtainsLock() throws java.io.IOException
void TestCheckIndex::testObtainsLock() 
{
  testObtainsLock(directory);
}
} // namespace org::apache::lucene::index