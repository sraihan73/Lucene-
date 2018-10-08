using namespace std;

#include "TestLruTaxonomyWriterCache.h"

namespace org::apache::lucene::facet::taxonomy::writercache
{
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDefaultLRUTypeIsCollisionSafe()
void TestLruTaxonomyWriterCache::testDefaultLRUTypeIsCollisionSafe()
{
  // These labels are clearly different, but have identical longHashCodes.
  // Note that these labels are clearly contrived. We did encounter
  // collisions in actual production data, but we aren't allowed to publish
  // those.
  shared_ptr<FacetLabel> *const a =
      make_shared<FacetLabel>(L"\0", L"\u0003\uFFE2");
  shared_ptr<FacetLabel> *const b = make_shared<FacetLabel>(L"\1", L"\0");
  // If this fails, then the longHashCode implementation has changed. This
  // cannot prevent collisions. (All hashes must allow for collisions.) It
  // will however stop the rest of this test from making sense. To fix, find
  // new colliding labels, or make a subclass of FacetLabel that produces
  // collisions.
  assertEquals(a->longHashCode(), b->longHashCode());
  // Make a cache with capacity > 2 so both our labels will fit. Don't
  // specify an LRUType, since we want to check if the default is
  // collision-safe.
  shared_ptr<LruTaxonomyWriterCache> *const cache =
      make_shared<LruTaxonomyWriterCache>(10);
  cache->put(a, 0);
  cache->put(b, 1);
  assertEquals(cache->get(a), 0);
  assertEquals(cache->get(b), 1);
}
} // namespace org::apache::lucene::facet::taxonomy::writercache