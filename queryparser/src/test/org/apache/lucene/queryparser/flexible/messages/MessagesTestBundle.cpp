using namespace std;

#include "MessagesTestBundle.h"

namespace org::apache::lucene::queryparser::flexible::messages
{

const wstring MessagesTestBundle::BUNDLE_NAME =
    MessagesTestBundle::typeid->getName();

MessagesTestBundle::MessagesTestBundle()
{
  // should never be instantiated
}

MessagesTestBundle::StaticConstructor::StaticConstructor()
{
  // register all string ids with NLS class and initialize static string
  // values
  NLS::initializeMessages(BUNDLE_NAME, MessagesTestBundle::typeid);
}

MessagesTestBundle::StaticConstructor MessagesTestBundle::staticConstructor;
wstring MessagesTestBundle::Q0001E_INVALID_SYNTAX;
wstring MessagesTestBundle::Q0004E_INVALID_SYNTAX_ESCAPE_UNICODE_TRUNCATION;
wstring MessagesTestBundle::Q0005E_MESSAGE_NOT_IN_BUNDLE;
} // namespace org::apache::lucene::queryparser::flexible::messages