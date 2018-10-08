using namespace std;

#include "SpatialOperation.h"

namespace org::apache::lucene::spatial::query
{
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
const unordered_map<wstring, std::shared_ptr<SpatialOperation>>
    SpatialOperation::registry =
        unordered_map<wstring, std::shared_ptr<SpatialOperation>>();
const deque<std::shared_ptr<SpatialOperation>> SpatialOperation::deque =
    deque<std::shared_ptr<SpatialOperation>>();
const shared_ptr<SpatialOperation> SpatialOperation::BBoxIntersects =
    make_shared<SpatialOperationAnonymousInnerClass>();

SpatialOperation::SpatialOperationAnonymousInnerClass::
    SpatialOperationAnonymousInnerClass()
    : SpatialOperation(L"BBoxIntersects")
{
}

bool SpatialOperation::SpatialOperationAnonymousInnerClass::evaluate(
    shared_ptr<Shape> indexedShape, shared_ptr<Shape> queryShape)
{
  return indexedShape->getBoundingBox().relate(queryShape).intersects();
}

const shared_ptr<SpatialOperation> SpatialOperation::BBoxWithin =
    make_shared<SpatialOperationAnonymousInnerClass2>();

SpatialOperation::SpatialOperationAnonymousInnerClass2::
    SpatialOperationAnonymousInnerClass2()
    : SpatialOperation(L"BBoxWithin")
{
}

bool SpatialOperation::SpatialOperationAnonymousInnerClass2::evaluate(
    shared_ptr<Shape> indexedShape, shared_ptr<Shape> queryShape)
{
  shared_ptr<Rectangle> bbox = indexedShape->getBoundingBox();
  return bbox->relate(queryShape) == SpatialRelation::WITHIN ||
         bbox->equals(queryShape);
}

const shared_ptr<SpatialOperation> SpatialOperation::Contains =
    make_shared<SpatialOperationAnonymousInnerClass3>();

SpatialOperation::SpatialOperationAnonymousInnerClass3::
    SpatialOperationAnonymousInnerClass3()
    : SpatialOperation(L"Contains")
{
}

bool SpatialOperation::SpatialOperationAnonymousInnerClass3::evaluate(
    shared_ptr<Shape> indexedShape, shared_ptr<Shape> queryShape)
{
  return indexedShape->relate(queryShape) == SpatialRelation::CONTAINS ||
         indexedShape->equals(queryShape);
}

const shared_ptr<SpatialOperation> SpatialOperation::Intersects =
    make_shared<SpatialOperationAnonymousInnerClass4>();

SpatialOperation::SpatialOperationAnonymousInnerClass4::
    SpatialOperationAnonymousInnerClass4()
    : SpatialOperation(L"Intersects")
{
}

bool SpatialOperation::SpatialOperationAnonymousInnerClass4::evaluate(
    shared_ptr<Shape> indexedShape, shared_ptr<Shape> queryShape)
{
  return indexedShape->relate(queryShape).intersects();
}

const shared_ptr<SpatialOperation> SpatialOperation::IsEqualTo =
    make_shared<SpatialOperationAnonymousInnerClass5>();

SpatialOperation::SpatialOperationAnonymousInnerClass5::
    SpatialOperationAnonymousInnerClass5()
    : SpatialOperation(L"Equals")
{
}

bool SpatialOperation::SpatialOperationAnonymousInnerClass5::evaluate(
    shared_ptr<Shape> indexedShape, shared_ptr<Shape> queryShape)
{
  return indexedShape->equals(queryShape);
}

const shared_ptr<SpatialOperation> SpatialOperation::IsDisjointTo =
    make_shared<SpatialOperationAnonymousInnerClass6>();

SpatialOperation::SpatialOperationAnonymousInnerClass6::
    SpatialOperationAnonymousInnerClass6()
    : SpatialOperation(L"Disjoint")
{
}

bool SpatialOperation::SpatialOperationAnonymousInnerClass6::evaluate(
    shared_ptr<Shape> indexedShape, shared_ptr<Shape> queryShape)
{
  return !indexedShape->relate(queryShape).intersects();
}

const shared_ptr<SpatialOperation> SpatialOperation::IsWithin =
    make_shared<SpatialOperationAnonymousInnerClass7>();

SpatialOperation::SpatialOperationAnonymousInnerClass7::
    SpatialOperationAnonymousInnerClass7()
    : SpatialOperation(L"Within")
{
}

bool SpatialOperation::SpatialOperationAnonymousInnerClass7::evaluate(
    shared_ptr<Shape> indexedShape, shared_ptr<Shape> queryShape)
{
  return indexedShape->relate(queryShape) == SpatialRelation::WITHIN ||
         indexedShape->equals(queryShape);
}

const shared_ptr<SpatialOperation> SpatialOperation::Overlaps =
    make_shared<SpatialOperationAnonymousInnerClass8>();

SpatialOperation::SpatialOperationAnonymousInnerClass8::
    SpatialOperationAnonymousInnerClass8()
    : SpatialOperation(L"Overlaps")
{
}

bool SpatialOperation::SpatialOperationAnonymousInnerClass8::evaluate(
    shared_ptr<Shape> indexedShape, shared_ptr<Shape> queryShape)
{
  return indexedShape->relate(queryShape) ==
         SpatialRelation::INTERSECTS; // not Contains or Within or Disjoint
}

SpatialOperation::SpatialOperation(const wstring &name) : name(name)
{
  register_(name);
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  deque.push_back(shared_from_this());
}

void SpatialOperation::register_(const wstring &name)
{
  registry.emplace(name, shared_from_this());
  registry.emplace(name.toUpperCase(Locale::ROOT), shared_from_this());
}

shared_ptr<SpatialOperation> SpatialOperation::get(const wstring &v)
{
  shared_ptr<SpatialOperation> op = registry[v];
  if (op == nullptr) {
    op = registry[v.toUpperCase(Locale::ROOT)];
  }
  if (op == nullptr) {
    throw invalid_argument(L"Unknown Operation: " + v);
  }
  return op;
}

deque<std::shared_ptr<SpatialOperation>> SpatialOperation::values()
{
  return deque;
}

bool SpatialOperation::is(shared_ptr<SpatialOperation> op,
                          deque<SpatialOperation> &tst)
{
  for (shared_ptr<SpatialOperation> t : tst) {
    if (op == t) {
      return true;
    }
  }
  return false;
}

wstring SpatialOperation::getName() { return name; }

wstring SpatialOperation::toString() { return name; }
} // namespace org::apache::lucene::spatial::query