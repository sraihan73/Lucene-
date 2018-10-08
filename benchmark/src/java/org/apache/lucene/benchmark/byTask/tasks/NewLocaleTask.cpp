using namespace std;

#include "NewLocaleTask.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

NewLocaleTask::NewLocaleTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

shared_ptr<Locale> NewLocaleTask::createLocale(const wstring &language,
                                               const wstring &country,
                                               const wstring &variant)
{
  if (language == L"" || language.length() == 0) {
    return nullptr;
  }

  wstring lang = language;
  // C++ TODO: The following Java case-insensitive std::wstring method call is not
  // converted:
  if (lang.equalsIgnoreCase(L"ROOT")) {
    lang = L""; // empty language is the root locale in the JDK
  }

  return make_shared<Locale>(lang, country, variant);
}

int NewLocaleTask::doLogic() 
{
  shared_ptr<Locale> locale = createLocale(language, country, variant);
  getRunData()->setLocale(locale);
  wcout << L"Changed Locale to: "
        << (locale == nullptr
                ? L"null"
                : (locale->getDisplayName(Locale::ENGLISH)->length() == 0)
                      ? L"root locale"
                      : locale)
        << endl;
  return 1;
}

void NewLocaleTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  language = country = variant = L"";
  shared_ptr<StringTokenizer> st = make_shared<StringTokenizer>(params, L",");
  if (st->hasMoreTokens()) {
    language = st->nextToken();
  }
  if (st->hasMoreTokens()) {
    country = st->nextToken();
  }
  if (st->hasMoreTokens()) {
    variant = st->nextToken();
  }
}

bool NewLocaleTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks