using namespace std;

#include "CoreParser.h"

namespace org::apache::lucene::queryparser::xml
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using namespace org::apache::lucene::queryparser::xml::builders;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using org::w3c::dom::Document;
using org::w3c::dom::Element;
using org::xml::sax::EntityResolver;
using org::xml::sax::ErrorHandler;
using org::xml::sax::SAXException;

CoreParser::CoreParser(shared_ptr<Analyzer> analyzer,
                       shared_ptr<QueryParser> parser)
    : CoreParser(nullptr, analyzer, parser)
{
}

CoreParser::CoreParser(const wstring &defaultField,
                       shared_ptr<Analyzer> analyzer)
    : CoreParser(defaultField, analyzer, nullptr)
{
}

CoreParser::CoreParser(const wstring &defaultField,
                       shared_ptr<Analyzer> analyzer,
                       shared_ptr<QueryParser> parser)
    : spanFactory(make_shared<SpanQueryBuilderFactory>())
{
  this->defaultField = defaultField;
  this->analyzer = analyzer;
  this->parser = parser;

  queryFactory = make_shared<QueryBuilderFactory>();

  queryFactory->addBuilder(L"TermQuery", make_shared<TermQueryBuilder>());
  queryFactory->addBuilder(L"TermsQuery",
                           make_shared<TermsQueryBuilder>(analyzer));
  queryFactory->addBuilder(L"MatchAllDocsQuery",
                           make_shared<MatchAllDocsQueryBuilder>());
  queryFactory->addBuilder(L"BooleanQuery",
                           make_shared<BooleanQueryBuilder>(queryFactory));
  queryFactory->addBuilder(L"PointRangeQuery",
                           make_shared<PointRangeQueryBuilder>());
  queryFactory->addBuilder(L"RangeQuery", make_shared<RangeQueryBuilder>());
  queryFactory->addBuilder(
      L"DisjunctionMaxQuery",
      make_shared<DisjunctionMaxQueryBuilder>(queryFactory));
  if (parser != nullptr) {
    queryFactory->addBuilder(L"UserQuery",
                             make_shared<UserInputQueryBuilder>(parser));
  } else {
    queryFactory->addBuilder(L"UserQuery", make_shared<UserInputQueryBuilder>(
                                               defaultField, analyzer));
  }
  queryFactory->addBuilder(
      L"ConstantScoreQuery",
      make_shared<ConstantScoreQueryBuilder>(queryFactory));

  shared_ptr<SpanNearBuilder> snb = make_shared<SpanNearBuilder>(spanFactory);
  spanFactory->addBuilder(L"SpanNear", snb);
  queryFactory->addBuilder(L"SpanNear", snb);

  shared_ptr<BoostingTermBuilder> btb = make_shared<BoostingTermBuilder>();
  spanFactory->addBuilder(L"BoostingTermQuery", btb);
  queryFactory->addBuilder(L"BoostingTermQuery", btb);

  shared_ptr<SpanTermBuilder> snt = make_shared<SpanTermBuilder>();
  spanFactory->addBuilder(L"SpanTerm", snt);
  queryFactory->addBuilder(L"SpanTerm", snt);

  shared_ptr<SpanOrBuilder> sot = make_shared<SpanOrBuilder>(spanFactory);
  spanFactory->addBuilder(L"SpanOr", sot);
  queryFactory->addBuilder(L"SpanOr", sot);

  shared_ptr<SpanOrTermsBuilder> sots =
      make_shared<SpanOrTermsBuilder>(analyzer);
  spanFactory->addBuilder(L"SpanOrTerms", sots);
  queryFactory->addBuilder(L"SpanOrTerms", sots);

  shared_ptr<SpanFirstBuilder> sft = make_shared<SpanFirstBuilder>(spanFactory);
  spanFactory->addBuilder(L"SpanFirst", sft);
  queryFactory->addBuilder(L"SpanFirst", sft);

  shared_ptr<SpanNotBuilder> snot = make_shared<SpanNotBuilder>(spanFactory);
  spanFactory->addBuilder(L"SpanNot", snot);
  queryFactory->addBuilder(L"SpanNot", snot);
}

shared_ptr<Query>
CoreParser::parse(shared_ptr<InputStream> xmlStream) 
{
  return getQuery(parseXML(xmlStream)->getDocumentElement());
}

shared_ptr<SpanQuery> CoreParser::parseAsSpanQuery(
    shared_ptr<InputStream> xmlStream) 
{
  return getSpanQuery(parseXML(xmlStream)->getDocumentElement());
}

void CoreParser::addQueryBuilder(const wstring &nodeName,
                                 shared_ptr<QueryBuilder> builder)
{
  queryFactory->addBuilder(nodeName, builder);
}

void CoreParser::addSpanBuilder(const wstring &nodeName,
                                shared_ptr<SpanQueryBuilder> builder)
{
  spanFactory->addBuilder(nodeName, builder);
}

void CoreParser::addSpanQueryBuilder(const wstring &nodeName,
                                     shared_ptr<SpanQueryBuilder> builder)
{
  queryFactory->addBuilder(nodeName, builder);
  spanFactory->addBuilder(nodeName, builder);
}

shared_ptr<EntityResolver> CoreParser::getEntityResolver()
{
  return DISALLOW_EXTERNAL_ENTITY_RESOLVER;
}

shared_ptr<ErrorHandler> CoreParser::getErrorHandler() { return nullptr; }

shared_ptr<Document>
CoreParser::parseXML(shared_ptr<InputStream> pXmlFile) 
{
  shared_ptr<DocumentBuilderFactory> *const dbf =
      DocumentBuilderFactory::newInstance();
  dbf->setValidating(false);
  try {
    dbf->setFeature(XMLConstants::FEATURE_SECURE_PROCESSING, true);
  } catch (const ParserConfigurationException &e) {
    // ignore since all implementations are required to support the
    // {@link javax.xml.XMLConstants#FEATURE_SECURE_PROCESSING} feature
  }
  shared_ptr<DocumentBuilder> *const db;
  try {
    db = dbf->newDocumentBuilder();
  } catch (const runtime_error &se) {
    throw make_shared<ParserException>(L"XML Parser configuration error.", se);
  }
  try {
    db->setEntityResolver(getEntityResolver());
    db->setErrorHandler(getErrorHandler());
    return db->parse(pXmlFile);
  } catch (const runtime_error &se) {
    throw make_shared<ParserException>(L"Error parsing XML stream: " + se, se);
  }
}

shared_ptr<Query>
CoreParser::getQuery(shared_ptr<Element> e) 
{
  return queryFactory->getQuery(e);
}

shared_ptr<SpanQuery>
CoreParser::getSpanQuery(shared_ptr<Element> e) 
{
  return spanFactory->getSpanQuery(e);
}

const shared_ptr<org::xml::sax::EntityResolver>
    CoreParser::DISALLOW_EXTERNAL_ENTITY_RESOLVER =
        [&](const wstring &publicId, const wstring &systemId) {
          throw make_shared<SAXException>(
              wstring::format(Locale::ENGLISH,
                              L"External Entity resolving unsupported:  "
                              L"publicId=\"%s\" systemId=\"%s\"",
                              publicId, systemId));
        };
} // namespace org::apache::lucene::queryparser::xml