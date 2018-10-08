using namespace std;

#include "PointRangeQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using Query = org::apache::lucene::search::Query;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using PointValues = org::apache::lucene::index::PointValues;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using org::w3c::dom::Element;

shared_ptr<Query>
PointRangeQueryBuilder::getQuery(shared_ptr<Element> e) 
{
  wstring field = DOMUtils::getAttributeWithInheritanceOrFail(e, L"fieldName");
  const wstring lowerTerm = DOMUtils::getAttribute(e, L"lowerTerm", L"");
  const wstring upperTerm = DOMUtils::getAttribute(e, L"upperTerm", L"");

  wstring type = DOMUtils::getAttribute(e, L"type", L"int");
  try {
    // C++ TODO: The following Java case-insensitive std::wstring method call is not
    // converted:
    if (type.equalsIgnoreCase(L"int")) {
      return IntPoint::newRangeQuery(
          field,
          (lowerTerm == L"" ? numeric_limits<int>::min() : stoi(lowerTerm)),
          (upperTerm == L"" ? numeric_limits<int>::max() : stoi(upperTerm)));
    }
    // C++ TODO: The following Java case-insensitive std::wstring method call is not
    // converted:
    else if (type.equalsIgnoreCase(L"long")) {
      return LongPoint::newRangeQuery(
          field,
          (lowerTerm == L"" ? numeric_limits<int64_t>::min()
                            : StringHelper::fromString<int64_t>(lowerTerm)),
          (upperTerm == L"" ? numeric_limits<int64_t>::max()
                            : StringHelper::fromString<int64_t>(upperTerm)));
    }
    // C++ TODO: The following Java case-insensitive std::wstring method call is not
    // converted:
    else if (type.equalsIgnoreCase(L"double")) {
      return DoublePoint::newRangeQuery(
          field,
          (lowerTerm == L"" ? -numeric_limits<double>::infinity()
                            : stod(lowerTerm)),
          (upperTerm == L"" ? numeric_limits<double>::infinity()
                            : stod(upperTerm)));
    }
    // C++ TODO: The following Java case-insensitive std::wstring method call is not
    // converted:
    else if (type.equalsIgnoreCase(L"float")) {
      return FloatPoint::newRangeQuery(
          field,
          (lowerTerm == L"" ? -numeric_limits<float>::infinity()
                            : stof(lowerTerm)),
          (upperTerm == L"" ? numeric_limits<float>::infinity()
                            : stof(upperTerm)));
    } else {
      throw make_shared<ParserException>(
          L"type attribute must be one of: [long, int, double, float]");
    }
  } catch (const NumberFormatException &nfe) {
    throw make_shared<ParserException>(
        L"Could not parse lowerTerm or upperTerm into a number", nfe);
  }
}
} // namespace org::apache::lucene::queryparser::xml::builders