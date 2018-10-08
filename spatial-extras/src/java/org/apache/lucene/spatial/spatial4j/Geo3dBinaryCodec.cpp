using namespace std;

#include "Geo3dBinaryCodec.h"

namespace org::apache::lucene::spatial::spatial4j
{
using GeoAreaShape = org::apache::lucene::spatial3d::geom::GeoAreaShape;
using GeoBBox = org::apache::lucene::spatial3d::geom::GeoBBox;
using GeoCircle = org::apache::lucene::spatial3d::geom::GeoCircle;
using GeoPointShape = org::apache::lucene::spatial3d::geom::GeoPointShape;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using SerializableObject =
    org::apache::lucene::spatial3d::geom::SerializableObject;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;
using org::locationtech::spatial4j::io::BinaryCodec;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::ShapeCollection;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") public
// Geo3dBinaryCodec(org.locationtech.spatial4j.context.SpatialContext ctx,
// org.locationtech.spatial4j.context.SpatialContextFactory factory)
Geo3dBinaryCodec::Geo3dBinaryCodec(shared_ptr<SpatialContext> ctx,
                                   shared_ptr<SpatialContextFactory> factory)
    : org::locationtech::spatial4j::io::BinaryCodec(ctx, factory)
{
  planetModel = (std::static_pointer_cast<Geo3dSpatialContextFactory>(factory))
                    ->planetModel;
}

shared_ptr<Shape>
Geo3dBinaryCodec::readShape(shared_ptr<DataInput> dataInput) 
{
  shared_ptr<SerializableObject> serializableObject =
      SerializableObject::readObject(
          planetModel, std::static_pointer_cast<InputStream>(dataInput));
  if (std::dynamic_pointer_cast<GeoAreaShape>(serializableObject) != nullptr) {
    shared_ptr<GeoAreaShape> shape =
        std::static_pointer_cast<GeoAreaShape>(serializableObject);
    return make_shared<Geo3dShape<>>(shape, ctx);
  }
  throw invalid_argument(L"trying to read a not supported shape: " +
                         serializableObject->getClass());
}

void Geo3dBinaryCodec::writeShape(shared_ptr<DataOutput> dataOutput,
                                  shared_ptr<Shape> s) 
{
  if (std::dynamic_pointer_cast<Geo3dShape>(s) != nullptr) {
    shared_ptr<Geo3dShape> geoAreaShape =
        std::static_pointer_cast<Geo3dShape>(s);
    SerializableObject::writeObject(
        std::static_pointer_cast<OutputStream>(dataOutput),
        geoAreaShape->shape);
  } else {
    throw invalid_argument(L"trying to write a not supported shape: " +
                           s->getClassName());
  }
}

shared_ptr<Point>
Geo3dBinaryCodec::readPoint(shared_ptr<DataInput> dataInput) 
{
  shared_ptr<SerializableObject> serializableObject =
      SerializableObject::readObject(
          planetModel, std::static_pointer_cast<InputStream>(dataInput));
  if (std::dynamic_pointer_cast<GeoPointShape>(serializableObject) != nullptr) {
    shared_ptr<GeoPointShape> shape =
        std::static_pointer_cast<GeoPointShape>(serializableObject);
    return make_shared<Geo3dPointShape>(shape, ctx);
  }
  throw invalid_argument(L"trying to read a not supported point shape: " +
                         serializableObject->getClass());
}

void Geo3dBinaryCodec::writePoint(shared_ptr<DataOutput> dataOutput,
                                  shared_ptr<Point> pt) 
{
  writeShape(dataOutput, pt);
}

shared_ptr<Rectangle>
Geo3dBinaryCodec::readRect(shared_ptr<DataInput> dataInput) 
{
  shared_ptr<SerializableObject> serializableObject =
      SerializableObject::readObject(
          planetModel, std::static_pointer_cast<InputStream>(dataInput));
  if (std::dynamic_pointer_cast<GeoBBox>(serializableObject) != nullptr) {
    shared_ptr<GeoBBox> shape =
        std::static_pointer_cast<GeoBBox>(serializableObject);
    return make_shared<Geo3dRectangleShape>(shape, ctx);
  }
  throw invalid_argument(L"trying to read a not supported rectangle shape: " +
                         serializableObject->getClass());
}

void Geo3dBinaryCodec::writeRect(shared_ptr<DataOutput> dataOutput,
                                 shared_ptr<Rectangle> r) 
{
  writeShape(dataOutput, r);
}

shared_ptr<Circle>
Geo3dBinaryCodec::readCircle(shared_ptr<DataInput> dataInput) 
{
  shared_ptr<SerializableObject> serializableObject =
      SerializableObject::readObject(
          planetModel, std::static_pointer_cast<InputStream>(dataInput));
  if (std::dynamic_pointer_cast<GeoCircle>(serializableObject) != nullptr) {
    shared_ptr<GeoCircle> shape =
        std::static_pointer_cast<GeoCircle>(serializableObject);
    return make_shared<Geo3dCircleShape>(shape, ctx);
  }
  throw invalid_argument(L"trying to read a not supported circle shape: " +
                         serializableObject->getClass());
}

void Geo3dBinaryCodec::writeCircle(shared_ptr<DataOutput> dataOutput,
                                   shared_ptr<Circle> c) 
{
  writeShape(dataOutput, c);
}

shared_ptr<ShapeCollection> Geo3dBinaryCodec::readCollection(
    shared_ptr<DataInput> dataInput) 
{
  throw make_shared<UnsupportedOperationException>();
}

void Geo3dBinaryCodec::writeCollection(
    shared_ptr<DataOutput> dataOutput,
    shared_ptr<ShapeCollection> col) 
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::spatial::spatial4j