using namespace std;

#include "SpatialDocMaker.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../spatial-extras/src/java/org/apache/lucene/spatial/SpatialStrategy.h"
#include "../../../../../../../../../spatial-extras/src/java/org/apache/lucene/spatial/composite/CompositeSpatialStrategy.h"
#include "../../../../../../../../../spatial-extras/src/java/org/apache/lucene/spatial/prefix/RecursivePrefixTreeStrategy.h"
#include "../../../../../../../../../spatial-extras/src/java/org/apache/lucene/spatial/prefix/tree/PackedQuadPrefixTree.h"
#include "../../../../../../../../../spatial-extras/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"
#include "../../../../../../../../../spatial-extras/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTreeFactory.h"
#include "../../../../../../../../../spatial-extras/src/java/org/apache/lucene/spatial/serialized/SerializedDVStrategy.h"
#include "../utils/Config.h"
#include "ContentSource.h"
#include "DocData.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using CompositeSpatialStrategy =
    org::apache::lucene::spatial::composite::CompositeSpatialStrategy;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using PackedQuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::PackedQuadPrefixTree;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialPrefixTreeFactory =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTreeFactory;
using SerializedDVStrategy =
    org::apache::lucene::spatial::serialized::SerializedDVStrategy;
const wstring SpatialDocMaker::SPATIAL_FIELD = L"spatial";
unordered_map<int,
              std::shared_ptr<org::apache::lucene::spatial::SpatialStrategy>>
    SpatialDocMaker::spatialStrategyCache = unordered_map<
        int, std::shared_ptr<org::apache::lucene::spatial::SpatialStrategy>>();

shared_ptr<SpatialStrategy> SpatialDocMaker::getSpatialStrategy(int roundNumber)
{
  shared_ptr<SpatialStrategy> result = spatialStrategyCache[roundNumber];
  if (result == nullptr) {
    throw make_shared<IllegalStateException>(
        L"Strategy should have been init'ed by SpatialDocMaker by now");
  }
  return result;
}

shared_ptr<SpatialStrategy>
SpatialDocMaker::makeSpatialStrategy(shared_ptr<Config> config)
{
  // A Map view of Config that prefixes keys with "spatial."
  unordered_map<wstring, wstring> configMap =
      make_shared<AbstractMapAnonymousInnerClass>(shared_from_this(), config);

  shared_ptr<SpatialContext> ctx =
      SpatialContextFactory::makeSpatialContext(configMap, nullptr);

  return makeSpatialStrategy(config, configMap, ctx);
}

SpatialDocMaker::AbstractMapAnonymousInnerClass::AbstractMapAnonymousInnerClass(
    shared_ptr<SpatialDocMaker> outerInstance, shared_ptr<Config> config)
{
  this->outerInstance = outerInstance;
  this->config = config;
}

shared_ptr<Set<Entry<wstring, wstring>>>
SpatialDocMaker::AbstractMapAnonymousInnerClass::entrySet()
{
  throw make_shared<UnsupportedOperationException>();
}

wstring SpatialDocMaker::AbstractMapAnonymousInnerClass::get(any key)
{
  return config->get(L"spatial." + key, L"");
}

shared_ptr<SpatialStrategy>
SpatialDocMaker::makeSpatialStrategy(shared_ptr<Config> config,
                                     unordered_map<wstring, wstring> &configMap,
                                     shared_ptr<SpatialContext> ctx)
{
  // TODO once strategies have factories, we could use them here.
  const wstring strategyName = config->get(L"spatial.strategy", L"rpt");
  switch (strategyName) {
  case L"rpt":
    return makeRPTStrategy(SPATIAL_FIELD, config, configMap, ctx);
  case L"composite":
    return makeCompositeStrategy(config, configMap, ctx);
  // TODO add more as-needed
  default:
    throw make_shared<IllegalStateException>(L"Unknown spatial.strategy: " +
                                             strategyName);
  }
}

shared_ptr<RecursivePrefixTreeStrategy> SpatialDocMaker::makeRPTStrategy(
    const wstring &spatialField, shared_ptr<Config> config,
    unordered_map<wstring, wstring> &configMap, shared_ptr<SpatialContext> ctx)
{
  // A factory for the prefix tree grid
  shared_ptr<SpatialPrefixTree> grid =
      SpatialPrefixTreeFactory::makeSPT(configMap, nullptr, ctx);

  shared_ptr<RecursivePrefixTreeStrategy> strategy =
      make_shared<RecursivePrefixTreeStrategy>(grid, spatialField);
  strategy->setPointsOnly(config->get(L"spatial.docPointsOnly", false));
  constexpr bool pruneLeafyBranches =
      config->get(L"spatial.pruneLeafyBranches", true);
  if (std::dynamic_pointer_cast<PackedQuadPrefixTree>(grid) != nullptr) {
    (std::static_pointer_cast<PackedQuadPrefixTree>(grid))
        ->setPruneLeafyBranches(pruneLeafyBranches);
    strategy->setPruneLeafyBranches(
        false); // always leave it to packed grid, even though it isn't the same
  } else {
    strategy->setPruneLeafyBranches(pruneLeafyBranches);
  }

  int prefixGridScanLevel =
      config->get(L"query.spatial.prefixGridScanLevel", -4);
  if (prefixGridScanLevel < 0) {
    prefixGridScanLevel = grid->getMaxLevels() + prefixGridScanLevel;
  }
  strategy->setPrefixGridScanLevel(prefixGridScanLevel);

  double distErrPct =
      config->get(L"spatial.distErrPct", .025); // doc & query; a default
  strategy->setDistErrPct(distErrPct);
  return strategy;
}

shared_ptr<SerializedDVStrategy> SpatialDocMaker::makeSerializedDVStrategy(
    const wstring &spatialField, shared_ptr<Config> config,
    unordered_map<wstring, wstring> &configMap, shared_ptr<SpatialContext> ctx)
{
  return make_shared<SerializedDVStrategy>(ctx, spatialField);
}

shared_ptr<SpatialStrategy> SpatialDocMaker::makeCompositeStrategy(
    shared_ptr<Config> config, unordered_map<wstring, wstring> &configMap,
    shared_ptr<SpatialContext> ctx)
{
  shared_ptr<CompositeSpatialStrategy> *const strategy =
      make_shared<CompositeSpatialStrategy>(
          SPATIAL_FIELD,
          makeRPTStrategy(SPATIAL_FIELD + L"_rpt", config, configMap, ctx),
          makeSerializedDVStrategy(SPATIAL_FIELD + L"_sdv", config, configMap,
                                   ctx));
  strategy->setOptimizePredicates(
      config->get(L"query.spatial.composite.optimizePredicates", true));
  return strategy;
}

void SpatialDocMaker::setConfig(shared_ptr<Config> config,
                                shared_ptr<ContentSource> source)
{
  DocMaker::setConfig(config, source);
  shared_ptr<SpatialStrategy> existing =
      spatialStrategyCache[config->getRoundNumber()];
  if (existing == nullptr) {
    // new round; we need to re-initialize
    strategy = makeSpatialStrategy(config);
    spatialStrategyCache.emplace(config->getRoundNumber(), strategy);
    // TODO remove previous round config?
    shapeConverter = makeShapeConverter(strategy, config, L"doc.spatial.");
    wcout << L"Spatial Strategy: " << strategy << endl;
  }
}

shared_ptr<ShapeConverter>
SpatialDocMaker::makeShapeConverter(shared_ptr<SpatialStrategy> spatialStrategy,
                                    shared_ptr<Config> config,
                                    const wstring &configKeyPrefix)
{
  // by default does no conversion
  constexpr double radiusDegrees =
      config->get(configKeyPrefix + L"radiusDegrees", 0.0);
  constexpr double plusMinus =
      config->get(configKeyPrefix + L"radiusDegreesRandPlusMinus", 0.0);
  constexpr bool bbox = config->get(configKeyPrefix + L"bbox", false);

  return make_shared<ShapeConverterAnonymousInnerClass>(
      spatialStrategy, radiusDegrees, plusMinus, bbox);
}

SpatialDocMaker::ShapeConverterAnonymousInnerClass::
    ShapeConverterAnonymousInnerClass(
        shared_ptr<SpatialStrategy> spatialStrategy, double radiusDegrees,
        double plusMinus, bool bbox)
{
  this->spatialStrategy = spatialStrategy;
  this->radiusDegrees = radiusDegrees;
  this->plusMinus = plusMinus;
  this->bbox = bbox;
}

shared_ptr<Shape> SpatialDocMaker::ShapeConverterAnonymousInnerClass::convert(
    shared_ptr<Shape> shape)
{
  if (std::dynamic_pointer_cast<Point>(shape) != nullptr &&
      (radiusDegrees != 0.0 || plusMinus != 0.0)) {
    shared_ptr<Point> point = std::static_pointer_cast<Point>(shape);
    double radius = radiusDegrees;
    if (plusMinus > 0.0) {
      shared_ptr<Random> random = make_shared<Random>(
          point->hashCode()); // use hashCode so it's reproducibly random
      radius += random->nextDouble() * 2 * plusMinus - plusMinus;
      radius = abs(radius); // can happen if configured plusMinus >
                            // radiusDegrees
    }
    shape = spatialStrategy->getSpatialContext()->makeCircle(point, radius);
  }
  if (bbox) {
    shape = shape->getBoundingBox();
  }
  return shape;
}

shared_ptr<Document> SpatialDocMaker::makeDocument() 
{

  shared_ptr<DocState> docState = getDocState();

  shared_ptr<Document> doc = DocMaker::makeDocument();

  // Set SPATIAL_FIELD from body
  shared_ptr<DocData> docData = docState->docData;
  //   makeDocument() resets docState.getBody() so we can't look there; look in
  //   Document
  wstring shapeStr = doc->getField(DocMaker::BODY_FIELD)->stringValue();
  shared_ptr<Shape> shape =
      makeShapeFromString(strategy, docData->getName(), shapeStr);
  if (shape != nullptr) {
    shape = shapeConverter->convert(shape);
    // index
    for (auto f : strategy->createIndexableFields(shape)) {
      doc->push_back(f);
    }
  }

  return doc;
}

shared_ptr<Shape>
SpatialDocMaker::makeShapeFromString(shared_ptr<SpatialStrategy> strategy,
                                     const wstring &name,
                                     const wstring &shapeStr)
{
  if (shapeStr != L"" && shapeStr.length() > 0) {
    try {
      return strategy->getSpatialContext()->readShapeFromWkt(shapeStr);
    } catch (const runtime_error &e) { // InvalidShapeException TODO
      System::err::println(L"Shape " + name + L" wasn't parseable: " + e +
                           L"  (skipping it)");
      return nullptr;
    }
  }
  return nullptr;
}

shared_ptr<Document>
SpatialDocMaker::makeDocument(int size) 
{
  // TODO consider abusing the 'size' notion to number of shapes per document
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::benchmark::byTask::feeds