using namespace std;

#include "TestNLS.h"

namespace org::apache::lucene::queryparser::flexible::messages
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestNLS::testMessageLoading()
{
  shared_ptr<Message> invalidSyntax = make_shared<MessageImpl>(
      MessagesTestBundle::Q0001E_INVALID_SYNTAX, L"XXX");
  /*
   * if the default locale is ja, you get ja as a fallback:
   * see ResourceBundle.html#getBundle(java.lang.std::wstring, java.util.Locale,
   * java.lang.ClassLoader)
   */
  if (!Locale::getDefault().getLanguage().equals(L"ja")) {
    assertEquals(L"Syntax Error: XXX",
                 invalidSyntax->getLocalizedMessage(Locale::ENGLISH));
  }
}

void TestNLS::testMessageLoading_ja()
{
  shared_ptr<Message> invalidSyntax = make_shared<MessageImpl>(
      MessagesTestBundle::Q0001E_INVALID_SYNTAX, L"XXX");
  assertEquals(L"構文エラー: XXX",
               invalidSyntax->getLocalizedMessage(Locale::JAPANESE));
}

void TestNLS::testNLSLoading()
{
  wstring message = NLS::getLocalizedMessage(
      MessagesTestBundle::Q0004E_INVALID_SYNTAX_ESCAPE_UNICODE_TRUNCATION,
      Locale::ENGLISH);
  /*
   * if the default locale is ja, you get ja as a fallback:
   * see ResourceBundle.html#getBundle(java.lang.std::wstring, java.util.Locale,
   * java.lang.ClassLoader)
   */
  if (!Locale::getDefault().getLanguage().equals(L"ja")) {
    assertEquals(L"Truncated unicode escape sequence.", message);
  }

  message = NLS::getLocalizedMessage(MessagesTestBundle::Q0001E_INVALID_SYNTAX,
                                     Locale::ENGLISH, L"XXX");
  /*
   * if the default locale is ja, you get ja as a fallback:
   * see ResourceBundle.html#getBundle(java.lang.std::wstring, java.util.Locale,
   * java.lang.ClassLoader)
   */
  if (!Locale::getDefault().getLanguage().equals(L"ja")) {
    assertEquals(L"Syntax Error: XXX", message);
  }
}

void TestNLS::testNLSLoading_ja()
{
  wstring message = NLS::getLocalizedMessage(
      MessagesTestBundle::Q0004E_INVALID_SYNTAX_ESCAPE_UNICODE_TRUNCATION,
      Locale::JAPANESE);
  assertEquals(L"切り捨てられたユニコード・エスケープ・シーケンス。", message);

  message = NLS::getLocalizedMessage(MessagesTestBundle::Q0001E_INVALID_SYNTAX,
                                     Locale::JAPANESE, L"XXX");
  assertEquals(L"構文エラー: XXX", message);
}

void TestNLS::testNLSLoading_xx_XX()
{
  shared_ptr<Locale> locale = make_shared<Locale>(L"xx", L"XX", L"");
  wstring message = NLS::getLocalizedMessage(
      MessagesTestBundle::Q0004E_INVALID_SYNTAX_ESCAPE_UNICODE_TRUNCATION,
      locale);
  /*
   * if the default locale is ja, you get ja as a fallback:
   * see ResourceBundle.html#getBundle(java.lang.std::wstring, java.util.Locale,
   * java.lang.ClassLoader)
   */
  if (!Locale::getDefault().getLanguage().equals(L"ja")) {
    assertEquals(L"Truncated unicode escape sequence.", message);
  }

  message = NLS::getLocalizedMessage(MessagesTestBundle::Q0001E_INVALID_SYNTAX,
                                     locale, L"XXX");
  /*
   * if the default locale is ja, you get ja as a fallback:
   * see ResourceBundle.html#getBundle(java.lang.std::wstring, java.util.Locale,
   * java.lang.ClassLoader)
   */
  if (!Locale::getDefault().getLanguage().equals(L"ja")) {
    assertEquals(L"Syntax Error: XXX", message);
  }
}

void TestNLS::testMissingMessage()
{
  shared_ptr<Locale> locale = Locale::ENGLISH;
  wstring message = NLS::getLocalizedMessage(
      MessagesTestBundle::Q0005E_MESSAGE_NOT_IN_BUNDLE, locale);

  assertEquals(L"Message with key:Q0005E_MESSAGE_NOT_IN_BUNDLE and locale: " +
                   locale->toLanguageTag() + L" not found.",
               message);
}
} // namespace org::apache::lucene::queryparser::flexible::messages