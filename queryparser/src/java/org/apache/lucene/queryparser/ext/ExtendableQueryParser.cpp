using namespace std;

#include "ExtendableQueryParser.h"

namespace org::apache::lucene::queryparser::ext
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using org::apache::lucene::queryparser::ext::Extensions::Pair;
using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using Query = org::apache::lucene::search::Query;
const shared_ptr<Extensions> ExtendableQueryParser::DEFAULT_EXTENSION =
    make_shared<Extensions>();

ExtendableQueryParser::ExtendableQueryParser(const wstring &f,
                                             shared_ptr<Analyzer> a)
    : ExtendableQueryParser(f, a, DEFAULT_EXTENSION)
{
}

ExtendableQueryParser::ExtendableQueryParser(const wstring &f,
                                             shared_ptr<Analyzer> a,
                                             shared_ptr<Extensions> ext)
    : org::apache::lucene::queryparser::classic::QueryParser(f, a),
      defaultField(f), extensions(ext)
{
}

wchar_t ExtendableQueryParser::getExtensionFieldDelimiter()
{
  return extensions->getExtensionFieldDelimiter();
}

shared_ptr<Query>
ExtendableQueryParser::getFieldQuery(const wstring &field,
                                     const wstring &queryText,
                                     bool quoted) 
{
  shared_ptr<Pair<wstring, wstring>> *const splitExtensionField =
      this->extensions->splitExtensionField(defaultField, field);
  shared_ptr<ParserExtension> *const extension =
      this->extensions->getExtension(splitExtensionField->cud);
  if (extension != nullptr) {
    return extension->parse(make_shared<ExtensionQuery>(
        shared_from_this(), splitExtensionField->cur, queryText));
  }
  return QueryParser::getFieldQuery(field, queryText, quoted);
}
} // namespace org::apache::lucene::queryparser::ext