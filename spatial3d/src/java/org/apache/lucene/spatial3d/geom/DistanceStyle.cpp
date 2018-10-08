using namespace std;

#include "DistanceStyle.h"

namespace org::apache::lucene::spatial3d::geom
{

const shared_ptr<ArcDistance> DistanceStyle::ARC = ArcDistance::INSTANCE;
const shared_ptr<LinearDistance> DistanceStyle::LINEAR =
    LinearDistance::INSTANCE;
const shared_ptr<LinearSquaredDistance> DistanceStyle::LINEAR_SQUARED =
    LinearSquaredDistance::INSTANCE;
const shared_ptr<NormalDistance> DistanceStyle::NORMAL =
    NormalDistance::INSTANCE;
const shared_ptr<NormalSquaredDistance> DistanceStyle::NORMAL_SQUARED =
    NormalSquaredDistance::INSTANCE;
} // namespace org::apache::lucene::spatial3d::geom