using namespace std;

#include "RandomBinaryCodecTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 10) public void
// testRandomPointCodec() throws java.io.IOException
void RandomBinaryCodecTest::testRandomPointCodec() 
{
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  shared_ptr<GeoPoint> shape = randomGeoPoint(planetModel);
  shared_ptr<ByteArrayOutputStream> outputStream =
      make_shared<ByteArrayOutputStream>();
  SerializableObject::writeObject(outputStream, shape);
  shared_ptr<ByteArrayInputStream> inputStream =
      make_shared<ByteArrayInputStream>(outputStream->toByteArray());
  shared_ptr<SerializableObject> shapeCopy =
      SerializableObject::readObject(planetModel, inputStream);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(shape->toString(), shape, shapeCopy);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 100) public void
// testRandomPlanetObjectCodec() throws java.io.IOException
void RandomBinaryCodecTest::testRandomPlanetObjectCodec() 
{
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  int type = randomShapeType();
  shared_ptr<GeoShape> shape = randomGeoShape(type, planetModel);
  shared_ptr<ByteArrayOutputStream> outputStream =
      make_shared<ByteArrayOutputStream>();
  SerializableObject::writePlanetObject(outputStream, shape);
  shared_ptr<ByteArrayInputStream> inputStream =
      make_shared<ByteArrayInputStream>(outputStream->toByteArray());
  shared_ptr<SerializableObject> shapeCopy =
      SerializableObject::readPlanetObject(inputStream);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(shape->toString(), shape, shapeCopy);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 100) public void
// testRandomShapeCodec() throws java.io.IOException
void RandomBinaryCodecTest::testRandomShapeCodec() 
{
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  int type = randomShapeType();
  shared_ptr<GeoShape> shape = randomGeoShape(type, planetModel);
  shared_ptr<ByteArrayOutputStream> outputStream =
      make_shared<ByteArrayOutputStream>();
  SerializableObject::writeObject(outputStream, shape);
  shared_ptr<ByteArrayInputStream> inputStream =
      make_shared<ByteArrayInputStream>(outputStream->toByteArray());
  shared_ptr<SerializableObject> shapeCopy =
      SerializableObject::readObject(planetModel, inputStream);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(shape->toString(), shape, shapeCopy);
}
} // namespace org::apache::lucene::spatial3d::geom