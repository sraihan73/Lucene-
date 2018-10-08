using namespace std;

#include "BasePlanetObject.h"

namespace org::apache::lucene::spatial3d::geom
{

BasePlanetObject::BasePlanetObject(shared_ptr<PlanetModel> planetModel)
    : planetModel(planetModel)
{
}

shared_ptr<PlanetModel> BasePlanetObject::getPlanetModel()
{
  return planetModel;
}

void BasePlanetObject::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int BasePlanetObject::hashCode() { return planetModel->hashCode(); }

bool BasePlanetObject::equals(any const o)
{
  if (!(std::dynamic_pointer_cast<BasePlanetObject>(o) != nullptr)) {
    return false;
  }
  return planetModel->equals(
      (any_cast<std::shared_ptr<BasePlanetObject>>(o)).planetModel);
}
} // namespace org::apache::lucene::spatial3d::geom