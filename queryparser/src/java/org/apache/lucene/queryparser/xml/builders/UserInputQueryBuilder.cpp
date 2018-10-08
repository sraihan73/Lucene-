using namespace std;

#include "UserInputQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Query = org::apache::lucene::search::Query;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using org::w3c::dom::Element;

UserInputQueryBuilder::UserInputQueryBuilder(shared_ptr<QueryParser> parser)
{
  this->unSafeParser = parser;
}

UserInputQueryBuilder::UserInputQueryBuilder(const wstring &defaultField,
                                             shared_ptr<Analyzer> analyzer)
{
  this->analyzer = analyzer;
  this->defaultField = defaultField;
}

shared_ptr<Query>
UserInputQueryBuilder::getQuery(shared_ptr<Element> e) 
{
  wstring text = DOMUtils::getText(e);
  try {
    shared_ptr<Query> q = nullptr;
    if (unSafeParser != nullptr) {
      {
        // synchronize on unsafe parser
        lock_guard<mutex> lock(unSafeParser);
        q = unSafeParser->parse(text);
      }
    } else {
      wstring fieldName = DOMUtils::getAttribute(e, L"fieldName", defaultField);
      // Create new parser
      shared_ptr<QueryParser> parser = createQueryParser(fieldName, analyzer);
      q = parser->parse(text);
    }
    float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
    return make_shared<BoostQuery>(q, boost);
  } catch (const ParseException &e1) {
    throw make_shared<ParserException>(e1->what());
  }
}

shared_ptr<QueryParser>
UserInputQueryBuilder::createQueryParser(const wstring &fieldName,
                                         shared_ptr<Analyzer> analyzer)
{
  return make_shared<QueryParser>(fieldName, analyzer);
}
} // namespace org::apache::lucene::queryparser::xml::builders