using namespace std;

#include "ParserException.h"

namespace org::apache::lucene::queryparser::xml
{

ParserException::ParserException() : Exception() {}

ParserException::ParserException(const wstring &message) : Exception(message) {}

ParserException::ParserException(const wstring &message, runtime_error cause)
    : Exception(message, cause)
{
}

ParserException::ParserException(runtime_error cause) : Exception(cause) {}
} // namespace org::apache::lucene::queryparser::xml