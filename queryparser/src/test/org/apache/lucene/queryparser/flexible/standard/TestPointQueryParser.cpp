using namespace std;

#include "TestPointQueryParser.h"

namespace org::apache::lucene::queryparser::flexible::standard
{
using DoublePoint = org::apache::lucene::document::DoublePoint;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using PointsConfig =
    org::apache::lucene::queryparser::flexible::standard::config::PointsConfig;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestPointQueryParser::testIntegers() 
{
  shared_ptr<StandardQueryParser> parser = make_shared<StandardQueryParser>();
  unordered_map<wstring, std::shared_ptr<PointsConfig>> pointsConfig =
      unordered_map<wstring, std::shared_ptr<PointsConfig>>();
  pointsConfig.emplace(
      L"intField",
      make_shared<PointsConfig>(NumberFormat::getIntegerInstance(Locale::ROOT),
                                Integer::typeid));
  parser->setPointsConfigMap(pointsConfig);

  assertEquals(IntPoint::newRangeQuery(L"intField", 1, 3),
               parser->parse(L"intField:[1 TO 3]", L"body"));
  assertEquals(IntPoint::newRangeQuery(L"intField", 1, 1),
               parser->parse(L"intField:1", L"body"));
}

void TestPointQueryParser::testLongs() 
{
  shared_ptr<StandardQueryParser> parser = make_shared<StandardQueryParser>();
  unordered_map<wstring, std::shared_ptr<PointsConfig>> pointsConfig =
      unordered_map<wstring, std::shared_ptr<PointsConfig>>();
  pointsConfig.emplace(
      L"longField",
      make_shared<PointsConfig>(NumberFormat::getIntegerInstance(Locale::ROOT),
                                Long::typeid));
  parser->setPointsConfigMap(pointsConfig);

  assertEquals(LongPoint::newRangeQuery(L"longField", 1, 3),
               parser->parse(L"longField:[1 TO 3]", L"body"));
  assertEquals(LongPoint::newRangeQuery(L"longField", 1, 1),
               parser->parse(L"longField:1", L"body"));
}

void TestPointQueryParser::testFloats() 
{
  shared_ptr<StandardQueryParser> parser = make_shared<StandardQueryParser>();
  unordered_map<wstring, std::shared_ptr<PointsConfig>> pointsConfig =
      unordered_map<wstring, std::shared_ptr<PointsConfig>>();
  pointsConfig.emplace(
      L"floatField",
      make_shared<PointsConfig>(NumberFormat::getNumberInstance(Locale::ROOT),
                                Float::typeid));
  parser->setPointsConfigMap(pointsConfig);

  assertEquals(FloatPoint::newRangeQuery(L"floatField", 1.5F, 3.6F),
               parser->parse(L"floatField:[1.5 TO 3.6]", L"body"));
  assertEquals(FloatPoint::newRangeQuery(L"floatField", 1.5F, 1.5F),
               parser->parse(L"floatField:1.5", L"body"));
}

void TestPointQueryParser::testDoubles() 
{
  shared_ptr<StandardQueryParser> parser = make_shared<StandardQueryParser>();
  unordered_map<wstring, std::shared_ptr<PointsConfig>> pointsConfig =
      unordered_map<wstring, std::shared_ptr<PointsConfig>>();
  pointsConfig.emplace(
      L"doubleField",
      make_shared<PointsConfig>(NumberFormat::getNumberInstance(Locale::ROOT),
                                Double::typeid));
  parser->setPointsConfigMap(pointsConfig);

  assertEquals(DoublePoint::newRangeQuery(L"doubleField", 1.5, 3.6),
               parser->parse(L"doubleField:[1.5 TO 3.6]", L"body"));
  assertEquals(DoublePoint::newRangeQuery(L"doubleField", 1.5, 1.5),
               parser->parse(L"doubleField:1.5", L"body"));
}
} // namespace org::apache::lucene::queryparser::flexible::standard