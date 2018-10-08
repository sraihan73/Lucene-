using namespace std;

#include "TestMergeRateLimiter.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMergeRateLimiter::testInitDefaults() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  w->addDocument(make_shared<Document>());
  delete w;

  shared_ptr<MergeRateLimiter> rateLimiter = make_shared<MergeRateLimiter>(
      make_shared<MergePolicy::OneMergeProgress>());
  assertEquals(numeric_limits<double>::infinity(), rateLimiter->getMBPerSec(),
               0.0);
  assertTrue(rateLimiter->getMinPauseCheckBytes() > 0);
  delete dir;
}
} // namespace org::apache::lucene::index