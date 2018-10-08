using namespace std;

#include "TestTypeAsSynonymFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    TestTypeAsSynonymFilterFactory::TOKENS = {token(L"Visit", L"<ALPHANUM>"),
                                              token(L"example.com", L"<URL>")};

void TestTypeAsSynonymFilterFactory::testBasic() 
{
  shared_ptr<TokenStream> stream = make_shared<CannedTokenStream>(TOKENS);
  stream = tokenFilterFactory(L"TypeAsSynonym").create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"Visit", L"<ALPHANUM>", L"example.com", L"<URL>"},
      nullptr, nullptr,
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>", L"<URL>", L"<URL>"},
      std::deque<int>{1, 0, 1, 0});
}

void TestTypeAsSynonymFilterFactory::testPrefix() 
{
  shared_ptr<TokenStream> stream = make_shared<CannedTokenStream>(TOKENS);
  stream = tokenFilterFactory(L"TypeAsSynonym", {L"prefix", L"_type_"})
               ->create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"Visit", L"_type_<ALPHANUM>", L"example.com",
                           L"_type_<URL>"},
      nullptr, nullptr,
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>", L"<URL>", L"<URL>"},
      std::deque<int>{1, 0, 1, 0});
}

shared_ptr<Token> TestTypeAsSynonymFilterFactory::token(const wstring &term,
                                                        const wstring &type)
{
  shared_ptr<Token> token = make_shared<Token>();
  token->setEmpty();
  token->append(term);
  token->setType(type);
  return token;
}
} // namespace org::apache::lucene::analysis::miscellaneous