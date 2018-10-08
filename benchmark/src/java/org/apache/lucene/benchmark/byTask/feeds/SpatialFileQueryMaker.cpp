using namespace std;

#include "SpatialFileQueryMaker.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../../queries/src/java/org/apache/lucene/queries/function/FunctionScoreQuery.h"
#include "../../../../../../../../../spatial-extras/src/java/org/apache/lucene/spatial/SpatialStrategy.h"
#include "../../../../../../../../../spatial-extras/src/java/org/apache/lucene/spatial/query/SpatialArgs.h"
#include "../../../../../../../../../spatial-extras/src/java/org/apache/lucene/spatial/query/SpatialOperation.h"
#include "../utils/Config.h"
#include "DocData.h"
#include "LineDocSource.h"
#include "NoMoreDataException.h"
#include "SpatialDocMaker.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using FunctionScoreQuery =
    org::apache::lucene::queries::function::FunctionScoreQuery;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Query = org::apache::lucene::search::Query;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::locationtech::spatial4j::shape::Shape;

void SpatialFileQueryMaker::setConfig(shared_ptr<Config> config) throw(
    runtime_error)
{
  strategy = SpatialDocMaker::getSpatialStrategy(config->getRoundNumber());
  shapeConverter =
      SpatialDocMaker::makeShapeConverter(strategy, config, L"query.spatial.");

  distErrPct = config->get(L"query.spatial.distErrPct", NAN);
  operation = SpatialOperation::get(
      config->get(L"query.spatial.predicate", L"Intersects"));
  score = config->get(L"query.spatial.score", false);

  AbstractQueryMaker::setConfig(config); // call last, will call
                                         // prepareQueries()
}

std::deque<std::shared_ptr<Query>>
SpatialFileQueryMaker::prepareQueries() 
{
  constexpr int maxQueries = config->get(L"query.file.maxQueries", 1000);
  shared_ptr<Config> srcConfig = make_shared<Config>(make_shared<Properties>());
  srcConfig->set(L"docs.file", config->get(L"query.file", L""));
  srcConfig->set(L"line.parser", config->get(L"query.file.line.parser", L""));
  srcConfig->set(L"content.source.forever", L"false");

  deque<std::shared_ptr<Query>> queries = deque<std::shared_ptr<Query>>();
  shared_ptr<LineDocSource> src = make_shared<LineDocSource>();
  try {
    src->setConfig(srcConfig);
    src->resetInputs();
    shared_ptr<DocData> docData = make_shared<DocData>();
    for (int i = 0; i < maxQueries; i++) {
      docData = src->getNextDocData(docData);
      shared_ptr<Shape> shape = SpatialDocMaker::makeShapeFromString(
          strategy, docData->getName(), docData->getBody());
      if (shape != nullptr) {
        shape = shapeConverter->convert(shape);
        queries.push_back(makeQueryFromShape(shape));
      } else {
        i--; // skip
      }
    }
  } catch (const NoMoreDataException &e) {
    // all-done
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete src;
  }
  return queries.toArray(std::deque<std::shared_ptr<Query>>(queries.size()));
}

shared_ptr<Query>
SpatialFileQueryMaker::makeQueryFromShape(shared_ptr<Shape> shape)
{
  shared_ptr<SpatialArgs> args = make_shared<SpatialArgs>(operation, shape);
  if (!isnan(distErrPct)) {
    args->setDistErrPct(distErrPct);
  }

  shared_ptr<Query> filterQuery = strategy->makeQuery(args);
  if (score) {
    // wrap with distance computing query
    shared_ptr<DoubleValuesSource> valueSource =
        strategy->makeDistanceValueSource(shape->getCenter());
    return make_shared<FunctionScoreQuery>(filterQuery, valueSource);
  } else {
    return filterQuery; // assume constant scoring
  }
}
} // namespace org::apache::lucene::benchmark::byTask::feeds