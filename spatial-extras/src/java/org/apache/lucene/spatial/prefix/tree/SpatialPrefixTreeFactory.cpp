using namespace std;

#include "SpatialPrefixTreeFactory.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
const wstring SpatialPrefixTreeFactory::PREFIX_TREE = L"prefixTree";
const wstring SpatialPrefixTreeFactory::MAX_LEVELS = L"maxLevels";
const wstring SpatialPrefixTreeFactory::MAX_DIST_ERR = L"maxDistErr";

shared_ptr<SpatialPrefixTree>
SpatialPrefixTreeFactory::makeSPT(unordered_map<wstring, wstring> &args,
                                  shared_ptr<ClassLoader> classLoader,
                                  shared_ptr<SpatialContext> ctx)
{
  // TODO refactor to use Java SPI like how Lucene already does for
  // codecs/postingsFormats, etc
  shared_ptr<SpatialPrefixTreeFactory> instance;
  wstring cname = args[PREFIX_TREE];
  if (cname == L"") {
    cname = ctx->isGeo() ? L"geohash" : L"quad";
  }
  // C++ TODO: The following Java case-insensitive std::wstring method call is not
  // converted:
  if ((wstring(L"geohash")).equalsIgnoreCase(cname)) {
    instance = make_shared<GeohashPrefixTree::Factory>();
  }
  // C++ TODO: The following Java case-insensitive std::wstring method call is not
  // converted:
  else if ((wstring(L"quad")).equalsIgnoreCase(cname)) {
    instance = make_shared<QuadPrefixTree::Factory>();
  }
  // C++ TODO: The following Java case-insensitive std::wstring method call is not
  // converted:
  else if ((wstring(L"packedQuad")).equalsIgnoreCase(cname)) {
    instance = make_shared<PackedQuadPrefixTree::Factory>();
  }
  // C++ TODO: The following Java case-insensitive std::wstring method call is not
  // converted:
  else if ((wstring(L"s2")).equalsIgnoreCase(cname)) {
    instance = make_shared<S2PrefixTree::Factory>();
  } else {
    try {
      type_info c = classLoader->loadClass(cname);
      instance =
          std::static_pointer_cast<SpatialPrefixTreeFactory>(c.newInstance());
    } catch (const runtime_error &e) {
      throw runtime_error(e);
    }
  }
  instance->init(args, ctx);
  return instance->newSPT();
}

void SpatialPrefixTreeFactory::init(unordered_map<wstring, wstring> &args,
                                    shared_ptr<SpatialContext> ctx)
{
  this->args = args;
  this->ctx = ctx;
  initMaxLevels();
}

void SpatialPrefixTreeFactory::initMaxLevels()
{
  wstring mlStr = args[MAX_LEVELS];
  if (mlStr != L"") {
    maxLevels = stoi(mlStr);
    return;
  }

  double degrees;
  wstring maxDetailDistStr = args[MAX_DIST_ERR];
  if (maxDetailDistStr == L"") {
    if (!ctx->isGeo()) {
      return; // let default to max
    }
    degrees = DistanceUtils::dist2Degrees(DEFAULT_GEO_MAX_DETAIL_KM,
                                          DistanceUtils::EARTH_MEAN_RADIUS_KM);
  } else {
    degrees = stod(maxDetailDistStr);
  }
  maxLevels = getLevelForDistance(degrees);
}
} // namespace org::apache::lucene::spatial::prefix::tree