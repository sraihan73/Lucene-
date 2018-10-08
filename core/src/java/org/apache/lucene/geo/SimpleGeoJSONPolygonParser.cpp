using namespace std;

#include "SimpleGeoJSONPolygonParser.h"

namespace org::apache::lucene::geo
{

SimpleGeoJSONPolygonParser::SimpleGeoJSONPolygonParser(const wstring &input)
    : input(input)
{
}

std::deque<std::shared_ptr<Polygon>>
SimpleGeoJSONPolygonParser::parse() 
{
  // parse entire object
  parseObject(L"");

  // make sure there's nothing left:
  readEnd();

  // The order of JSON object keys (type, geometry, coordinates in our case) can
  // be arbitrary, so we wait until we are done parsing to put the pieces
  // together here:

  if (coordinates.empty()) {
    throw newParseException(L"did not see any polygon coordinates");
  }

  if (polyType == L"") {
    throw newParseException(L"did not see type: Polygon or MultiPolygon");
  }

  if (polyType == L"Polygon") {
    return std::deque<std::shared_ptr<Polygon>>{parsePolygon(coordinates)};
  } else {
    deque<std::shared_ptr<Polygon>> polygons =
        deque<std::shared_ptr<Polygon>>();
    for (int i = 0; i < coordinates.size(); i++) {
      any o = coordinates[i];
      if (dynamic_cast<deque>(o) != nullptr == false) {
        throw newParseException(
            L"elements of coordinates array should be an array, but got: " +
            o.type());
      }
      polygons.push_back(parsePolygon(any_cast<deque<any>>(o)));
    }

    return polygons.toArray(
        std::deque<std::shared_ptr<Polygon>>(polygons.size()));
  }
}

void SimpleGeoJSONPolygonParser::parseObject(const wstring &path) throw(
    ParseException)
{
  scan(L'{');
  bool first = true;
  while (true) {
    wchar_t ch = peek();
    if (ch == L'}') {
      break;
    } else if (first == false) {
      if (ch == L',') {
        // ok
        upto++;
        ch = peek();
        if (ch == L'}') {
          break;
        }
      } else {
        throw newParseException(L"expected , but got " +
                                StringHelper::toString(ch));
      }
    }

    first = false;

    int uptoStart = upto;
    wstring key = parseString();

    if (path == L"crs.properties" && key == L"href") {
      upto = uptoStart;
      throw newParseException(L"cannot handle linked crs");
    }

    scan(L':');

    any o;

    ch = peek();

    uptoStart = upto;

    if (ch == L'[') {
      wstring newPath;
      if (path.length() == 0) {
        newPath = key;
      } else {
        newPath = path + L"." + key;
      }
      o = parseArray(newPath);
    } else if (ch == L'{') {
      wstring newPath;
      if (path.length() == 0) {
        newPath = key;
      } else {
        newPath = path + L"." + key;
      }
      parseObject(newPath);
      o = nullptr;
    } else if (ch == L'"') {
      o = parseString();
    } else if (ch == L't') {
      scan(L"true");
      o = Boolean::TRUE;
    } else if (ch == L'f') {
      scan(L"false");
      o = Boolean::FALSE;
    } else if (ch == L'n') {
      scan(L"null");
      o = nullptr;
    } else if (ch == L'-' || ch == L'.' || (ch >= L'0' && ch <= L'9')) {
      o = parseNumber();
    } else if (ch == L'}') {
      break;
    } else {
      throw newParseException(
          L"expected array, object, string or literal value, but got: " +
          StringHelper::toString(ch));
    }

    if (path == L"crs.properties" && key == L"name") {
      if (dynamic_cast<wstring>(o) != nullptr == false) {
        upto = uptoStart;
        throw newParseException(
            L"crs.properties.name should be a string, but saw: " + o);
      }
      wstring crs = any_cast<wstring>(o);
      if (StringHelper::startsWith(crs, L"urn:ogc:def:crs:OGC") == false ||
          StringHelper::endsWith(crs, L":CRS84") == false) {
        upto = uptoStart;
        throw newParseException(L"crs must be CRS84 from OGC, but saw: " + o);
      }
    }

    if (key == L"type" && StringHelper::startsWith(path, L"crs") == false) {
      if (dynamic_cast<wstring>(o) != nullptr == false) {
        upto = uptoStart;
        throw newParseException(L"type should be a string, but got: " + o);
      }
      wstring type = any_cast<wstring>(o);
      if (type == L"Polygon" && isValidGeometryPath(path)) {
        polyType = L"Polygon";
      } else if (type == L"MultiPolygon" && isValidGeometryPath(path)) {
        polyType = L"MultiPolygon";
      } else if ((type == L"FeatureCollection" || type == L"Feature") &&
                 (path == L"features.[]" || path == L"")) {
        // OK, we recurse
      } else {
        upto = uptoStart;
        throw newParseException(
            L"can only handle type FeatureCollection (if it has a single "
            L"polygon geometry), Feature, Polygon or MutiPolygon, but got " +
            type);
      }
    } else if (key == L"coordinates" && isValidGeometryPath(path)) {
      if (dynamic_cast<deque>(o) != nullptr == false) {
        upto = uptoStart;
        throw newParseException(L"coordinates should be an array, but got: " +
                                o.type());
      }
      if (coordinates.size() > 0) {
        upto = uptoStart;
        throw newParseException(
            L"only one Polygon or MultiPolygon is supported");
      }
      coordinates = any_cast<deque<any>>(o);
    }
  }

  scan(L'}');
}

bool SimpleGeoJSONPolygonParser::isValidGeometryPath(const wstring &path)
{
  return path == L"" || path == L"geometry" || path == L"features.[].geometry";
}

shared_ptr<Polygon> SimpleGeoJSONPolygonParser::parsePolygon(
    deque<any> &coordinates) 
{
  deque<std::shared_ptr<Polygon>> holes = deque<std::shared_ptr<Polygon>>();
  any o = coordinates[0];
  if (dynamic_cast<deque>(o) != nullptr == false) {
    throw newParseException(L"first element of polygon array must be an array "
                            L"[[lat, lon], [lat, lon] ...] but got: " +
                            o);
  }
  std::deque<std::deque<double>> polyPoints =
      parsePoints(any_cast<deque<any>>(o));
  for (int i = 1; i < coordinates.size(); i++) {
    o = coordinates[i];
    if (dynamic_cast<deque>(o) != nullptr == false) {
      throw newParseException(L"elements of coordinates array must be an array "
                              L"[[lat, lon], [lat, lon] ...] but got: " +
                              o);
    }
    std::deque<std::deque<double>> holePoints =
        parsePoints(any_cast<deque<any>>(o));
    holes.push_back(make_shared<Polygon>(holePoints[0], holePoints[1]));
  }
  return make_shared<Polygon>(
      polyPoints[0], polyPoints[1],
      holes.toArray(std::deque<std::shared_ptr<Polygon>>(holes.size())));
}

std::deque<std::deque<double>>
SimpleGeoJSONPolygonParser::parsePoints(deque<any> &o) 
{
  std::deque<double> lats(o.size());
  std::deque<double> lons(o.size());
  for (int i = 0; i < o.size(); i++) {
    any point = o[i];
    if (dynamic_cast<deque>(point) != nullptr == false) {
      throw newParseException(
          L"elements of coordinates array must [lat, lon] array, but got: " +
          point);
    }
    deque<any> pointList = any_cast<deque<any>>(point);
    if (pointList.size() != 2) {
      throw newParseException(L"elements of coordinates array must [lat, lon] "
                              L"array, but got wrong element count: " +
                              pointList);
    }
    if (dynamic_cast<Double>(pointList[0]) != nullptr == false) {
      throw newParseException(L"elements of coordinates array must [lat, lon] "
                              L"array, but first element is not a Double: " +
                              pointList[0]);
    }
    if (dynamic_cast<Double>(pointList[1]) != nullptr == false) {
      throw newParseException(L"elements of coordinates array must [lat, lon] "
                              L"array, but second element is not a Double: " +
                              pointList[1]);
    }

    // lon, lat ordering in GeoJSON!
    lons[i] = (any_cast<optional<double>>(pointList[0])).value();
    lats[i] = (any_cast<optional<double>>(pointList[1])).value();
  }

  return std::deque<std::deque<double>>{lats, lons};
}

deque<any> SimpleGeoJSONPolygonParser::parseArray(const wstring &path) throw(
    ParseException)
{
  deque<any> result = deque<any>();
  scan(L'[');
  while (upto < input.length()) {
    wchar_t ch = peek();
    if (ch == L']') {
      scan(L']');
      return result;
    }

    if (result.size() > 0) {
      if (ch != L',') {
        throw newParseException(
            L"expected ',' separating deque items, but got '" +
            StringHelper::toString(ch) + L"'");
      }

      // skip the ,
      upto++;

      if (upto == input.length()) {
        throw newParseException(L"hit EOF while parsing array");
      }
      ch = peek();
    }

    any o;
    if (ch == L'[') {
      o = parseArray(path + L".[]");
    } else if (ch == L'{') {
      // This is only used when parsing the "features" in type:
      // FeatureCollection
      parseObject(path + L".[]");
      o = nullptr;
    } else if (ch == L'-' || ch == L'.' || (ch >= L'0' && ch <= L'9')) {
      o = parseNumber();
    } else {
      throw newParseException(
          L"expected another array or number while parsing array, not '" +
          StringHelper::toString(ch) + L"'");
    }

    result.push_back(o);
  }

  throw newParseException(L"hit EOF while reading array");
}

shared_ptr<Number>
SimpleGeoJSONPolygonParser::parseNumber() 
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  int uptoStart = upto;
  while (upto < input.length()) {
    wchar_t ch = input[upto];
    if (ch == L'-' || ch == L'.' || (ch >= L'0' && ch <= L'9') || ch == L'e' ||
        ch == L'E') {
      upto++;
      b->append(ch);
    } else {
      break;
    }
  }

  // we only handle doubles
  try {
    return static_cast<Double>(b->toString());
  } catch (const NumberFormatException &nfe) {
    upto = uptoStart;
    throw newParseException(L"could not parse number as double");
  }
}

wstring SimpleGeoJSONPolygonParser::parseString() 
{
  scan(L'"');
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  while (upto < input.length()) {
    wchar_t ch = input[upto];
    if (ch == L'"') {
      upto++;
      return b->toString();
    }
    if (ch == L'\\') {
      // an escaped character
      upto++;
      if (upto == input.length()) {
        throw newParseException(L"hit EOF inside string literal");
      }
      ch = input[upto];
      if (ch == L'u') {
        // 4 hex digit unicode BMP escape
        upto++;
        if (upto + 4 > input.length()) {
          throw newParseException(L"hit EOF inside string literal");
        }
        // C++ TODO: Only single-argument parse and valueOf methods are
        // converted: ORIGINAL LINE:
        // b.append(Integer.parseInt(input.substring(upto, 4), 16));
        b->append(Integer::valueOf(input.substr(upto, 4), 16));
      } else if (ch == L'\\') {
        b->append(L'\\');
        upto++;
      } else {
        // TODO: allow \n, \t, etc.???
        throw newParseException(L"unsupported string escape character \\" +
                                StringHelper::toString(ch));
      }
    } else {
      b->append(ch);
      upto++;
    }
  }

  throw newParseException(L"hit EOF inside string literal");
}

wchar_t SimpleGeoJSONPolygonParser::peek() 
{
  while (upto < input.length()) {
    wchar_t ch = input[upto];
    if (isJSONWhitespace(ch)) {
      upto++;
      continue;
    }
    return ch;
  }

  throw newParseException(L"unexpected EOF");
}

void SimpleGeoJSONPolygonParser::scan(wchar_t expected) 
{
  while (upto < input.length()) {
    wchar_t ch = input[upto];
    if (isJSONWhitespace(ch)) {
      upto++;
      continue;
    }
    if (ch != expected) {
      throw newParseException(L"expected '" + StringHelper::toString(expected) +
                              L"' but got '" + StringHelper::toString(ch) +
                              L"'");
    }
    upto++;
    return;
  }
  throw newParseException(L"expected '" + StringHelper::toString(expected) +
                          L"' but got EOF");
}

void SimpleGeoJSONPolygonParser::readEnd() 
{
  while (upto < input.length()) {
    wchar_t ch = input[upto];
    if (isJSONWhitespace(ch) == false) {
      throw newParseException(L"unexpected character '" +
                              StringHelper::toString(ch) +
                              L"' after end of GeoJSON object");
    }
    upto++;
  }
}

void SimpleGeoJSONPolygonParser::scan(const wstring &expected) throw(
    ParseException)
{
  if (upto + expected.length() > input.length()) {
    throw newParseException(L"expected \"" + expected + L"\" but hit EOF");
  }
  wstring subString = input.substr(upto, expected.length());
  if (subString == expected == false) {
    throw newParseException(L"expected \"" + expected + L"\" but got \"" +
                            subString + L"\"");
  }
  upto += expected.length();
}

bool SimpleGeoJSONPolygonParser::isJSONWhitespace(wchar_t ch)
{
  // JSON doesn't accept allow unicode whitespace?
  return ch == 0x20 || ch == 0x09 || ch == 0x0a || ch == 0x0d; // newline
}

shared_ptr<ParseException> SimpleGeoJSONPolygonParser::newParseException(
    const wstring &details) 
{
  wstring fragment;
  int end = min(input.length(), upto + 1);
  if (upto < 50) {
    fragment = input.substr(0, end);
  } else {
    fragment = L"..." + input.substr(upto - 50, end - (upto - 50));
  }
  return make_shared<ParseException>(
      details + L" at character offset " + to_wstring(upto) +
          L"; fragment leading to this:\n" + fragment,
      upto);
}
} // namespace org::apache::lucene::geo