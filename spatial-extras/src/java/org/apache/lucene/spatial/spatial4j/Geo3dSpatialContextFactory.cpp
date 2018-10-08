using namespace std;

#include "Geo3dSpatialContextFactory.h"

namespace org::apache::lucene::spatial::spatial4j
{
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;
const shared_ptr<org::apache::lucene::spatial3d::geom::PlanetModel>
    Geo3dSpatialContextFactory::DEFAULT_PLANET_MODEL =
        org::apache::lucene::spatial3d::geom::PlanetModel::SPHERE;

Geo3dSpatialContextFactory::Geo3dSpatialContextFactory()
{
  this->binaryCodecClass = Geo3dBinaryCodec::typeid;
  this->shapeFactoryClass = Geo3dShapeFactory::typeid;
}

shared_ptr<SpatialContext> Geo3dSpatialContextFactory::newSpatialContext()
{
  if (planetModel == nullptr) {
    planetModel = DEFAULT_PLANET_MODEL;
  }
  if (distCalc == nullptr) {
    this->distCalc = make_shared<Geo3dDistanceCalculator>(planetModel);
  }
  return make_shared<SpatialContext>(shared_from_this());
}

void Geo3dSpatialContextFactory::init(unordered_map<wstring, wstring> &args,
                                      shared_ptr<ClassLoader> classLoader)
{
  initPlanetModel(args);
  SpatialContextFactory::init(args, classLoader);
}

void Geo3dSpatialContextFactory::initPlanetModel(
    unordered_map<wstring, wstring> &args)
{
  wstring planetModel = args[L"planetModel"];
  if (planetModel != L"") {
    // C++ TODO: The following Java case-insensitive std::wstring method call is not
    // converted:
    if (planetModel.equalsIgnoreCase(L"sphere")) {
      this->planetModel = PlanetModel::SPHERE;
    }
    // C++ TODO: The following Java case-insensitive std::wstring method call is not
    // converted:
    else if (planetModel.equalsIgnoreCase(L"wgs84")) {
      this->planetModel = PlanetModel::WGS84;
    } else {
      throw runtime_error(L"Unknown planet model: " + planetModel);
    }
  } else {
    this->planetModel = DEFAULT_PLANET_MODEL;
  }
}

void Geo3dSpatialContextFactory::initCalculator()
{
  wstring calcStr = this->args->get(L"distCalculator");
  if (calcStr == L"") {
    return;
  } else if (calcStr == L"geo3d") {
    this->distCalc = make_shared<Geo3dDistanceCalculator>(planetModel);
  } else {
    SpatialContextFactory::initCalculator(); // some other distance calculator
  }
}
} // namespace org::apache::lucene::spatial::spatial4j