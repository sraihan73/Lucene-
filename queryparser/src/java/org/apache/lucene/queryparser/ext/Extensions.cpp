using namespace std;

#include "Extensions.h"

namespace org::apache::lucene::queryparser::ext
{
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using QueryParserBase =
    org::apache::lucene::queryparser::classic::QueryParserBase;

Extensions::Extensions() : Extensions(DEFAULT_EXTENSION_FIELD_DELIMITER) {}

Extensions::Extensions(wchar_t extensionFieldDelimiter)
    : extensionFieldDelimiter(extensionFieldDelimiter)
{
}

void Extensions::add(const wstring &key, shared_ptr<ParserExtension> extension)
{
  this->extensions.emplace(key, extension);
}

shared_ptr<ParserExtension> Extensions::getExtension(const wstring &key)
{
  return this->extensions[key];
}

wchar_t Extensions::getExtensionFieldDelimiter()
{
  return extensionFieldDelimiter;
}

shared_ptr<Pair<wstring, wstring>>
Extensions::splitExtensionField(const wstring &defaultField,
                                const wstring &field)
{
  int indexOf = (int)field.find(this->extensionFieldDelimiter);
  if (indexOf < 0) {
    return make_shared<Pair<wstring, wstring>>(field, nullptr);
  }
  const wstring indexField =
      indexOf == 0 ? defaultField : field.substr(0, indexOf);
  const wstring extensionKey = field.substr(indexOf + 1);
  return make_shared<Pair<wstring, wstring>>(indexField, extensionKey);
}

wstring Extensions::escapeExtensionField(const wstring &extfield)
{
  return QueryParserBase::escape(extfield);
}

wstring Extensions::buildExtensionField(const wstring &extensionKey)
{
  return buildExtensionField(extensionKey, L"");
}

wstring Extensions::buildExtensionField(const wstring &extensionKey,
                                        const wstring &field)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>(field);
  builder->append(this->extensionFieldDelimiter);
  builder->append(extensionKey);
  return escapeExtensionField(builder->toString());
}
} // namespace org::apache::lucene::queryparser::ext