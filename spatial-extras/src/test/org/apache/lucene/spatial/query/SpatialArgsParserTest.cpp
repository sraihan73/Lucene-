using namespace std;

#include "SpatialArgsParserTest.h"

namespace org::apache::lucene::spatial::query
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Rectangle;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testArgsParser() throws Exception
void SpatialArgsParserTest::testArgsParser() 
{
  shared_ptr<SpatialArgsParser> parser = make_shared<SpatialArgsParser>();

  wstring arg = SpatialOperation::IsWithin + L"(Envelope(-10, 10, 20, -20))";
  shared_ptr<SpatialArgs> out = parser->parse(arg, ctx);
  assertEquals(SpatialOperation::IsWithin, out->getOperation());
  shared_ptr<Rectangle> bounds =
      std::static_pointer_cast<Rectangle>(out->getShape());
  assertEquals(-10.0, bounds->getMinX(), 0);
  assertEquals(10.0, bounds->getMaxX(), 0);

  // Disjoint should not be scored
  arg = SpatialOperation::IsDisjointTo + L" (Envelope(-10,-20,20,10))";
  out = parser->parse(arg, ctx);
  assertEquals(SpatialOperation::IsDisjointTo, out->getOperation());

  // spatial operations need args
  expectThrows(runtime_error::typeid, [&]() {
    parser->parse(SpatialOperation::IsDisjointTo + L"[ ]", ctx);
  });

  // unknown operation
  expectThrows(runtime_error::typeid, [&]() {
    parser->parse(L"XXXX(Envelope(-10, 10, 20, -20))", ctx);
  });

  assertAlias(SpatialOperation::IsWithin, L"CoveredBy");
  assertAlias(SpatialOperation::IsWithin, L"COVEREDBY");
  assertAlias(SpatialOperation::IsWithin, L"coveredBy");
  assertAlias(SpatialOperation::IsWithin, L"Within");
  assertAlias(SpatialOperation::IsEqualTo, L"Equals");
  assertAlias(SpatialOperation::IsDisjointTo, L"disjoint");
  assertAlias(SpatialOperation::Contains, L"Covers");
}

void SpatialArgsParserTest::assertAlias(
    shared_ptr<SpatialOperation> op, const wstring &name) 
{
  wstring arg;
  shared_ptr<SpatialArgs> out;
  arg = name + L"(Point(0 0))";
  out = (make_shared<SpatialArgsParser>())->parse(arg, ctx);
  assertEquals(op, out->getOperation());
}
} // namespace org::apache::lucene::spatial::query