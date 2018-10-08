using namespace std;

#include "DistanceFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using WhitespaceAnalyzer =
    org::apache::lucene::analysis::core::WhitespaceAnalyzer;
using Document = org::apache::lucene::document::Document;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using Expression = org::apache::lucene::expressions::Expression;
using SimpleBindings = org::apache::lucene::expressions::SimpleBindings;
using JavascriptCompiler =
    org::apache::lucene::expressions::js::JavascriptCompiler;
using DrillDownQuery = org::apache::lucene::facet::DrillDownQuery;
using DrillSideways = org::apache::lucene::facet::DrillSideways;
using FacetResult = org::apache::lucene::facet::FacetResult;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using DoubleRange = org::apache::lucene::facet::range::DoubleRange;
using DoubleRangeFacetCounts =
    org::apache::lucene::facet::range::DoubleRangeFacetCounts;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using SortField = org::apache::lucene::search::SortField;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using SloppyMath = org::apache::lucene::util::SloppyMath;

DistanceFacetsExample::DistanceFacetsExample() {}

void DistanceFacetsExample::index() 
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      indexDir,
      (make_shared<IndexWriterConfig>(make_shared<WhitespaceAnalyzer>()))
          ->setOpenMode(IndexWriterConfig::OpenMode::CREATE));

  // TODO: we could index in radians instead ... saves all the conversions in
  // getBoundingBoxFilter

  // Add documents with latitude/longitude location:
  // we index these both as DoublePoints (for bounding box/ranges) and as
  // NumericDocValuesFields (for scoring)
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<DoublePoint>(L"latitude", 40.759011));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"latitude", Double::doubleToRawLongBits(40.759011)));
  doc->push_back(make_shared<DoublePoint>(L"longitude", -73.9844722));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"longitude", Double::doubleToRawLongBits(-73.9844722)));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoublePoint>(L"latitude", 40.718266));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"latitude", Double::doubleToRawLongBits(40.718266)));
  doc->push_back(make_shared<DoublePoint>(L"longitude", -74.007819));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"longitude", Double::doubleToRawLongBits(-74.007819)));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoublePoint>(L"latitude", 40.7051157));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"latitude", Double::doubleToRawLongBits(40.7051157)));
  doc->push_back(make_shared<DoublePoint>(L"longitude", -74.0088305));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"longitude", Double::doubleToRawLongBits(-74.0088305)));
  writer->addDocument(doc);

  // Open near-real-time searcher
  searcher = make_shared<IndexSearcher>(DirectoryReader::open(writer));
  delete writer;
}

shared_ptr<DoubleValuesSource> DistanceFacetsExample::getDistanceValueSource()
{
  shared_ptr<Expression> distance;
  try {
    distance = JavascriptCompiler::compile(
        L"haversin(" + to_wstring(ORIGIN_LATITUDE) + L"," +
        to_wstring(ORIGIN_LONGITUDE) + L",latitude,longitude)");
  } catch (const ParseException &pe) {
    // Should not happen
    throw runtime_error(pe);
  }
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"latitude", SortField::Type::DOUBLE));
  bindings->add(make_shared<SortField>(L"longitude", SortField::Type::DOUBLE));

  return distance->getDoubleValuesSource(bindings);
}

shared_ptr<Query>
DistanceFacetsExample::getBoundingBoxQuery(double originLat, double originLng,
                                           double maxDistanceKM)
{

  // Basic bounding box geo math from
  // http://JanMatuschek.de/LatitudeLongitudeBoundingCoordinates,
  // licensed under creative commons 3.0:
  // http://creativecommons.org/licenses/by/3.0

  // TODO: maybe switch to recursive prefix tree instead
  // (in lucene/spatial)?  It should be more efficient
  // since it's a 2D trie...

  // Degrees -> Radians:
  double originLatRadians = SloppyMath::toRadians(originLat);
  double originLngRadians = SloppyMath::toRadians(originLng);

  double angle = maxDistanceKM / EARTH_RADIUS_KM;

  double minLat = originLatRadians - angle;
  double maxLat = originLatRadians + angle;

  double minLng;
  double maxLng;
  if (minLat > SloppyMath::toRadians(-90) &&
      maxLat < SloppyMath::toRadians(90)) {
    double delta = asin(sin(angle) / cos(originLatRadians));
    minLng = originLngRadians - delta;
    if (minLng < SloppyMath::toRadians(-180)) {
      minLng += 2 * M_PI;
    }
    maxLng = originLngRadians + delta;
    if (maxLng > SloppyMath::toRadians(180)) {
      maxLng -= 2 * M_PI;
    }
  } else {
    // The query includes a pole!
    minLat = max(minLat, SloppyMath::toRadians(-90));
    maxLat = min(maxLat, SloppyMath::toRadians(90));
    minLng = SloppyMath::toRadians(-180);
    maxLng = SloppyMath::toRadians(180);
  }

  shared_ptr<BooleanQuery::Builder> f = make_shared<BooleanQuery::Builder>();

  // Add latitude range filter:
  f->add(DoublePoint::newRangeQuery(L"latitude", SloppyMath::toDegrees(minLat),
                                    SloppyMath::toDegrees(maxLat)),
         BooleanClause::Occur::FILTER);

  // Add longitude range filter:
  if (minLng > maxLng) {
    // The bounding box crosses the international date
    // line:
    shared_ptr<BooleanQuery::Builder> lonF =
        make_shared<BooleanQuery::Builder>();
    lonF->add(DoublePoint::newRangeQuery(L"longitude",
                                         SloppyMath::toDegrees(minLng),
                                         numeric_limits<double>::infinity()),
              BooleanClause::Occur::SHOULD);
    lonF->add(DoublePoint::newRangeQuery(L"longitude",
                                         -numeric_limits<double>::infinity(),
                                         SloppyMath::toDegrees(maxLng)),
              BooleanClause::Occur::SHOULD);
    f->add(lonF->build(), BooleanClause::Occur::MUST);
  } else {
    f->add(DoublePoint::newRangeQuery(L"longitude",
                                      SloppyMath::toDegrees(minLng),
                                      SloppyMath::toDegrees(maxLng)),
           BooleanClause::Occur::FILTER);
  }

  return f->build();
}

shared_ptr<FacetResult> DistanceFacetsExample::search() 
{

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  searcher->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<Facets> facets = make_shared<DoubleRangeFacetCounts>(
      L"field", getDistanceValueSource(), fc,
      getBoundingBoxQuery(ORIGIN_LATITUDE, ORIGIN_LONGITUDE, 10.0), ONE_KM,
      TWO_KM, FIVE_KM, TEN_KM);

  return facets->getTopChildren(10, L"field");
}

shared_ptr<TopDocs> DistanceFacetsExample::drillDown(
    shared_ptr<DoubleRange> range) 
{

  // Passing no baseQuery means we drill down on all
  // documents ("browse only"):
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(nullptr);
  shared_ptr<DoubleValuesSource> *const vs = getDistanceValueSource();
  q->add(L"field",
         range->getQuery(
             getBoundingBoxQuery(ORIGIN_LATITUDE, ORIGIN_LONGITUDE, range->max),
             vs));
  shared_ptr<DrillSideways> ds = make_shared<DrillSidewaysAnonymousInnerClass>(
      shared_from_this(), searcher, config,
      std::static_pointer_cast<TaxonomyReader>(nullptr), vs);
  return ds->search(q, 10)->hits;
}

    DistanceFacetsExample::DrillSidewaysAnonymousInnerClass::DrillSidewaysAnonymousInnerClass(shared_ptr<DistanceFacetsExample> outerInstance, shared_ptr<IndexSearcher> searcher, shared_ptr<FacetsConfig> config, shared_ptr<TaxonomyReader> org) : org->apache.lucene.facet.DrillSideways(searcher, config, TaxonomyReader) nullptr)
    {
      this->outerInstance = outerInstance;
      this->vs = vs;
    }

    shared_ptr<Facets>
    DistanceFacetsExample::DrillSidewaysAnonymousInnerClass::buildFacetsResult(
        shared_ptr<FacetsCollector> drillDowns,
        std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
        std::deque<wstring> &drillSidewaysDims) 
    {
      assert(drillSideways.size() == 1);
      return make_shared<DoubleRangeFacetCounts>(
          L"field", vs, drillSideways[0], outerInstance->ONE_KM,
          outerInstance->TWO_KM, outerInstance->FIVE_KM, outerInstance->TEN_KM);
    }

    DistanceFacetsExample::~DistanceFacetsExample()
    {
      delete searcher->getIndexReader();
      delete indexDir;
    }

    void
    DistanceFacetsExample::main(std::deque<wstring> &args) 
    {
      shared_ptr<DistanceFacetsExample> example =
          make_shared<DistanceFacetsExample>();
      example->index();

      wcout << L"Distance facet counting example:" << endl;
      wcout << L"-----------------------" << endl;
      wcout << example->search() << endl;

      wcout << L"Distance facet drill-down example (field/< 2 km):" << endl;
      wcout << L"---------------------------------------------" << endl;
      shared_ptr<TopDocs> hits = example->drillDown(example->TWO_KM);
      wcout << hits->totalHits << L" totalHits" << endl;

      delete example;
    }
    } // namespace org::apache::lucene::demo::facet