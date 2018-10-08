using namespace std;

#include "KStemmer.h"
#include "../util/OpenStringBuilder.h"
#include "KStemData1.h"
#include "KStemData2.h"
#include "KStemData3.h"
#include "KStemData4.h"
#include "KStemData5.h"
#include "KStemData6.h"
#include "KStemData7.h"
#include "KStemData8.h"

namespace org::apache::lucene::analysis::en
{
using CharArrayMap = org::apache::lucene::analysis::CharArrayMap;
using OpenStringBuilder =
    org::apache::lucene::analysis::util::OpenStringBuilder;
std::deque<wstring> const KStemmer::exceptionWords = {
    L"aide",   L"bathe", L"caste", L"cute",     L"dame",      L"dime",
    L"doge",   L"done",  L"dune",  L"envelope", L"gage",      L"grille",
    L"grippe", L"lobe",  L"mane",  L"mare",     L"nape",      L"node",
    L"pane",   L"pate",  L"plane", L"pope",     L"programme", L"quite",
    L"ripe",   L"rote",  L"rune",  L"sage",     L"severe",    L"shoppe",
    L"sine",   L"slime", L"snipe", L"steppe",   L"suite",     L"swinge",
    L"tare",   L"tine",  L"tope",  L"tripe",    L"twine"};
std::deque<std::deque<wstring>> const KStemmer::directConflations = {
    {L"aging", L"age"},
    {L"going", L"go"},
    {L"goes", L"go"},
    {L"lying", L"lie"},
    {L"using", L"use"},
    {L"owing", L"owe"},
    {L"suing", L"sue"},
    {L"dying", L"die"},
    {L"tying", L"tie"},
    {L"vying", L"vie"},
    {L"aged", L"age"},
    {L"used", L"use"},
    {L"vied", L"vie"},
    {L"cued", L"cue"},
    {L"died", L"die"},
    {L"eyed", L"eye"},
    {L"hued", L"hue"},
    {L"iced", L"ice"},
    {L"lied", L"lie"},
    {L"owed", L"owe"},
    {L"sued", L"sue"},
    {L"toed", L"toe"},
    {L"tied", L"tie"},
    {L"does", L"do"},
    {L"doing", L"do"},
    {L"aeronautical", L"aeronautics"},
    {L"mathematical", L"mathematics"},
    {L"political", L"politics"},
    {L"metaphysical", L"metaphysics"},
    {L"cylindrical", L"cylinder"},
    {L"nazism", L"nazi"},
    {L"ambiguity", L"ambiguous"},
    {L"barbarity", L"barbarous"},
    {L"credulity", L"credulous"},
    {L"generosity", L"generous"},
    {L"spontaneity", L"spontaneous"},
    {L"unanimity", L"unanimous"},
    {L"voracity", L"voracious"},
    {L"fled", L"flee"},
    {L"miscarriage", L"miscarry"}};
std::deque<std::deque<wstring>> const KStemmer::countryNationality = {
    {L"afghan", L"afghanistan"},
    {L"african", L"africa"},
    {L"albanian", L"albania"},
    {L"algerian", L"algeria"},
    {L"american", L"america"},
    {L"andorran", L"andorra"},
    {L"angolan", L"angola"},
    {L"arabian", L"arabia"},
    {L"argentine", L"argentina"},
    {L"armenian", L"armenia"},
    {L"asian", L"asia"},
    {L"australian", L"australia"},
    {L"austrian", L"austria"},
    {L"azerbaijani", L"azerbaijan"},
    {L"azeri", L"azerbaijan"},
    {L"bangladeshi", L"bangladesh"},
    {L"belgian", L"belgium"},
    {L"bermudan", L"bermuda"},
    {L"bolivian", L"bolivia"},
    {L"bosnian", L"bosnia"},
    {L"botswanan", L"botswana"},
    {L"brazilian", L"brazil"},
    {L"british", L"britain"},
    {L"bulgarian", L"bulgaria"},
    {L"burmese", L"burma"},
    {L"californian", L"california"},
    {L"cambodian", L"cambodia"},
    {L"canadian", L"canada"},
    {L"chadian", L"chad"},
    {L"chilean", L"chile"},
    {L"chinese", L"china"},
    {L"colombian", L"colombia"},
    {L"croat", L"croatia"},
    {L"croatian", L"croatia"},
    {L"cuban", L"cuba"},
    {L"cypriot", L"cyprus"},
    {L"czechoslovakian", L"czechoslovakia"},
    {L"danish", L"denmark"},
    {L"egyptian", L"egypt"},
    {L"equadorian", L"equador"},
    {L"eritrean", L"eritrea"},
    {L"estonian", L"estonia"},
    {L"ethiopian", L"ethiopia"},
    {L"european", L"europe"},
    {L"fijian", L"fiji"},
    {L"filipino", L"philippines"},
    {L"finnish", L"finland"},
    {L"french", L"france"},
    {L"gambian", L"gambia"},
    {L"georgian", L"georgia"},
    {L"german", L"germany"},
    {L"ghanian", L"ghana"},
    {L"greek", L"greece"},
    {L"grenadan", L"grenada"},
    {L"guamian", L"guam"},
    {L"guatemalan", L"guatemala"},
    {L"guinean", L"guinea"},
    {L"guyanan", L"guyana"},
    {L"haitian", L"haiti"},
    {L"hawaiian", L"hawaii"},
    {L"holland", L"dutch"},
    {L"honduran", L"honduras"},
    {L"hungarian", L"hungary"},
    {L"icelandic", L"iceland"},
    {L"indonesian", L"indonesia"},
    {L"iranian", L"iran"},
    {L"iraqi", L"iraq"},
    {L"iraqui", L"iraq"},
    {L"irish", L"ireland"},
    {L"israeli", L"israel"},
    {L"italian", L"italy"},
    {L"jamaican", L"jamaica"},
    {L"japanese", L"japan"},
    {L"jordanian", L"jordan"},
    {L"kampuchean", L"cambodia"},
    {L"kenyan", L"kenya"},
    {L"korean", L"korea"},
    {L"kuwaiti", L"kuwait"},
    {L"lankan", L"lanka"},
    {L"laotian", L"laos"},
    {L"latvian", L"latvia"},
    {L"lebanese", L"lebanon"},
    {L"liberian", L"liberia"},
    {L"libyan", L"libya"},
    {L"lithuanian", L"lithuania"},
    {L"macedonian", L"macedonia"},
    {L"madagascan", L"madagascar"},
    {L"malaysian", L"malaysia"},
    {L"maltese", L"malta"},
    {L"mauritanian", L"mauritania"},
    {L"mexican", L"mexico"},
    {L"micronesian", L"micronesia"},
    {L"moldovan", L"moldova"},
    {L"monacan", L"monaco"},
    {L"mongolian", L"mongolia"},
    {L"montenegran", L"montenegro"},
    {L"moroccan", L"morocco"},
    {L"myanmar", L"burma"},
    {L"namibian", L"namibia"},
    {L"nepalese", L"nepal"},
    {L"nicaraguan", L"nicaragua"},
    {L"nigerian", L"nigeria"},
    {L"norwegian", L"norway"},
    {L"omani", L"oman"},
    {L"pakistani", L"pakistan"},
    {L"panamanian", L"panama"},
    {L"papuan", L"papua"},
    {L"paraguayan", L"paraguay"},
    {L"peruvian", L"peru"},
    {L"portuguese", L"portugal"},
    {L"romanian", L"romania"},
    {L"rumania", L"romania"},
    {L"rumanian", L"romania"},
    {L"russian", L"russia"},
    {L"rwandan", L"rwanda"},
    {L"samoan", L"samoa"},
    {L"scottish", L"scotland"},
    {L"serb", L"serbia"},
    {L"serbian", L"serbia"},
    {L"siam", L"thailand"},
    {L"siamese", L"thailand"},
    {L"slovakia", L"slovak"},
    {L"slovakian", L"slovak"},
    {L"slovenian", L"slovenia"},
    {L"somali", L"somalia"},
    {L"somalian", L"somalia"},
    {L"spanish", L"spain"},
    {L"swedish", L"sweden"},
    {L"swiss", L"switzerland"},
    {L"syrian", L"syria"},
    {L"taiwanese", L"taiwan"},
    {L"tanzanian", L"tanzania"},
    {L"texan", L"texas"},
    {L"thai", L"thailand"},
    {L"tunisian", L"tunisia"},
    {L"turkish", L"turkey"},
    {L"ugandan", L"uganda"},
    {L"ukrainian", L"ukraine"},
    {L"uruguayan", L"uruguay"},
    {L"uzbek", L"uzbekistan"},
    {L"venezuelan", L"venezuela"},
    {L"vietnamese", L"viet"},
    {L"virginian", L"virginia"},
    {L"yemeni", L"yemen"},
    {L"yugoslav", L"yugoslavia"},
    {L"yugoslavian", L"yugoslavia"},
    {L"zambian", L"zambia"},
    {L"zealander", L"zealand"},
    {L"zimbabwean", L"zimbabwe"}};
std::deque<wstring> const KStemmer::supplementDict = {
    L"aids",          L"applicator",   L"capacitor",   L"digitize",
    L"electromagnet", L"ellipsoid",    L"exosphere",   L"extensible",
    L"ferromagnet",   L"graphics",     L"hydromagnet", L"polygraph",
    L"toroid",        L"superconduct", L"backscatter", L"connectionism"};
std::deque<wstring> const KStemmer::properNouns = {
    L"abrams",      L"achilles",     L"acropolis",    L"adams",
    L"agnes",       L"aires",        L"alexander",    L"alexis",
    L"alfred",      L"algiers",      L"alps",         L"amadeus",
    L"ames",        L"amos",         L"andes",        L"angeles",
    L"annapolis",   L"antilles",     L"aquarius",     L"archimedes",
    L"arkansas",    L"asher",        L"ashly",        L"athens",
    L"atkins",      L"atlantis",     L"avis",         L"bahamas",
    L"bangor",      L"barbados",     L"barger",       L"bering",
    L"brahms",      L"brandeis",     L"brussels",     L"bruxelles",
    L"cairns",      L"camoros",      L"camus",        L"carlos",
    L"celts",       L"chalker",      L"charles",      L"cheops",
    L"ching",       L"christmas",    L"cocos",        L"collins",
    L"columbus",    L"confucius",    L"conners",      L"connolly",
    L"copernicus",  L"cramer",       L"cyclops",      L"cygnus",
    L"cyprus",      L"dallas",       L"damascus",     L"daniels",
    L"davies",      L"davis",        L"decker",       L"denning",
    L"dennis",      L"descartes",    L"dickens",      L"doris",
    L"douglas",     L"downs",        L"dreyfus",      L"dukakis",
    L"dulles",      L"dumfries",     L"ecclesiastes", L"edwards",
    L"emily",       L"erasmus",      L"euphrates",    L"evans",
    L"everglades",  L"fairbanks",    L"federales",    L"fisher",
    L"fitzsimmons", L"fleming",      L"forbes",       L"fowler",
    L"france",      L"francis",      L"goering",      L"goodling",
    L"goths",       L"grenadines",   L"guiness",      L"hades",
    L"harding",     L"harris",       L"hastings",     L"hawkes",
    L"hawking",     L"hayes",        L"heights",      L"hercules",
    L"himalayas",   L"hippocrates",  L"hobbs",        L"holmes",
    L"honduras",    L"hopkins",      L"hughes",       L"humphreys",
    L"illinois",    L"indianapolis", L"inverness",    L"iris",
    L"iroquois",    L"irving",       L"isaacs",       L"italy",
    L"james",       L"jarvis",       L"jeffreys",     L"jesus",
    L"jones",       L"josephus",     L"judas",        L"julius",
    L"kansas",      L"keynes",       L"kipling",      L"kiwanis",
    L"lansing",     L"laos",         L"leeds",        L"levis",
    L"leviticus",   L"lewis",        L"louis",        L"maccabees",
    L"madras",      L"maimonides",   L"maldive",      L"massachusetts",
    L"matthews",    L"mauritius",    L"memphis",      L"mercedes",
    L"midas",       L"mingus",       L"minneapolis",  L"mohammed",
    L"moines",      L"morris",       L"moses",        L"myers",
    L"myknos",      L"nablus",       L"nanjing",      L"nantes",
    L"naples",      L"neal",         L"netherlands",  L"nevis",
    L"nostradamus", L"oedipus",      L"olympus",      L"orleans",
    L"orly",        L"papas",        L"paris",        L"parker",
    L"pauling",     L"peking",       L"pershing",     L"peter",
    L"peters",      L"philippines",  L"phineas",      L"pisces",
    L"pryor",       L"pythagoras",   L"queens",       L"rabelais",
    L"ramses",      L"reynolds",     L"rhesus",       L"rhodes",
    L"richards",    L"robins",       L"rodgers",      L"rogers",
    L"rubens",      L"sagittarius",  L"seychelles",   L"socrates",
    L"texas",       L"thames",       L"thomas",       L"tiberias",
    L"tunis",       L"venus",        L"vilnius",      L"wales",
    L"warner",      L"wilkins",      L"williams",     L"wyoming",
    L"xmas",        L"yonkers",      L"zeus",         L"frances",
    L"aarhus",      L"adonis",       L"andrews",      L"angus",
    L"antares",     L"aquinas",      L"arcturus",     L"ares",
    L"artemis",     L"augustus",     L"ayers",        L"barnabas",
    L"barnes",      L"becker",       L"bejing",       L"biggs",
    L"billings",    L"boeing",       L"boris",        L"borroughs",
    L"briggs",      L"buenos",       L"calais",       L"caracas",
    L"cassius",     L"cerberus",     L"ceres",        L"cervantes",
    L"chantilly",   L"chartres",     L"chester",      L"connally",
    L"conner",      L"coors",        L"cummings",     L"curtis",
    L"daedalus",    L"dionysus",     L"dobbs",        L"dolores",
    L"edmonds"};

KStemmer::DictEntry::DictEntry(const wstring &root, bool isException)
{
  this->root = root;
  this->exception = isException;
}

const shared_ptr<
    org::apache::lucene::analysis::CharArrayMap<std::shared_ptr<DictEntry>>>
    KStemmer::dict_ht = initializeDictHash();

wchar_t KStemmer::finalChar() { return word->charAt(k); }

wchar_t KStemmer::penultChar() { return word->charAt(k - 1); }

bool KStemmer::isVowel(int index) { return !isCons(index); }

bool KStemmer::isCons(int index)
{
  wchar_t ch;

  ch = word->charAt(index);

  if ((ch == L'a') || (ch == L'e') || (ch == L'i') || (ch == L'o') ||
      (ch == L'u')) {
    return false;
  }
  if ((ch != L'y') || (index == 0)) {
    return true;
  } else {
    return (!isCons(index - 1));
  }
}

shared_ptr<CharArrayMap<std::shared_ptr<DictEntry>>>
KStemmer::initializeDictHash()
{
  shared_ptr<DictEntry> defaultEntry;
  shared_ptr<DictEntry> entry;

  shared_ptr<CharArrayMap<std::shared_ptr<DictEntry>>> d =
      make_shared<CharArrayMap<std::shared_ptr<DictEntry>>>(1000, false);
  for (int i = 0; i < exceptionWords.size(); i++) {
    if (!d->containsKey(exceptionWords[i])) {
      entry = make_shared<DictEntry>(exceptionWords[i], true);
      d->put(exceptionWords[i], entry);
    } else {
      throw runtime_error(L"Warning: Entry [" + exceptionWords[i] +
                          L"] already in dictionary 1");
    }
  }

  for (int i = 0; i < directConflations.size(); i++) {
    if (!d->containsKey(directConflations[i][0])) {
      entry = make_shared<DictEntry>(directConflations[i][1], false);
      d->put(directConflations[i][0], entry);
    } else {
      throw runtime_error(L"Warning: Entry [" + directConflations[i][0] +
                          L"] already in dictionary 2");
    }
  }

  for (int i = 0; i < countryNationality.size(); i++) {
    if (!d->containsKey(countryNationality[i][0])) {
      entry = make_shared<DictEntry>(countryNationality[i][1], false);
      d->put(countryNationality[i][0], entry);
    } else {
      throw runtime_error(L"Warning: Entry [" + countryNationality[i][0] +
                          L"] already in dictionary 3");
    }
  }

  defaultEntry = make_shared<DictEntry>(nullptr, false);

  std::deque<wstring> array_;
  array_ = KStemData1::data;

  for (int i = 0; i < array_.size(); i++) {
    if (!d->containsKey(array_[i])) {
      d->put(array_[i], defaultEntry);
    } else {
      throw runtime_error(L"Warning: Entry [" + array_[i] +
                          L"] already in dictionary 4");
    }
  }

  array_ = KStemData2::data;
  for (int i = 0; i < array_.size(); i++) {
    if (!d->containsKey(array_[i])) {
      d->put(array_[i], defaultEntry);
    } else {
      throw runtime_error(L"Warning: Entry [" + array_[i] +
                          L"] already in dictionary 4");
    }
  }

  array_ = KStemData3::data;
  for (int i = 0; i < array_.size(); i++) {
    if (!d->containsKey(array_[i])) {
      d->put(array_[i], defaultEntry);
    } else {
      throw runtime_error(L"Warning: Entry [" + array_[i] +
                          L"] already in dictionary 4");
    }
  }

  array_ = KStemData4::data;
  for (int i = 0; i < array_.size(); i++) {
    if (!d->containsKey(array_[i])) {
      d->put(array_[i], defaultEntry);
    } else {
      throw runtime_error(L"Warning: Entry [" + array_[i] +
                          L"] already in dictionary 4");
    }
  }

  array_ = KStemData5::data;
  for (int i = 0; i < array_.size(); i++) {
    if (!d->containsKey(array_[i])) {
      d->put(array_[i], defaultEntry);
    } else {
      throw runtime_error(L"Warning: Entry [" + array_[i] +
                          L"] already in dictionary 4");
    }
  }

  array_ = KStemData6::data;
  for (int i = 0; i < array_.size(); i++) {
    if (!d->containsKey(array_[i])) {
      d->put(array_[i], defaultEntry);
    } else {
      throw runtime_error(L"Warning: Entry [" + array_[i] +
                          L"] already in dictionary 4");
    }
  }

  array_ = KStemData7::data;
  for (int i = 0; i < array_.size(); i++) {
    if (!d->containsKey(array_[i])) {
      d->put(array_[i], defaultEntry);
    } else {
      throw runtime_error(L"Warning: Entry [" + array_[i] +
                          L"] already in dictionary 4");
    }
  }

  for (int i = 0; i < KStemData8::data.size(); i++) {
    if (!d->containsKey(KStemData8::data[i])) {
      d->put(KStemData8::data[i], defaultEntry);
    } else {
      throw runtime_error(L"Warning: Entry [" + KStemData8::data[i] +
                          L"] already in dictionary 4");
    }
  }

  for (int i = 0; i < supplementDict.size(); i++) {
    if (!d->containsKey(supplementDict[i])) {
      d->put(supplementDict[i], defaultEntry);
    } else {
      throw runtime_error(L"Warning: Entry [" + supplementDict[i] +
                          L"] already in dictionary 5");
    }
  }

  for (int i = 0; i < properNouns.size(); i++) {
    if (!d->containsKey(properNouns[i])) {
      d->put(properNouns[i], defaultEntry);
    } else {
      throw runtime_error(L"Warning: Entry [" + properNouns[i] +
                          L"] already in dictionary 6");
    }
  }

  return d;
}

bool KStemmer::isAlpha(wchar_t ch)
{
  return ch >= L'a' && ch <= L'z'; // terms must be lowercased already
}

int KStemmer::stemLength() { return j + 1; };

bool KStemmer::endsIn(std::deque<wchar_t> &s)
{
  if (s.size() > k) {
    return false;
  }

  int r = word->length() - s.size(); // length of word before this suffix
  j = k;
  for (int r1 = r, i = 0; i < s.size(); i++, r1++) {
    if (s[i] != word->charAt(r1)) {
      return false;
    }
  }
  j = r - 1; // index of the character BEFORE the posfix
  return true;
}

bool KStemmer::endsIn(wchar_t a, wchar_t b)
{
  if (2 > k) {
    return false;
  }
  // check left to right since the endings have often already matched
  if (word->charAt(k - 1) == a && word->charAt(k) == b) {
    j = k - 2;
    return true;
  }
  return false;
}

bool KStemmer::endsIn(wchar_t a, wchar_t b, wchar_t c)
{
  if (3 > k) {
    return false;
  }
  if (word->charAt(k - 2) == a && word->charAt(k - 1) == b &&
      word->charAt(k) == c) {
    j = k - 3;
    return true;
  }
  return false;
}

bool KStemmer::endsIn(wchar_t a, wchar_t b, wchar_t c, wchar_t d)
{
  if (4 > k) {
    return false;
  }
  if (word->charAt(k - 3) == a && word->charAt(k - 2) == b &&
      word->charAt(k - 1) == c && word->charAt(k) == d) {
    j = k - 4;
    return true;
  }
  return false;
}

shared_ptr<DictEntry> KStemmer::wordInDict()
{
  /***
   * if (matchedEntry != null) { if (dict_ht.get(word.getArray(), 0,
   * word.size()) != matchedEntry) {
   * System.out.println("Uh oh... cached entry doesn't match"); } return
   * matchedEntry; }
   ***/
  if (matchedEntry != nullptr) {
    return matchedEntry;
  }
  shared_ptr<DictEntry> e = dict_ht->get(word->getArray(), 0, word->length());
  if (e != nullptr && !e->exception) {
    matchedEntry = e; // only cache if it's not an exception.
  }
  // lookups.add(word.toString());
  return e;
}

void KStemmer::plural()
{
  if (word->charAt(k) == L's') {
    if (endsIn(L'i', L'e', L's')) {
      word->setLength(j + 3);
      k--;
      if (lookup()) // ensure calories -> calorie
      {
        return;
      }
      k++;
      word->unsafeWrite(L's');
      setSuffix(L"y");
      lookup();
    } else if (endsIn(L'e', L's')) {
      /* try just removing the "s" */
      word->setLength(j + 2);
      k--;

      /*
       * note: don't check for exceptions here. So, `aides' -> `aide', but
       * `aided' -> `aid'. The exception for double s is used to prevent
       * crosses -> crosse. This is actually correct if crosses is a plural
       * noun (a type of racket used in lacrosse), but the verb is much more
       * common
       */

      /****
       * YCS: this was the one place where lookup was not followed by return.
       * So restructure it. if ((j>0)&&(lookup(word.toString())) &&
       * !((word.charAt(j) == 's') && (word.charAt(j-1) == 's'))) return;
       *****/
      bool tryE = j > 0 &&
                  !((word->charAt(j) == L's') && (word->charAt(j - 1) == L's'));
      if (tryE && lookup()) {
        return;
      }

      /* try removing the "es" */

      word->setLength(j + 1);
      k--;
      if (lookup()) {
        return;
      }

      /* the default is to retain the "e" */
      word->unsafeWrite(L'e');
      k++;

      if (!tryE) {
        lookup(); // if we didn't try the "e" ending before
      }
      return;
    } else {
      if (word->length() > 3 && penultChar() != L's' &&
          !endsIn(L'o', L'u', L's')) {
        /* unless the word ends in "ous" or a double "s", remove the final "s"
         */

        word->setLength(k);
        k--;
        lookup();
      }
    }
  }
}

void KStemmer::setSuffix(const wstring &s) { setSuff(s, s.length()); }

void KStemmer::setSuff(const wstring &s, int len)
{
  word->setLength(j + 1);
  for (int l = 0; l < len; l++) {
    word->unsafeWrite(s[l]);
  }
  k = j + len;
}

bool KStemmer::lookup()
{
  /******
   * debugging code std::wstring thisLookup = word.toString(); bool added =
   * lookups.add(thisLookup); if (!added) {
   * System.out.println("######extra lookup:" + thisLookup); // occaasional
   * extra lookups aren't necessarily errors... could happen by diff
   * manipulations // throw new RuntimeException("######extra lookup:" +
   * thisLookup); } else { // System.out.println("new lookup:" + thisLookup);
   * }
   ******/

  matchedEntry = dict_ht->get(word->getArray(), 0, word->size());
  return matchedEntry != nullptr;
}

void KStemmer::pastTense()
{
  /*
   * Handle words less than 5 letters with a direct mapping This prevents
   * (fled -> fl).
   */
  if (word->length() <= 4) {
    return;
  }

  if (endsIn(L'i', L'e', L'd')) {
    word->setLength(j + 3);
    k--;
    if (lookup()) // we almost always want to convert -ied to -y, but
    {
      return; // this isn't true for short words (died->die)
    }
    k++; // I don't know any long words that this applies to,
    word->unsafeWrite(L'd'); // but just in case...
    setSuffix(L"y");
    lookup();
    return;
  }

  /* the vowelInStem() is necessary so we don't stem acronyms */
  if (endsIn(L'e', L'd') && vowelInStem()) {
    /* see if the root ends in `e' */
    word->setLength(j + 2);
    k = j + 1;

    shared_ptr<DictEntry> entry = wordInDict();
    if (entry != nullptr) {
      if (!entry
               ->exception) /*
     {
                                                 * if it's in the dictionary and
                                                 * not an exception
                                                 */
        return;
    }
  }

  /* try removing the "ed" */
  word->setLength(j + 1);
  k = j;
  if (lookup()) {
    return;
  }

  /*
   * try removing a doubled consonant. if the root isn't found in the
   * dictionary, the default is to leave it doubled. This will correctly
   * capture `backfilled' -> `backfill' instead of `backfill' ->
   * `backfille', and seems correct most of the time
   */

  if (doubleC(k)) {
    word->setLength(k);
    k--;
    if (lookup()) {
      return;
    }
    word->unsafeWrite(word->charAt(k));
    k++;
    lookup();
    return;
  }

  /* if we have a `un-' prefix, then leave the word alone */
  /* (this will sometimes screw up with `under-', but we */
  /* will take care of that later) */

  if ((word->charAt(0) == L'u') && (word->charAt(1) == L'n')) {
    word->unsafeWrite(L'e');
    word->unsafeWrite(L'd');
    k = k + 2;
    // nolookup()
    return;
  }

  /*
   * it wasn't found by just removing the `d' or the `ed', so prefer to end
   * with an `e' (e.g., `microcoded' -> `microcode').
   */

  word->setLength(j + 1);
  word->unsafeWrite(L'e');
  k = j + 1;
  // nolookup() - we already tried the "e" ending
  return;
}
} // namespace org::apache::lucene::analysis::en

bool KStemmer::doubleC(int i)
{
  if (i < 1) {
    return false;
  }

  if (word->charAt(i) != word->charAt(i - 1)) {
    return false;
  }
  return (isCons(i));
}

bool KStemmer::vowelInStem()
{
  for (int i = 0; i < stemLength(); i++) {
    if (isVowel(i)) {
      return true;
    }
  }
  return false;
}

void KStemmer::aspect()
{
  /*
   * handle short words (aging -> age) via a direct mapping. This prevents
   * (thing -> the) in the version of this routine that ignores inflectional
   * variants that are mentioned in the dictionary (when the root is also
   * present)
   */

  if (word->length() <= 5) {
    return;
  }

  /* the vowelinstem() is necessary so we don't stem acronyms */
  if (endsIn(L'i', L'n', L'g') && vowelInStem()) {

    /* try adding an `e' to the stem and check against the dictionary */
    word->setCharAt(j + 1, L'e');
    word->setLength(j + 2);
    k = j + 1;

    shared_ptr<DictEntry> entry = wordInDict();
    if (entry != nullptr) {
      if (!entry->exception) // if it's in the dictionary and not an exception
      {
        return;
      }
    }

    /* adding on the `e' didn't work, so remove it */
    word->setLength(k);
    k--; // note that `ing' has also been removed

    if (lookup()) {
      return;
    }

    /* if I can remove a doubled consonant and get a word, then do so */
    if (doubleC(k)) {
      k--;
      word->setLength(k + 1);
      if (lookup()) {
        return;
      }
      word->unsafeWrite(word->charAt(k)); // restore the doubled consonant

      /* the default is to leave the consonant doubled */
      /* (e.g.,`fingerspelling' -> `fingerspell'). Unfortunately */
      /* `bookselling' -> `booksell' and `mislabelling' -> `mislabell'). */
      /* Without making the algorithm significantly more complicated, this */
      /* is the best I can do */
      k++;
      lookup();
      return;
    }

    /*
     * the word wasn't in the dictionary after removing the stem, and then
     * checking with and without a final `e'. The default is to add an `e'
     * unless the word ends in two consonants, so `microcoding' ->
     * `microcode'. The two consonants restriction wouldn't normally be
     * necessary, but is needed because we don't try to deal with prefixes and
     * compounds, and most of the time it is correct (e.g., footstamping ->
     * footstamp, not footstampe; however, decoupled -> decoupl). We can
     * prevent almost all of the incorrect stems if we try to do some prefix
     * analysis first
     */

    if ((j > 0) && isCons(j) && isCons(j - 1)) {
      k = j;
      word->setLength(k + 1);
      // nolookup() because we already did according to the comment
      return;
    }

    word->setLength(j + 1);
    word->unsafeWrite(L'e');
    k = j + 1;
    // nolookup(); we already tried an 'e' ending
    return;
  }
}

void KStemmer::ityEndings()
{
  int old_k = k;

  if (endsIn(L'i', L't', L'y')) {
    word->setLength(j + 1); // try just removing -ity
    k = j;
    if (lookup()) {
      return;
    }
    word->unsafeWrite(L'e'); // try removing -ity and adding -e
    k = j + 1;
    if (lookup()) {
      return;
    }
    word->setCharAt(j + 1, L'i');
    word->append(L"ty");
    k = old_k;
    /*
     * the -ability and -ibility endings are highly productive, so just accept
     * them
     */
    if ((j > 0) && (word->charAt(j - 1) == L'i') && (word->charAt(j) == L'l')) {
      word->setLength(j - 1);
      word->append(L"le"); // convert to -ble
      k = j;
      lookup();
      return;
    }

    /* ditto for -ivity */
    if ((j > 0) && (word->charAt(j - 1) == L'i') && (word->charAt(j) == L'v')) {
      word->setLength(j + 1);
      word->unsafeWrite(L'e'); // convert to -ive
      k = j + 1;
      lookup();
      return;
    }
    /* ditto for -ality */
    if ((j > 0) && (word->charAt(j - 1) == L'a') && (word->charAt(j) == L'l')) {
      word->setLength(j + 1);
      k = j;
      lookup();
      return;
    }

    /*
     * if the root isn't in the dictionary, and the variant *is* there, then
     * use the variant. This allows `immunity'->`immune', but prevents
     * `capacity'->`capac'. If neither the variant nor the root form are in
     * the dictionary, then remove the ending as a default
     */

    if (lookup()) {
      return;
    }

    /* the default is to remove -ity altogether */
    word->setLength(j + 1);
    k = j;
    // nolookup(), we already did it.
    return;
  }
}

void KStemmer::nceEndings()
{
  int old_k = k;
  wchar_t word_char;

  if (endsIn(L'n', L'c', L'e')) {
    word_char = word->charAt(j);
    if (!((word_char == L'e') || (word_char == L'a'))) {
      return;
    }
    word->setLength(j);
    word->unsafeWrite(L'e'); // try converting -e/ance to -e (adherance/adhere)
    k = j;
    if (lookup()) {
      return;
    }
    word->setLength(j); /*
                         * try removing -e/ance altogether
                         * (disappearance/disappear)
                         */
    k = j - 1;
    if (lookup()) {
      return;
    }
    word->unsafeWrite(word_char); // restore the original ending
    word->append(L"nce");
    k = old_k;
    // nolookup() because we restored the original ending
  }
  return;
}

void KStemmer::nessEndings()
{
  if (endsIn(L'n', L'e', L's', L's')) {
    /*
     * this is a very productive endings, so
     * just accept it
     */
    word->setLength(j + 1);
    k = j;
    if (word->charAt(j) == L'i') {
      word->setCharAt(j, L'y');
    }
    lookup();
  }
  return;
}

void KStemmer::ismEndings()
{
  if (endsIn(L'i', L's', L'm')) {
    /*
     * this is a very productive ending, so just
     * accept it
     */
    word->setLength(j + 1);
    k = j;
    lookup();
  }
  return;
}

void KStemmer::mentEndings()
{
  int old_k = k;

  if (endsIn(L'm', L'e', L'n', L't')) {
    word->setLength(j + 1);
    k = j;
    if (lookup()) {
      return;
    }
    word->append(L"ment");
    k = old_k;
    // nolookup
  }
  return;
}

void KStemmer::izeEndings()
{
  int old_k = k;

  if (endsIn(L'i', L'z', L'e')) {
    word->setLength(j + 1); // try removing -ize entirely
    k = j;
    if (lookup()) {
      return;
    }
    word->unsafeWrite(L'i');

    if (doubleC(j)) { // allow for a doubled consonant
      word->setLength(j);
      k = j - 1;
      if (lookup()) {
        return;
      }
      word->unsafeWrite(word->charAt(j - 1));
    }

    word->setLength(j + 1);
    word->unsafeWrite(L'e'); // try removing -ize and adding -e
    k = j + 1;
    if (lookup()) {
      return;
    }
    word->setLength(j + 1);
    word->append(L"ize");
    k = old_k;
    // nolookup()
  }
  return;
}

void KStemmer::ncyEndings()
{
  if (endsIn(L'n', L'c', L'y')) {
    if (!((word->charAt(j) == L'e') || (word->charAt(j) == L'a'))) {
      return;
    }
    word->setCharAt(j + 2, L't'); // try converting -ncy to -nt
    word->setLength(j + 3);
    k = j + 2;

    if (lookup()) {
      return;
    }

    word->setCharAt(j + 2, L'c'); // the default is to convert it to -nce
    word->unsafeWrite(L'e');
    k = j + 3;
    lookup();
  }
  return;
}

void KStemmer::bleEndings()
{
  int old_k = k;
  wchar_t word_char;

  if (endsIn(L'b', L'l', L'e')) {
    if (!((word->charAt(j) == L'a') || (word->charAt(j) == L'i'))) {
      return;
    }
    word_char = word->charAt(j);
    word->setLength(j); // try just removing the ending
    k = j - 1;
    if (lookup()) {
      return;
    }
    if (doubleC(k)) { // allow for a doubled consonant
      word->setLength(k);
      k--;
      if (lookup()) {
        return;
      }
      k++;
      word->unsafeWrite(word->charAt(k - 1));
    }
    word->setLength(j);
    word->unsafeWrite(L'e'); // try removing -a/ible and adding -e
    k = j;
    if (lookup()) {
      return;
    }
    word->setLength(j);
    word->append(L"ate"); // try removing -able and adding -ate
    /* (e.g., compensable/compensate) */
    k = j + 2;
    if (lookup()) {
      return;
    }
    word->setLength(j);
    word->unsafeWrite(word_char); // restore the original values
    word->append(L"ble");
    k = old_k;
    // nolookup()
  }
  return;
}

void KStemmer::icEndings()
{
  if (endsIn(L'i', L'c')) {
    word->setLength(j + 3);
    word->append(L"al"); // try converting -ic to -ical
    k = j + 4;
    if (lookup()) {
      return;
    }

    word->setCharAt(j + 1, L'y'); // try converting -ic to -y
    word->setLength(j + 2);
    k = j + 1;
    if (lookup()) {
      return;
    }

    word->setCharAt(j + 1, L'e'); // try converting -ic to -e
    if (lookup()) {
      return;
    }

    word->setLength(j + 1); // try removing -ic altogether
    k = j;
    if (lookup()) {
      return;
    }
    word->append(L"ic"); // restore the original ending
    k = j + 2;
    // nolookup()
  }
  return;
}

std::deque<wchar_t> KStemmer::ization = (wstring(L"ization")).toCharArray();
std::deque<wchar_t> KStemmer::ition = (wstring(L"ition")).toCharArray();
std::deque<wchar_t> KStemmer::ation = (wstring(L"ation")).toCharArray();
std::deque<wchar_t> KStemmer::ication = (wstring(L"ication")).toCharArray();

void KStemmer::ionEndings()
{
  int old_k = k;
  if (!endsIn(L'i', L'o', L'n')) {
    return;
  }

  if (endsIn(ization)) {
    /*
     * the -ize ending is very productive, so simply
     * accept it as the root
     */
    word->setLength(j + 3);
    word->unsafeWrite(L'e');
    k = j + 3;
    lookup();
    return;
  }

  if (endsIn(ition)) {
    word->setLength(j + 1);
    word->unsafeWrite(L'e');
    k = j + 1;
    if (lookup()) /*
    {
                         * remove -ition and add `e', and check against the
                         * dictionary
                         */
      return;     // (e.g., definition->define, opposition->oppose)
  }

  /* restore original values */
  word->setLength(j + 1);
  word->append(L"ition");
  k = old_k;
  // nolookup()
}
else if (endsIn(ation))
{
  word->setLength(j + 3);
  word->unsafeWrite(L'e');
  k = j + 3;
  if (lookup()) // remove -ion and add `e', and check against the dictionary
  {
    return; // (elmination -> eliminate)
  }

  word->setLength(j + 1);
  word->unsafeWrite(L'e'); /*
                            * remove -ation and add `e', and check against the
                            * dictionary
                            */
  k = j + 1;
  if (lookup()) {
    return;
  }

  word->setLength(j + 1); /*
                           * just remove -ation (resignation->resign) and
                           * check dictionary
                           */
  k = j;
  if (lookup()) {
    return;
  }

  /* restore original values */
  word->setLength(j + 1);
  word->append(L"ation");
  k = old_k;
  // nolookup()
}

/*
 * test -ication after -ation is attempted (e.g., `complication->complicate'
 * rather than `complication->comply')
 */

if (endsIn(ication)) {
  word->setLength(j + 1);
  word->unsafeWrite(L'y');
  k = j + 1;
  if (lookup()) /*
  {
                       * remove -ication and add `y', and check against the
                       * dictionary
                       */
    return;     // (e.g., amplification -> amplify)
}

/* restore original values */
word->setLength(j + 1);
word->append(L"ication");
k = old_k;
// nolookup()
}

// if (endsIn(ion)) {
if (true) {  // we checked for this earlier... just need to set "j"
  j = k - 3; // YCS

  word->setLength(j + 1);
  word->unsafeWrite(L'e');
  k = j + 1;
  if (lookup()) // remove -ion and add `e', and check against the dictionary
  {
    return;
  }

  word->setLength(j + 1);
  k = j;
  if (lookup()) // remove -ion, and if it's found, treat that as the root
  {
    return;
  }

  /* restore original values */
  word->setLength(j + 1);
  word->append(L"ion");
  k = old_k;
  // nolookup()
}

// nolookup(); all of the other paths restored original values
return;
}

void KStemmer::erAndOrEndings()
{
  int old_k = k;

  if (word->charAt(k) != L'r') {
    return; // YCS
  }

  wchar_t word_char; // so we can remember if it was -er or -or

  if (endsIn(L'i', L'z', L'e', L'r')) {
    /*
     * -ize is very productive, so accept it
     * as the root
     */
    word->setLength(j + 4);
    k = j + 3;
    lookup();
    return;
  }

  if (endsIn(L'e', L'r') || endsIn(L'o', L'r')) {
    word_char = word->charAt(j + 1);
    if (doubleC(j)) {
      word->setLength(j);
      k = j - 1;
      if (lookup()) {
        return;
      }
      word->unsafeWrite(word->charAt(j - 1)); // restore the doubled consonant
    }

    if (word->charAt(j) == L'i') { // do we have a -ier ending?
      word->setCharAt(j, L'y');
      word->setLength(j + 1);
      k = j;
      if (lookup()) // yes, so check against the dictionary
      {
        return;
      }
      word->setCharAt(j, L'i'); // restore the endings
      word->unsafeWrite(L'e');
    }

    if (word->charAt(j) == L'e') { // handle -eer
      word->setLength(j);
      k = j - 1;
      if (lookup()) {
        return;
      }
      word->unsafeWrite(L'e');
    }

    word->setLength(j + 2); // remove the -r ending
    k = j + 1;
    if (lookup()) {
      return;
    }
    word->setLength(j + 1); // try removing -er/-or
    k = j;
    if (lookup()) {
      return;
    }
    word->unsafeWrite(L'e'); // try removing -or and adding -e
    k = j + 1;
    if (lookup()) {
      return;
    }
    word->setLength(j + 1);
    word->unsafeWrite(word_char);
    word->unsafeWrite(L'r'); // restore the word to the way it was
    k = old_k;
    // nolookup()
  }
}

void KStemmer::lyEndings()
{
  int old_k = k;

  if (endsIn(L'l', L'y')) {

    word->setCharAt(j + 2, L'e'); // try converting -ly to -le

    if (lookup()) {
      return;
    }
    word->setCharAt(j + 2, L'y');

    word->setLength(j + 1); // try just removing the -ly
    k = j;

    if (lookup()) {
      return;
    }

    if ((j > 0) && (word->charAt(j - 1) == L'a') &&
        (word->charAt(j) == L'l')) /*
{
                                      * always
                                      * convert
                                      * -
                                      * ally
                                      * to
                                      * -
                                      * al
                                      */
      return;
  }
  word->append(L"ly");
  k = old_k;

  if ((j > 0) && (word->charAt(j - 1) == L'a') && (word->charAt(j) == L'b')) {
    /*
     * always
     * convert
     * -
     * ably
     * to
     * -
     * able
     */
    word->setCharAt(j + 2, L'e');
    k = j + 2;
    return;
  }

  if (word->charAt(j) == L'i') { // e.g., militarily -> military
    word->setLength(j);
    word->unsafeWrite(L'y');
    k = j;
    if (lookup()) {
      return;
    }
    word->setLength(j);
    word->append(L"ily");
    k = old_k;
  }

  word->setLength(j + 1); // the default is to remove -ly

  k = j;
  // nolookup()... we already tried removing the "ly" variant
}
return;
}

void KStemmer::alEndings()
{
  int old_k = k;

  if (word->length() < 4) {
    return;
  }
  if (endsIn(L'a', L'l')) {
    word->setLength(j + 1);
    k = j;
    if (lookup()) // try just removing the -al
    {
      return;
    }

    if (doubleC(j)) { // allow for a doubled consonant
      word->setLength(j);
      k = j - 1;
      if (lookup()) {
        return;
      }
      word->unsafeWrite(word->charAt(j - 1));
    }

    word->setLength(j + 1);
    word->unsafeWrite(L'e'); // try removing the -al and adding -e
    k = j + 1;
    if (lookup()) {
      return;
    }

    word->setLength(j + 1);
    word->append(L"um"); // try converting -al to -um
    /* (e.g., optimal - > optimum ) */
    k = j + 2;
    if (lookup()) {
      return;
    }

    word->setLength(j + 1);
    word->append(L"al"); // restore the ending to the way it was
    k = old_k;

    if ((j > 0) && (word->charAt(j - 1) == L'i') && (word->charAt(j) == L'c')) {
      word->setLength(j - 1); // try removing -ical
      k = j - 2;
      if (lookup()) {
        return;
      }

      word->setLength(j - 1);
      word->unsafeWrite(
          L'y'); // try turning -ical to -y (e.g., bibliographical)
      k = j - 1;
      if (lookup()) {
        return;
      }

      word->setLength(j - 1);
      word->append(L"ic"); // the default is to convert -ical to -ic
      k = j;
      // nolookup() ... converting ical to ic means removing "al" which we
      // already tried
      // ERROR
      lookup();
      return;
    }

    if (word->charAt(j) == L'i') { // sometimes -ial endings should be removed
      word->setLength(j);          // (sometimes it gets turned into -y, but we
      k = j - 1;                   // aren't dealing with that case for now)
      if (lookup()) {
        return;
      }
      word->append(L"ial");
      k = old_k;
      lookup();
    }
  }
  return;
}

void KStemmer::iveEndings()
{
  int old_k = k;

  if (endsIn(L'i', L'v', L'e')) {
    word->setLength(j + 1); // try removing -ive entirely
    k = j;
    if (lookup()) {
      return;
    }

    word->unsafeWrite(L'e'); // try removing -ive and adding -e
    k = j + 1;
    if (lookup()) {
      return;
    }
    word->setLength(j + 1);
    word->append(L"ive");
    if ((j > 0) && (word->charAt(j - 1) == L'a') && (word->charAt(j) == L't')) {
      word->setCharAt(j - 1, L'e'); // try removing -ative and adding -e
      word->setLength(j);           // (e.g., determinative -> determine)
      k = j - 1;
      if (lookup()) {
        return;
      }
      word->setLength(j - 1); // try just removing -ative
      if (lookup()) {
        return;
      }

      word->append(L"ative");
      k = old_k;
    }

    /* try mapping -ive to -ion (e.g., injunctive/injunction) */
    word->setCharAt(j + 2, L'o');
    word->setCharAt(j + 3, L'n');
    if (lookup()) {
      return;
    }

    word->setCharAt(j + 2, L'v'); // restore the original values
    word->setCharAt(j + 3, L'e');
    k = old_k;
    // nolookup()
  }
  return;
}

KStemmer::KStemmer() {}

wstring KStemmer::stem(const wstring &term)
{
  bool changed = stem(term.toCharArray(), term.length());
  if (!changed) {
    return term;
  }
  return asString();
}

wstring KStemmer::asString()
{
  wstring s = getString();
  if (s != L"") {
    return s;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return word->toString();
}

shared_ptr<std::wstring> KStemmer::asCharSequence()
{
  return result != L"" ? result : word;
}

wstring KStemmer::getString() { return result; }

std::deque<wchar_t> KStemmer::getChars() { return word->getArray(); }

int KStemmer::getLength() { return word->length(); }

bool KStemmer::matched()
{
  /***
   * if (!lookups.contains(word.toString())) { throw new
   * RuntimeException("didn't look up "+word.toString()+" prev="+prevLookup);
   * }
   ***/
  // lookup();
  return matchedEntry != nullptr;
}

bool KStemmer::stem(std::deque<wchar_t> &term, int len)
{

  result = L"";

  k = len - 1;
  if ((k <= 1) || (k >= MaxWordLen - 1)) {
    return false; // don't stem
  }

  // first check the stemmer dictionaries, and avoid using the
  // cache if it's in there.
  shared_ptr<DictEntry> entry = dict_ht->get(term, 0, len);
  if (entry != nullptr) {
    if (entry->root != L"") {
      result = entry->root;
      return true;
    }
    return false;
  }

  /***
   * caching off is normally faster if (cache == null) initializeStemHash();
   *
   * // now check the cache, before we copy chars to "word" if (cache != null)
   * { std::wstring val = cache.get(term, 0, len); if (val != null) { if (val !=
   * SAME) { result = val; return true; } return false; } }
   ***/

  word->reset();
  // allocate enough space so that an expansion is never needed
  word->reserve(len + 10);
  for (int i = 0; i < len; i++) {
    wchar_t ch = term[i];
    if (!isAlpha(ch)) {
      return false; // don't stem
    }
    // don't lowercase... it's a requirement that lowercase filter be
    // used before this stemmer.
    word->unsafeWrite(ch);
  }

  matchedEntry.reset();
  /***
   * lookups.clear(); lookups.add(word.toString());
   ***/

  /*
   * This while loop will never be executed more than one time; it is here
   * only to allow the break statement to be used to escape as soon as a word
   * is recognized
   */
  while (true) {
    // YCS: extra lookup()s were inserted so we don't need to
    // do an extra wordInDict() here.
    plural();
    if (matched()) {
      break;
    }
    pastTense();
    if (matched()) {
      break;
    }
    aspect();
    if (matched()) {
      break;
    }
    ityEndings();
    if (matched()) {
      break;
    }
    nessEndings();
    if (matched()) {
      break;
    }
    ionEndings();
    if (matched()) {
      break;
    }
    erAndOrEndings();
    if (matched()) {
      break;
    }
    lyEndings();
    if (matched()) {
      break;
    }
    alEndings();
    if (matched()) {
      break;
    }
    entry = wordInDict();
    iveEndings();
    if (matched()) {
      break;
    }
    izeEndings();
    if (matched()) {
      break;
    }
    mentEndings();
    if (matched()) {
      break;
    }
    bleEndings();
    if (matched()) {
      break;
    }
    ismEndings();
    if (matched()) {
      break;
    }
    icEndings();
    if (matched()) {
      break;
    }
    ncyEndings();
    if (matched()) {
      break;
    }
    nceEndings();
    matched();
    break;
  }

  /*
   * try for a direct mapping (allows for cases like `Italian'->`Italy' and
   * `Italians'->`Italy')
   */
  entry = matchedEntry;
  if (entry != nullptr) {
    result = entry->root; // may be null, which means that "word" is the stem
  }

  /***
   * caching off is normally faster if (cache != null && cache.size() <
   * maxCacheSize) { char[] key = new char[len]; System.arraycopy(term, 0,
   * key, 0, len); if (result != null) { cache.put(key, result); } else {
   * cache.put(key, word.toString()); } }
   ***/

  /***
   * if (entry == null) { if (!word.toString().equals(new std::wstring(term,0,len)))
   * { System.out.println("CASE:" + word.toString() + "," + new
   * std::wstring(term,0,len));
   *
   * } }
   ***/

  // no entry matched means result is "word"
  return true;
}
}