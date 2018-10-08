using namespace std;

#include "BrazilianStemmer.h"

namespace org::apache::lucene::analysis::br
{

const shared_ptr<java::util::Locale> BrazilianStemmer::locale =
    make_shared<java::util::Locale>(L"pt", L"BR");

BrazilianStemmer::BrazilianStemmer() {}

wstring BrazilianStemmer::stem(const wstring &term)
{
  bool altered = false; // altered the term

  // creates CT
  createCT(term);

  if (!isIndexable(CT)) {
    return L"";
  }
  if (!isStemmable(CT)) {
    return CT;
  }

  R1 = getR1(CT);
  R2 = getR1(R1);
  RV = getRV(CT);
  TERM = term + L";" + CT;

  altered = step1();
  if (!altered) {
    altered = step2();
  }

  if (altered) {
    step3();
  } else {
    step4();
  }

  step5();

  return CT;
}

bool BrazilianStemmer::isStemmable(const wstring &term)
{
  for (int c = 0; c < term.length(); c++) {
    // Discard terms that contain non-letter characters.
    if (!isalpha(term[c])) {
      return false;
    }
  }
  return true;
}

bool BrazilianStemmer::isIndexable(const wstring &term)
{
  return (term.length() < 30) && (term.length() > 2);
}

bool BrazilianStemmer::isVowel(wchar_t value)
{
  return (value == L'a') || (value == L'e') || (value == L'i') ||
         (value == L'o') || (value == L'u');
}

wstring BrazilianStemmer::getR1(const wstring &value)
{
  int i;
  int j;

  // be-safe !!!
  if (value == L"") {
    return L"";
  }

  // find 1st vowel
  i = value.length() - 1;
  for (j = 0; j < i; j++) {
    if (isVowel(value[j])) {
      break;
    }
  }

  if (!(j < i)) {
    return L"";
  }

  // find 1st non-vowel
  for (; j < i; j++) {
    if (!(isVowel(value[j]))) {
      break;
    }
  }

  if (!(j < i)) {
    return L"";
  }

  return value.substr(j + 1);
}

wstring BrazilianStemmer::getRV(const wstring &value)
{
  int i;
  int j;

  // be-safe !!!
  if (value == L"") {
    return L"";
  }

  i = value.length() - 1;

  // RV - IF the second letter is a consonant, RV is the region after
  //      the next following vowel,
  if ((i > 0) && !isVowel(value[1])) {
    // find 1st vowel
    for (j = 2; j < i; j++) {
      if (isVowel(value[j])) {
        break;
      }
    }

    if (j < i) {
      return value.substr(j + 1);
    }
  }

  // RV - OR if the first two letters are vowels, RV is the region
  //      after the next consonant,
  if ((i > 1) && isVowel(value[0]) && isVowel(value[1])) {
    // find 1st consoant
    for (j = 2; j < i; j++) {
      if (!isVowel(value[j])) {
        break;
      }
    }

    if (j < i) {
      return value.substr(j + 1);
    }
  }

  // RV - AND otherwise (consonant-vowel case) RV is the region after
  //      the third letter.
  if (i > 2) {
    return value.substr(3);
  }

  return L"";
}

wstring BrazilianStemmer::changeTerm(const wstring &value)
{
  int j;
  wstring r = L"";

  // be-safe !!!
  if (value == L"") {
    return L"";
  }

  value = value.toLowerCase(locale);
  for (j = 0; j < value.length(); j++) {
    if ((value[j] == L'á') || (value[j] == L'â') || (value[j] == L'ã')) {
      r = r + L"a";
      continue;
    }
    if ((value[j] == L'é') || (value[j] == L'ê')) {
      r = r + L"e";
      continue;
    }
    if (value[j] == L'í') {
      r = r + L"i";
      continue;
    }
    if ((value[j] == L'ó') || (value[j] == L'ô') || (value[j] == L'õ')) {
      r = r + L"o";
      continue;
    }
    if ((value[j] == L'ú') || (value[j] == L'ü')) {
      r = r + L"u";
      continue;
    }
    if (value[j] == L'ç') {
      r = r + L"c";
      continue;
    }
    if (value[j] == L'ñ') {
      r = r + L"n";
      continue;
    }

    r = r + StringHelper::toString(value[j]);
  }

  return r;
}

bool BrazilianStemmer::suffix(const wstring &value, const wstring &suffix)
{

  // be-safe !!!
  if ((value == L"") || (suffix == L"")) {
    return false;
  }

  if (suffix.length() > value.length()) {
    return false;
  }

  return value.substr(value.length() - suffix.length())->equals(suffix);
}

wstring BrazilianStemmer::replaceSuffix(const wstring &value,
                                        const wstring &toReplace,
                                        const wstring &changeTo)
{
  wstring vvalue;

  // be-safe !!!
  if ((value == L"") || (toReplace == L"") || (changeTo == L"")) {
    return value;
  }

  vvalue = removeSuffix(value, toReplace);

  if (value == vvalue) {
    return value;
  } else {
    return vvalue + changeTo;
  }
}

wstring BrazilianStemmer::removeSuffix(const wstring &value,
                                       const wstring &toRemove)
{
  // be-safe !!!
  if ((value == L"") || (toRemove == L"") || !suffix(value, toRemove)) {
    return value;
  }

  return value.substr(0, value.length() - toRemove.length());
}

bool BrazilianStemmer::suffixPreceded(const wstring &value,
                                      const wstring &suffix,
                                      const wstring &preceded)
{
  // be-safe !!!
  if ((value == L"") || (suffix == L"") || (preceded == L"") ||
      !this->suffix(value, suffix)) {
    return false;
  }

  return this->suffix(removeSuffix(value, suffix), preceded);
}

void BrazilianStemmer::createCT(const wstring &term)
{
  CT = changeTerm(term);

  if (CT.length() < 2) {
    return;
  }

  // if the first character is ... , remove it
  if ((CT[0] == L'"') || (CT[0] == L'\'') || (CT[0] == L'-') ||
      (CT[0] == L',') || (CT[0] == L';') || (CT[0] == L'.') ||
      (CT[0] == L'?') || (CT[0] == L'!')) {
    CT = CT.substr(1);
  }

  if (CT.length() < 2) {
    return;
  }

  // if the last character is ... , remove it
  if ((CT[CT.length() - 1] == L'-') || (CT[CT.length() - 1] == L',') ||
      (CT[CT.length() - 1] == L';') || (CT[CT.length() - 1] == L'.') ||
      (CT[CT.length() - 1] == L'?') || (CT[CT.length() - 1] == L'!') ||
      (CT[CT.length() - 1] == L'\'') || (CT[CT.length() - 1] == L'"')) {
    CT = CT.substr(0, CT.length() - 1);
  }
}

bool BrazilianStemmer::step1()
{
  if (CT == L"") {
    return false;
  }

  // suffix length = 7
  if (suffix(CT, L"uciones") && suffix(R2, L"uciones")) {
    CT = replaceSuffix(CT, L"uciones", L"u");
    return true;
  }

  // suffix length = 6
  if (CT.length() >= 6) {
    if (suffix(CT, L"imentos") && suffix(R2, L"imentos")) {
      CT = removeSuffix(CT, L"imentos");
      return true;
    }
    if (suffix(CT, L"amentos") && suffix(R2, L"amentos")) {
      CT = removeSuffix(CT, L"amentos");
      return true;
    }
    if (suffix(CT, L"adores") && suffix(R2, L"adores")) {
      CT = removeSuffix(CT, L"adores");
      return true;
    }
    if (suffix(CT, L"adoras") && suffix(R2, L"adoras")) {
      CT = removeSuffix(CT, L"adoras");
      return true;
    }
    if (suffix(CT, L"logias") && suffix(R2, L"logias")) {
      replaceSuffix(CT, L"logias", L"log");
      return true;
    }
    if (suffix(CT, L"encias") && suffix(R2, L"encias")) {
      CT = replaceSuffix(CT, L"encias", L"ente");
      return true;
    }
    if (suffix(CT, L"amente") && suffix(R1, L"amente")) {
      CT = removeSuffix(CT, L"amente");
      return true;
    }
    if (suffix(CT, L"idades") && suffix(R2, L"idades")) {
      CT = removeSuffix(CT, L"idades");
      return true;
    }
  }

  // suffix length = 5
  if (CT.length() >= 5) {
    if (suffix(CT, L"acoes") && suffix(R2, L"acoes")) {
      CT = removeSuffix(CT, L"acoes");
      return true;
    }
    if (suffix(CT, L"imento") && suffix(R2, L"imento")) {
      CT = removeSuffix(CT, L"imento");
      return true;
    }
    if (suffix(CT, L"amento") && suffix(R2, L"amento")) {
      CT = removeSuffix(CT, L"amento");
      return true;
    }
    if (suffix(CT, L"adora") && suffix(R2, L"adora")) {
      CT = removeSuffix(CT, L"adora");
      return true;
    }
    if (suffix(CT, L"ismos") && suffix(R2, L"ismos")) {
      CT = removeSuffix(CT, L"ismos");
      return true;
    }
    if (suffix(CT, L"istas") && suffix(R2, L"istas")) {
      CT = removeSuffix(CT, L"istas");
      return true;
    }
    if (suffix(CT, L"logia") && suffix(R2, L"logia")) {
      CT = replaceSuffix(CT, L"logia", L"log");
      return true;
    }
    if (suffix(CT, L"ucion") && suffix(R2, L"ucion")) {
      CT = replaceSuffix(CT, L"ucion", L"u");
      return true;
    }
    if (suffix(CT, L"encia") && suffix(R2, L"encia")) {
      CT = replaceSuffix(CT, L"encia", L"ente");
      return true;
    }
    if (suffix(CT, L"mente") && suffix(R2, L"mente")) {
      CT = removeSuffix(CT, L"mente");
      return true;
    }
    if (suffix(CT, L"idade") && suffix(R2, L"idade")) {
      CT = removeSuffix(CT, L"idade");
      return true;
    }
  }

  // suffix length = 4
  if (CT.length() >= 4) {
    if (suffix(CT, L"acao") && suffix(R2, L"acao")) {
      CT = removeSuffix(CT, L"acao");
      return true;
    }
    if (suffix(CT, L"ezas") && suffix(R2, L"ezas")) {
      CT = removeSuffix(CT, L"ezas");
      return true;
    }
    if (suffix(CT, L"icos") && suffix(R2, L"icos")) {
      CT = removeSuffix(CT, L"icos");
      return true;
    }
    if (suffix(CT, L"icas") && suffix(R2, L"icas")) {
      CT = removeSuffix(CT, L"icas");
      return true;
    }
    if (suffix(CT, L"ismo") && suffix(R2, L"ismo")) {
      CT = removeSuffix(CT, L"ismo");
      return true;
    }
    if (suffix(CT, L"avel") && suffix(R2, L"avel")) {
      CT = removeSuffix(CT, L"avel");
      return true;
    }
    if (suffix(CT, L"ivel") && suffix(R2, L"ivel")) {
      CT = removeSuffix(CT, L"ivel");
      return true;
    }
    if (suffix(CT, L"ista") && suffix(R2, L"ista")) {
      CT = removeSuffix(CT, L"ista");
      return true;
    }
    if (suffix(CT, L"osos") && suffix(R2, L"osos")) {
      CT = removeSuffix(CT, L"osos");
      return true;
    }
    if (suffix(CT, L"osas") && suffix(R2, L"osas")) {
      CT = removeSuffix(CT, L"osas");
      return true;
    }
    if (suffix(CT, L"ador") && suffix(R2, L"ador")) {
      CT = removeSuffix(CT, L"ador");
      return true;
    }
    if (suffix(CT, L"ivas") && suffix(R2, L"ivas")) {
      CT = removeSuffix(CT, L"ivas");
      return true;
    }
    if (suffix(CT, L"ivos") && suffix(R2, L"ivos")) {
      CT = removeSuffix(CT, L"ivos");
      return true;
    }
    if (suffix(CT, L"iras") && suffix(RV, L"iras") &&
        suffixPreceded(CT, L"iras", L"e")) {
      CT = replaceSuffix(CT, L"iras", L"ir");
      return true;
    }
  }

  // suffix length = 3
  if (CT.length() >= 3) {
    if (suffix(CT, L"eza") && suffix(R2, L"eza")) {
      CT = removeSuffix(CT, L"eza");
      return true;
    }
    if (suffix(CT, L"ico") && suffix(R2, L"ico")) {
      CT = removeSuffix(CT, L"ico");
      return true;
    }
    if (suffix(CT, L"ica") && suffix(R2, L"ica")) {
      CT = removeSuffix(CT, L"ica");
      return true;
    }
    if (suffix(CT, L"oso") && suffix(R2, L"oso")) {
      CT = removeSuffix(CT, L"oso");
      return true;
    }
    if (suffix(CT, L"osa") && suffix(R2, L"osa")) {
      CT = removeSuffix(CT, L"osa");
      return true;
    }
    if (suffix(CT, L"iva") && suffix(R2, L"iva")) {
      CT = removeSuffix(CT, L"iva");
      return true;
    }
    if (suffix(CT, L"ivo") && suffix(R2, L"ivo")) {
      CT = removeSuffix(CT, L"ivo");
      return true;
    }
    if (suffix(CT, L"ira") && suffix(RV, L"ira") &&
        suffixPreceded(CT, L"ira", L"e")) {
      CT = replaceSuffix(CT, L"ira", L"ir");
      return true;
    }
  }

  // no ending was removed by step1
  return false;
}

bool BrazilianStemmer::step2()
{
  if (RV == L"") {
    return false;
  }

  // suffix lenght = 7
  if (RV.length() >= 7) {
    if (suffix(RV, L"issemos")) {
      CT = removeSuffix(CT, L"issemos");
      return true;
    }
    if (suffix(RV, L"essemos")) {
      CT = removeSuffix(CT, L"essemos");
      return true;
    }
    if (suffix(RV, L"assemos")) {
      CT = removeSuffix(CT, L"assemos");
      return true;
    }
    if (suffix(RV, L"ariamos")) {
      CT = removeSuffix(CT, L"ariamos");
      return true;
    }
    if (suffix(RV, L"eriamos")) {
      CT = removeSuffix(CT, L"eriamos");
      return true;
    }
    if (suffix(RV, L"iriamos")) {
      CT = removeSuffix(CT, L"iriamos");
      return true;
    }
  }

  // suffix length = 6
  if (RV.length() >= 6) {
    if (suffix(RV, L"iremos")) {
      CT = removeSuffix(CT, L"iremos");
      return true;
    }
    if (suffix(RV, L"eremos")) {
      CT = removeSuffix(CT, L"eremos");
      return true;
    }
    if (suffix(RV, L"aremos")) {
      CT = removeSuffix(CT, L"aremos");
      return true;
    }
    if (suffix(RV, L"avamos")) {
      CT = removeSuffix(CT, L"avamos");
      return true;
    }
    if (suffix(RV, L"iramos")) {
      CT = removeSuffix(CT, L"iramos");
      return true;
    }
    if (suffix(RV, L"eramos")) {
      CT = removeSuffix(CT, L"eramos");
      return true;
    }
    if (suffix(RV, L"aramos")) {
      CT = removeSuffix(CT, L"aramos");
      return true;
    }
    if (suffix(RV, L"asseis")) {
      CT = removeSuffix(CT, L"asseis");
      return true;
    }
    if (suffix(RV, L"esseis")) {
      CT = removeSuffix(CT, L"esseis");
      return true;
    }
    if (suffix(RV, L"isseis")) {
      CT = removeSuffix(CT, L"isseis");
      return true;
    }
    if (suffix(RV, L"arieis")) {
      CT = removeSuffix(CT, L"arieis");
      return true;
    }
    if (suffix(RV, L"erieis")) {
      CT = removeSuffix(CT, L"erieis");
      return true;
    }
    if (suffix(RV, L"irieis")) {
      CT = removeSuffix(CT, L"irieis");
      return true;
    }
  }

  // suffix length = 5
  if (RV.length() >= 5) {
    if (suffix(RV, L"irmos")) {
      CT = removeSuffix(CT, L"irmos");
      return true;
    }
    if (suffix(RV, L"iamos")) {
      CT = removeSuffix(CT, L"iamos");
      return true;
    }
    if (suffix(RV, L"armos")) {
      CT = removeSuffix(CT, L"armos");
      return true;
    }
    if (suffix(RV, L"ermos")) {
      CT = removeSuffix(CT, L"ermos");
      return true;
    }
    if (suffix(RV, L"areis")) {
      CT = removeSuffix(CT, L"areis");
      return true;
    }
    if (suffix(RV, L"ereis")) {
      CT = removeSuffix(CT, L"ereis");
      return true;
    }
    if (suffix(RV, L"ireis")) {
      CT = removeSuffix(CT, L"ireis");
      return true;
    }
    if (suffix(RV, L"asses")) {
      CT = removeSuffix(CT, L"asses");
      return true;
    }
    if (suffix(RV, L"esses")) {
      CT = removeSuffix(CT, L"esses");
      return true;
    }
    if (suffix(RV, L"isses")) {
      CT = removeSuffix(CT, L"isses");
      return true;
    }
    if (suffix(RV, L"astes")) {
      CT = removeSuffix(CT, L"astes");
      return true;
    }
    if (suffix(RV, L"assem")) {
      CT = removeSuffix(CT, L"assem");
      return true;
    }
    if (suffix(RV, L"essem")) {
      CT = removeSuffix(CT, L"essem");
      return true;
    }
    if (suffix(RV, L"issem")) {
      CT = removeSuffix(CT, L"issem");
      return true;
    }
    if (suffix(RV, L"ardes")) {
      CT = removeSuffix(CT, L"ardes");
      return true;
    }
    if (suffix(RV, L"erdes")) {
      CT = removeSuffix(CT, L"erdes");
      return true;
    }
    if (suffix(RV, L"irdes")) {
      CT = removeSuffix(CT, L"irdes");
      return true;
    }
    if (suffix(RV, L"ariam")) {
      CT = removeSuffix(CT, L"ariam");
      return true;
    }
    if (suffix(RV, L"eriam")) {
      CT = removeSuffix(CT, L"eriam");
      return true;
    }
    if (suffix(RV, L"iriam")) {
      CT = removeSuffix(CT, L"iriam");
      return true;
    }
    if (suffix(RV, L"arias")) {
      CT = removeSuffix(CT, L"arias");
      return true;
    }
    if (suffix(RV, L"erias")) {
      CT = removeSuffix(CT, L"erias");
      return true;
    }
    if (suffix(RV, L"irias")) {
      CT = removeSuffix(CT, L"irias");
      return true;
    }
    if (suffix(RV, L"estes")) {
      CT = removeSuffix(CT, L"estes");
      return true;
    }
    if (suffix(RV, L"istes")) {
      CT = removeSuffix(CT, L"istes");
      return true;
    }
    if (suffix(RV, L"areis")) {
      CT = removeSuffix(CT, L"areis");
      return true;
    }
    if (suffix(RV, L"aveis")) {
      CT = removeSuffix(CT, L"aveis");
      return true;
    }
  }

  // suffix length = 4
  if (RV.length() >= 4) {
    if (suffix(RV, L"aria")) {
      CT = removeSuffix(CT, L"aria");
      return true;
    }
    if (suffix(RV, L"eria")) {
      CT = removeSuffix(CT, L"eria");
      return true;
    }
    if (suffix(RV, L"iria")) {
      CT = removeSuffix(CT, L"iria");
      return true;
    }
    if (suffix(RV, L"asse")) {
      CT = removeSuffix(CT, L"asse");
      return true;
    }
    if (suffix(RV, L"esse")) {
      CT = removeSuffix(CT, L"esse");
      return true;
    }
    if (suffix(RV, L"isse")) {
      CT = removeSuffix(CT, L"isse");
      return true;
    }
    if (suffix(RV, L"aste")) {
      CT = removeSuffix(CT, L"aste");
      return true;
    }
    if (suffix(RV, L"este")) {
      CT = removeSuffix(CT, L"este");
      return true;
    }
    if (suffix(RV, L"iste")) {
      CT = removeSuffix(CT, L"iste");
      return true;
    }
    if (suffix(RV, L"arei")) {
      CT = removeSuffix(CT, L"arei");
      return true;
    }
    if (suffix(RV, L"erei")) {
      CT = removeSuffix(CT, L"erei");
      return true;
    }
    if (suffix(RV, L"irei")) {
      CT = removeSuffix(CT, L"irei");
      return true;
    }
    if (suffix(RV, L"aram")) {
      CT = removeSuffix(CT, L"aram");
      return true;
    }
    if (suffix(RV, L"eram")) {
      CT = removeSuffix(CT, L"eram");
      return true;
    }
    if (suffix(RV, L"iram")) {
      CT = removeSuffix(CT, L"iram");
      return true;
    }
    if (suffix(RV, L"avam")) {
      CT = removeSuffix(CT, L"avam");
      return true;
    }
    if (suffix(RV, L"arem")) {
      CT = removeSuffix(CT, L"arem");
      return true;
    }
    if (suffix(RV, L"erem")) {
      CT = removeSuffix(CT, L"erem");
      return true;
    }
    if (suffix(RV, L"irem")) {
      CT = removeSuffix(CT, L"irem");
      return true;
    }
    if (suffix(RV, L"ando")) {
      CT = removeSuffix(CT, L"ando");
      return true;
    }
    if (suffix(RV, L"endo")) {
      CT = removeSuffix(CT, L"endo");
      return true;
    }
    if (suffix(RV, L"indo")) {
      CT = removeSuffix(CT, L"indo");
      return true;
    }
    if (suffix(RV, L"arao")) {
      CT = removeSuffix(CT, L"arao");
      return true;
    }
    if (suffix(RV, L"erao")) {
      CT = removeSuffix(CT, L"erao");
      return true;
    }
    if (suffix(RV, L"irao")) {
      CT = removeSuffix(CT, L"irao");
      return true;
    }
    if (suffix(RV, L"adas")) {
      CT = removeSuffix(CT, L"adas");
      return true;
    }
    if (suffix(RV, L"idas")) {
      CT = removeSuffix(CT, L"idas");
      return true;
    }
    if (suffix(RV, L"aras")) {
      CT = removeSuffix(CT, L"aras");
      return true;
    }
    if (suffix(RV, L"eras")) {
      CT = removeSuffix(CT, L"eras");
      return true;
    }
    if (suffix(RV, L"iras")) {
      CT = removeSuffix(CT, L"iras");
      return true;
    }
    if (suffix(RV, L"avas")) {
      CT = removeSuffix(CT, L"avas");
      return true;
    }
    if (suffix(RV, L"ares")) {
      CT = removeSuffix(CT, L"ares");
      return true;
    }
    if (suffix(RV, L"eres")) {
      CT = removeSuffix(CT, L"eres");
      return true;
    }
    if (suffix(RV, L"ires")) {
      CT = removeSuffix(CT, L"ires");
      return true;
    }
    if (suffix(RV, L"ados")) {
      CT = removeSuffix(CT, L"ados");
      return true;
    }
    if (suffix(RV, L"idos")) {
      CT = removeSuffix(CT, L"idos");
      return true;
    }
    if (suffix(RV, L"amos")) {
      CT = removeSuffix(CT, L"amos");
      return true;
    }
    if (suffix(RV, L"emos")) {
      CT = removeSuffix(CT, L"emos");
      return true;
    }
    if (suffix(RV, L"imos")) {
      CT = removeSuffix(CT, L"imos");
      return true;
    }
    if (suffix(RV, L"iras")) {
      CT = removeSuffix(CT, L"iras");
      return true;
    }
    if (suffix(RV, L"ieis")) {
      CT = removeSuffix(CT, L"ieis");
      return true;
    }
  }

  // suffix length = 3
  if (RV.length() >= 3) {
    if (suffix(RV, L"ada")) {
      CT = removeSuffix(CT, L"ada");
      return true;
    }
    if (suffix(RV, L"ida")) {
      CT = removeSuffix(CT, L"ida");
      return true;
    }
    if (suffix(RV, L"ara")) {
      CT = removeSuffix(CT, L"ara");
      return true;
    }
    if (suffix(RV, L"era")) {
      CT = removeSuffix(CT, L"era");
      return true;
    }
    if (suffix(RV, L"ira")) {
      CT = removeSuffix(CT, L"ava");
      return true;
    }
    if (suffix(RV, L"iam")) {
      CT = removeSuffix(CT, L"iam");
      return true;
    }
    if (suffix(RV, L"ado")) {
      CT = removeSuffix(CT, L"ado");
      return true;
    }
    if (suffix(RV, L"ido")) {
      CT = removeSuffix(CT, L"ido");
      return true;
    }
    if (suffix(RV, L"ias")) {
      CT = removeSuffix(CT, L"ias");
      return true;
    }
    if (suffix(RV, L"ais")) {
      CT = removeSuffix(CT, L"ais");
      return true;
    }
    if (suffix(RV, L"eis")) {
      CT = removeSuffix(CT, L"eis");
      return true;
    }
    if (suffix(RV, L"ira")) {
      CT = removeSuffix(CT, L"ira");
      return true;
    }
    if (suffix(RV, L"ear")) {
      CT = removeSuffix(CT, L"ear");
      return true;
    }
  }

  // suffix length = 2
  if (RV.length() >= 2) {
    if (suffix(RV, L"ia")) {
      CT = removeSuffix(CT, L"ia");
      return true;
    }
    if (suffix(RV, L"ei")) {
      CT = removeSuffix(CT, L"ei");
      return true;
    }
    if (suffix(RV, L"am")) {
      CT = removeSuffix(CT, L"am");
      return true;
    }
    if (suffix(RV, L"em")) {
      CT = removeSuffix(CT, L"em");
      return true;
    }
    if (suffix(RV, L"ar")) {
      CT = removeSuffix(CT, L"ar");
      return true;
    }
    if (suffix(RV, L"er")) {
      CT = removeSuffix(CT, L"er");
      return true;
    }
    if (suffix(RV, L"ir")) {
      CT = removeSuffix(CT, L"ir");
      return true;
    }
    if (suffix(RV, L"as")) {
      CT = removeSuffix(CT, L"as");
      return true;
    }
    if (suffix(RV, L"es")) {
      CT = removeSuffix(CT, L"es");
      return true;
    }
    if (suffix(RV, L"is")) {
      CT = removeSuffix(CT, L"is");
      return true;
    }
    if (suffix(RV, L"eu")) {
      CT = removeSuffix(CT, L"eu");
      return true;
    }
    if (suffix(RV, L"iu")) {
      CT = removeSuffix(CT, L"iu");
      return true;
    }
    if (suffix(RV, L"iu")) {
      CT = removeSuffix(CT, L"iu");
      return true;
    }
    if (suffix(RV, L"ou")) {
      CT = removeSuffix(CT, L"ou");
      return true;
    }
  }

  // no ending was removed by step2
  return false;
}

void BrazilianStemmer::step3()
{
  if (RV == L"") {
    return;
  }

  if (suffix(RV, L"i") && suffixPreceded(RV, L"i", L"c")) {
    CT = removeSuffix(CT, L"i");
  }
}

void BrazilianStemmer::step4()
{
  if (RV == L"") {
    return;
  }

  if (suffix(RV, L"os")) {
    CT = removeSuffix(CT, L"os");
    return;
  }
  if (suffix(RV, L"a")) {
    CT = removeSuffix(CT, L"a");
    return;
  }
  if (suffix(RV, L"i")) {
    CT = removeSuffix(CT, L"i");
    return;
  }
  if (suffix(RV, L"o")) {
    CT = removeSuffix(CT, L"o");
    return;
  }
}

void BrazilianStemmer::step5()
{
  if (RV == L"") {
    return;
  }

  if (suffix(RV, L"e")) {
    if (suffixPreceded(RV, L"e", L"gu")) {
      CT = removeSuffix(CT, L"e");
      CT = removeSuffix(CT, L"u");
      return;
    }

    if (suffixPreceded(RV, L"e", L"ci")) {
      CT = removeSuffix(CT, L"e");
      CT = removeSuffix(CT, L"i");
      return;
    }

    CT = removeSuffix(CT, L"e");
    return;
  }
}

wstring BrazilianStemmer::log()
{
  return L" (TERM = " + TERM + L")" + L" (CT = " + CT + L")" + L" (RV = " + RV +
         L")" + L" (R1 = " + R1 + L")" + L" (R2 = " + R2 + L")";
}
} // namespace org::apache::lucene::analysis::br