using namespace std;

#include "SpatialArgsParser.h"

namespace org::apache::lucene::spatial::query
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::exception::InvalidShapeException;
using org::locationtech::spatial4j::shape::Shape;
const wstring SpatialArgsParser::DIST_ERR_PCT = L"distErrPct";
const wstring SpatialArgsParser::DIST_ERR = L"distErr";

wstring SpatialArgsParser::writeSpatialArgs(shared_ptr<SpatialArgs> args)
{
  shared_ptr<StringBuilder> str = make_shared<StringBuilder>();
  str->append(args->getOperation()->getName());
  str->append(L'(');
  // C++ TODO: There is no native C++ equivalent to 'toString':
  str->append(args->getShape()->toString());
  if (args->getDistErrPct()) {
    str->append(L" distErrPct=")
        ->append(wstring::format(Locale::ROOT, L"%.2f%%",
                                 args->getDistErrPct() * 100));
  }
  if (args->getDistErr()) {
    str->append(L" distErr=")->append(args->getDistErr());
  }
  str->append(L')');
  return str->toString();
}

shared_ptr<SpatialArgs> SpatialArgsParser::parse(
    const wstring &v,
    shared_ptr<SpatialContext> ctx) 
{
  int idx = (int)v.find(L'(');
  int edx = (int)v.rfind(L')');

  if (idx < 0 || idx > edx) {
    throw make_shared<ParseException>(L"missing parens: " + v, -1);
  }

  shared_ptr<SpatialOperation> op =
      SpatialOperation::get(v.substr(0, idx)->trim());

  wstring body = v.substr(idx + 1, edx - (idx + 1))->trim();
  if (body.length() < 1) {
    throw make_shared<ParseException>(L"missing body : " + v, idx + 1);
  }

  shared_ptr<Shape> shape = parseShape(body, ctx);
  shared_ptr<SpatialArgs> args = newSpatialArgs(op, shape);

  if (v.length() > (edx + 1)) {
    body = v.substr(edx + 1)->trim();
    if (body.length() > 0) {
      unordered_map<wstring, wstring> aa = parseMap(body);
      readNameValuePairs(args, aa);
      if (!aa.empty()) {
        throw invalid_argument(L"unused parameters: " + aa);
      }
    }
  }
  args->validate();
  return args;
}

shared_ptr<SpatialArgs>
SpatialArgsParser::newSpatialArgs(shared_ptr<SpatialOperation> op,
                                  shared_ptr<Shape> shape)
{
  return make_shared<SpatialArgs>(op, shape);
}

void SpatialArgsParser::readNameValuePairs(
    shared_ptr<SpatialArgs> args, unordered_map<wstring, wstring> &nameValPairs)
{
  args->setDistErrPct(readDouble(nameValPairs.erase(DIST_ERR_PCT)));
  args->setDistErr(readDouble(nameValPairs.erase(DIST_ERR)));
}

shared_ptr<Shape> SpatialArgsParser::parseShape(
    const wstring &str, shared_ptr<SpatialContext> ctx) 
{
  // return ctx.readShape(str);//still in Spatial4j 0.4 but will be deleted
  return ctx->readShapeFromWkt(str);
}

optional<double> SpatialArgsParser::readDouble(const wstring &v)
{
  return v == L"" ? nullopt : stod(v);
}

bool SpatialArgsParser::readBool(const wstring &v, bool defaultValue)
{
  return v == L"" ? defaultValue : StringHelper::fromString<bool>(v);
}

unordered_map<wstring, wstring> SpatialArgsParser::parseMap(const wstring &body)
{
  unordered_map<wstring, wstring> map_obj = unordered_map<wstring, wstring>();
  shared_ptr<StringTokenizer> st = make_shared<StringTokenizer>(body, L" \n\t");
  while (st->hasMoreTokens()) {
    wstring a = st->nextToken();
    int idx = (int)a.find(L'=');
    if (idx > 0) {
      wstring k = a.substr(0, idx);
      wstring v = a.substr(idx + 1);
      map_obj.emplace(k, v);
    } else {
      map_obj.emplace(a, a);
    }
  }
  return map_obj;
}
} // namespace org::apache::lucene::spatial::query