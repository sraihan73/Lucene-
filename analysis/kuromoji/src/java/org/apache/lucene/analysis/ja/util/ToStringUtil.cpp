using namespace std;

#include "ToStringUtil.h"

namespace org::apache::lucene::analysis::ja::util
{

const unordered_map<wstring, wstring> ToStringUtil::posTranslations =
    unordered_map<wstring, wstring>();

ToStringUtil::StaticConstructor::StaticConstructor()
{
  posTranslations.emplace(L"名詞", L"noun");
  posTranslations.emplace(L"名詞-一般", L"noun-common");
  posTranslations.emplace(L"名詞-固有名詞", L"noun-proper");
  posTranslations.emplace(L"名詞-固有名詞-一般", L"noun-proper-misc");
  posTranslations.emplace(L"名詞-固有名詞-人名", L"noun-proper-person");
  posTranslations.emplace(L"名詞-固有名詞-人名-一般",
                          L"noun-proper-person-misc");
  posTranslations.emplace(L"名詞-固有名詞-人名-姓",
                          L"noun-proper-person-surname");
  posTranslations.emplace(L"名詞-固有名詞-人名-名",
                          L"noun-proper-person-given_name");
  posTranslations.emplace(L"名詞-固有名詞-組織", L"noun-proper-organization");
  posTranslations.emplace(L"名詞-固有名詞-地域", L"noun-proper-place");
  posTranslations.emplace(L"名詞-固有名詞-地域-一般",
                          L"noun-proper-place-misc");
  posTranslations.emplace(L"名詞-固有名詞-地域-国",
                          L"noun-proper-place-country");
  posTranslations.emplace(L"名詞-代名詞", L"noun-pronoun");
  posTranslations.emplace(L"名詞-代名詞-一般", L"noun-pronoun-misc");
  posTranslations.emplace(L"名詞-代名詞-縮約", L"noun-pronoun-contraction");
  posTranslations.emplace(L"名詞-副詞可能", L"noun-adverbial");
  posTranslations.emplace(L"名詞-サ変接続", L"noun-verbal");
  posTranslations.emplace(L"名詞-形容動詞語幹", L"noun-adjective-base");
  posTranslations.emplace(L"名詞-数", L"noun-numeric");
  posTranslations.emplace(L"名詞-非自立", L"noun-affix");
  posTranslations.emplace(L"名詞-非自立-一般", L"noun-affix-misc");
  posTranslations.emplace(L"名詞-非自立-副詞可能", L"noun-affix-adverbial");
  posTranslations.emplace(L"名詞-非自立-助動詞語幹", L"noun-affix-aux");
  posTranslations.emplace(L"名詞-非自立-形容動詞語幹",
                          L"noun-affix-adjective-base");
  posTranslations.emplace(L"名詞-特殊", L"noun-special");
  posTranslations.emplace(L"名詞-特殊-助動詞語幹", L"noun-special-aux");
  posTranslations.emplace(L"名詞-接尾", L"noun-suffix");
  posTranslations.emplace(L"名詞-接尾-一般", L"noun-suffix-misc");
  posTranslations.emplace(L"名詞-接尾-人名", L"noun-suffix-person");
  posTranslations.emplace(L"名詞-接尾-地域", L"noun-suffix-place");
  posTranslations.emplace(L"名詞-接尾-サ変接続", L"noun-suffix-verbal");
  posTranslations.emplace(L"名詞-接尾-助動詞語幹", L"noun-suffix-aux");
  posTranslations.emplace(L"名詞-接尾-形容動詞語幹",
                          L"noun-suffix-adjective-base");
  posTranslations.emplace(L"名詞-接尾-副詞可能", L"noun-suffix-adverbial");
  posTranslations.emplace(L"名詞-接尾-助数詞", L"noun-suffix-classifier");
  posTranslations.emplace(L"名詞-接尾-特殊", L"noun-suffix-special");
  posTranslations.emplace(L"名詞-接続詞的", L"noun-suffix-conjunctive");
  posTranslations.emplace(L"名詞-動詞非自立的", L"noun-verbal_aux");
  posTranslations.emplace(L"名詞-引用文字列", L"noun-quotation");
  posTranslations.emplace(L"名詞-ナイ形容詞語幹", L"noun-nai_adjective");
  posTranslations.emplace(L"接頭詞", L"prefix");
  posTranslations.emplace(L"接頭詞-名詞接続", L"prefix-nominal");
  posTranslations.emplace(L"接頭詞-動詞接続", L"prefix-verbal");
  posTranslations.emplace(L"接頭詞-形容詞接続", L"prefix-adjectival");
  posTranslations.emplace(L"接頭詞-数接続", L"prefix-numerical");
  posTranslations.emplace(L"動詞", L"verb");
  posTranslations.emplace(L"動詞-自立", L"verb-main");
  posTranslations.emplace(L"動詞-非自立", L"verb-auxiliary");
  posTranslations.emplace(L"動詞-接尾", L"verb-suffix");
  posTranslations.emplace(L"形容詞", L"adjective");
  posTranslations.emplace(L"形容詞-自立", L"adjective-main");
  posTranslations.emplace(L"形容詞-非自立", L"adjective-auxiliary");
  posTranslations.emplace(L"形容詞-接尾", L"adjective-suffix");
  posTranslations.emplace(L"副詞", L"adverb");
  posTranslations.emplace(L"副詞-一般", L"adverb-misc");
  posTranslations.emplace(L"副詞-助詞類接続", L"adverb-particle_conjunction");
  posTranslations.emplace(L"連体詞", L"adnominal");
  posTranslations.emplace(L"接続詞", L"conjunction");
  posTranslations.emplace(L"助詞", L"particle");
  posTranslations.emplace(L"助詞-格助詞", L"particle-case");
  posTranslations.emplace(L"助詞-格助詞-一般", L"particle-case-misc");
  posTranslations.emplace(L"助詞-格助詞-引用", L"particle-case-quote");
  posTranslations.emplace(L"助詞-格助詞-連語", L"particle-case-compound");
  posTranslations.emplace(L"助詞-接続助詞", L"particle-conjunctive");
  posTranslations.emplace(L"助詞-係助詞", L"particle-dependency");
  posTranslations.emplace(L"助詞-副助詞", L"particle-adverbial");
  posTranslations.emplace(L"助詞-間投助詞", L"particle-interjective");
  posTranslations.emplace(L"助詞-並立助詞", L"particle-coordinate");
  posTranslations.emplace(L"助詞-終助詞", L"particle-final");
  posTranslations.emplace(L"助詞-副助詞／並立助詞／終助詞",
                          L"particle-adverbial/conjunctive/final");
  posTranslations.emplace(L"助詞-連体化", L"particle-adnominalizer");
  posTranslations.emplace(L"助詞-副詞化", L"particle-adnominalizer");
  posTranslations.emplace(L"助詞-特殊", L"particle-special");
  posTranslations.emplace(L"助動詞", L"auxiliary-verb");
  posTranslations.emplace(L"感動詞", L"interjection");
  posTranslations.emplace(L"記号", L"symbol");
  posTranslations.emplace(L"記号-一般", L"symbol-misc");
  posTranslations.emplace(L"記号-句点", L"symbol-period");
  posTranslations.emplace(L"記号-読点", L"symbol-comma");
  posTranslations.emplace(L"記号-空白", L"symbol-space");
  posTranslations.emplace(L"記号-括弧開", L"symbol-open_bracket");
  posTranslations.emplace(L"記号-括弧閉", L"symbol-close_bracket");
  posTranslations.emplace(L"記号-アルファベット", L"symbol-alphabetic");
  posTranslations.emplace(L"その他", L"other");
  posTranslations.emplace(L"その他-間投", L"other-interjection");
  posTranslations.emplace(L"フィラー", L"filler");
  posTranslations.emplace(L"非言語音", L"non-verbal");
  posTranslations.emplace(L"語断片", L"fragment");
  posTranslations.emplace(L"未知語", L"unknown");
  inflTypeTranslations.emplace(L"*", L"*");
  inflTypeTranslations.emplace(L"形容詞・アウオ段", L"adj-group-a-o-u");
  inflTypeTranslations.emplace(L"形容詞・イ段", L"adj-group-i");
  inflTypeTranslations.emplace(L"形容詞・イイ", L"adj-group-ii");
  inflTypeTranslations.emplace(L"不変化型", L"non-inflectional");
  inflTypeTranslations.emplace(L"特殊・タ", L"special-da");
  inflTypeTranslations.emplace(L"特殊・ダ", L"special-ta");
  inflTypeTranslations.emplace(L"文語・ゴトシ", L"classical-gotoshi");
  inflTypeTranslations.emplace(L"特殊・ジャ", L"special-ja");
  inflTypeTranslations.emplace(L"特殊・ナイ", L"special-nai");
  inflTypeTranslations.emplace(L"五段・ラ行特殊", L"5-row-cons-r-special");
  inflTypeTranslations.emplace(L"特殊・ヌ", L"special-nu");
  inflTypeTranslations.emplace(L"文語・キ", L"classical-ki");
  inflTypeTranslations.emplace(L"特殊・タイ", L"special-tai");
  inflTypeTranslations.emplace(L"文語・ベシ", L"classical-beshi");
  inflTypeTranslations.emplace(L"特殊・ヤ", L"special-ya");
  inflTypeTranslations.emplace(L"文語・マジ", L"classical-maji");
  inflTypeTranslations.emplace(L"下二・タ行", L"2-row-lower-cons-t");
  inflTypeTranslations.emplace(L"特殊・デス", L"special-desu");
  inflTypeTranslations.emplace(L"特殊・マス", L"special-masu");
  inflTypeTranslations.emplace(L"五段・ラ行アル", L"5-row-aru");
  inflTypeTranslations.emplace(L"文語・ナリ", L"classical-nari");
  inflTypeTranslations.emplace(L"文語・リ", L"classical-ri");
  inflTypeTranslations.emplace(L"文語・ケリ", L"classical-keri");
  inflTypeTranslations.emplace(L"文語・ル", L"classical-ru");
  inflTypeTranslations.emplace(L"五段・カ行イ音便", L"5-row-cons-k-i-onbin");
  inflTypeTranslations.emplace(L"五段・サ行", L"5-row-cons-s");
  inflTypeTranslations.emplace(L"一段", L"1-row");
  inflTypeTranslations.emplace(L"五段・ワ行促音便", L"5-row-cons-w-cons-onbin");
  inflTypeTranslations.emplace(L"五段・マ行", L"5-row-cons-m");
  inflTypeTranslations.emplace(L"五段・タ行", L"5-row-cons-t");
  inflTypeTranslations.emplace(L"五段・ラ行", L"5-row-cons-r");
  inflTypeTranslations.emplace(L"サ変・−スル", L"irregular-suffix-suru");
  inflTypeTranslations.emplace(L"五段・ガ行", L"5-row-cons-g");
  inflTypeTranslations.emplace(L"サ変・−ズル", L"irregular-suffix-zuru");
  inflTypeTranslations.emplace(L"五段・バ行", L"5-row-cons-b");
  inflTypeTranslations.emplace(L"五段・ワ行ウ音便", L"5-row-cons-w-u-onbin");
  inflTypeTranslations.emplace(L"下二・ダ行", L"2-row-lower-cons-d");
  inflTypeTranslations.emplace(L"五段・カ行促音便ユク",
                               L"5-row-cons-k-cons-onbin-yuku");
  inflTypeTranslations.emplace(L"上二・ダ行", L"2-row-upper-cons-d");
  inflTypeTranslations.emplace(L"五段・カ行促音便", L"5-row-cons-k-cons-onbin");
  inflTypeTranslations.emplace(L"一段・得ル", L"1-row-eru");
  inflTypeTranslations.emplace(L"四段・タ行", L"4-row-cons-t");
  inflTypeTranslations.emplace(L"五段・ナ行", L"5-row-cons-n");
  inflTypeTranslations.emplace(L"下二・ハ行", L"2-row-lower-cons-h");
  inflTypeTranslations.emplace(L"四段・ハ行", L"4-row-cons-h");
  inflTypeTranslations.emplace(L"四段・バ行", L"4-row-cons-b");
  inflTypeTranslations.emplace(L"サ変・スル", L"irregular-suru");
  inflTypeTranslations.emplace(L"上二・ハ行", L"2-row-upper-cons-h");
  inflTypeTranslations.emplace(L"下二・マ行", L"2-row-lower-cons-m");
  inflTypeTranslations.emplace(L"四段・サ行", L"4-row-cons-s");
  inflTypeTranslations.emplace(L"下二・ガ行", L"2-row-lower-cons-g");
  inflTypeTranslations.emplace(L"カ変・来ル", L"kuru-kanji");
  inflTypeTranslations.emplace(L"一段・クレル", L"1-row-kureru");
  inflTypeTranslations.emplace(L"下二・得", L"2-row-lower-u");
  inflTypeTranslations.emplace(L"カ変・クル", L"kuru-kana");
  inflTypeTranslations.emplace(L"ラ変", L"irregular-cons-r");
  inflTypeTranslations.emplace(L"下二・カ行", L"2-row-lower-cons-k");
  inflFormTranslations.emplace(L"*", L"*");
  inflFormTranslations.emplace(L"基本形", L"base");
  inflFormTranslations.emplace(L"文語基本形", L"classical-base");
  inflFormTranslations.emplace(L"未然ヌ接続", L"imperfective-nu-connection");
  inflFormTranslations.emplace(L"未然ウ接続", L"imperfective-u-connection");
  inflFormTranslations.emplace(L"連用タ接続", L"conjunctive-ta-connection");
  inflFormTranslations.emplace(L"連用テ接続", L"conjunctive-te-connection");
  inflFormTranslations.emplace(L"連用ゴザイ接続",
                               L"conjunctive-gozai-connection");
  inflFormTranslations.emplace(L"体言接続", L"uninflected-connection");
  inflFormTranslations.emplace(L"仮定形", L"subjunctive");
  inflFormTranslations.emplace(L"命令ｅ", L"imperative-e");
  inflFormTranslations.emplace(L"仮定縮約１", L"conditional-contracted-1");
  inflFormTranslations.emplace(L"仮定縮約２", L"conditional-contracted-2");
  inflFormTranslations.emplace(L"ガル接続", L"garu-connection");
  inflFormTranslations.emplace(L"未然形", L"imperfective");
  inflFormTranslations.emplace(L"連用形", L"conjunctive");
  inflFormTranslations.emplace(L"音便基本形", L"onbin-base");
  inflFormTranslations.emplace(L"連用デ接続", L"conjunctive-de-connection");
  inflFormTranslations.emplace(L"未然特殊", L"imperfective-special");
  inflFormTranslations.emplace(L"命令ｉ", L"imperative-i");
  inflFormTranslations.emplace(L"連用ニ接続", L"conjunctive-ni-connection");
  inflFormTranslations.emplace(L"命令ｙｏ", L"imperative-yo");
  inflFormTranslations.emplace(L"体言接続特殊", L"adnominal-special");
  inflFormTranslations.emplace(L"命令ｒｏ", L"imperative-ro");
  inflFormTranslations.emplace(L"体言接続特殊２",
                               L"uninflected-special-connection-2");
  inflFormTranslations.emplace(L"未然レル接続",
                               L"imperfective-reru-connection");
  inflFormTranslations.emplace(L"現代基本形", L"modern-base");
  inflFormTranslations.emplace(L"基本形-促音便",
                               L"base-onbin"); // not sure about this
}

ToStringUtil::StaticConstructor ToStringUtil::staticConstructor;

wstring ToStringUtil::getPOSTranslation(const wstring &s)
{
  return posTranslations[s];
}

const unordered_map<wstring, wstring> ToStringUtil::inflTypeTranslations =
    unordered_map<wstring, wstring>();

wstring ToStringUtil::getInflectionTypeTranslation(const wstring &s)
{
  return inflTypeTranslations[s];
}

const unordered_map<wstring, wstring> ToStringUtil::inflFormTranslations =
    unordered_map<wstring, wstring>();

wstring ToStringUtil::getInflectedFormTranslation(const wstring &s)
{
  return inflFormTranslations[s];
}

wstring ToStringUtil::getRomanization(const wstring &s)
{
  shared_ptr<StringBuilder> out = make_shared<StringBuilder>();
  try {
    getRomanization(out, s);
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
  return out->toString();
}

void ToStringUtil::getRomanization(
    shared_ptr<Appendable> builder,
    shared_ptr<std::wstring> s) 
{
  constexpr int len = s->length();
  for (int i = 0; i < len; i++) {
    // maximum lookahead: 3
    wchar_t ch = s->charAt(i);
    wchar_t ch2 = (i < len - 1) ? s->charAt(i + 1) : 0;
    wchar_t ch3 = (i < len - 2) ? s->charAt(i + 2) : 0;

    switch (ch) {
    case L'ッ':
      switch (ch2) {
      case L'カ':
      case L'キ':
      case L'ク':
      case L'ケ':
      case L'コ':
        builder->append(L'k');
        goto mainBreak;
      case L'サ':
      case L'シ':
      case L'ス':
      case L'セ':
      case L'ソ':
        builder->append(L's');
        goto mainBreak;
      case L'タ':
      case L'チ':
      case L'ツ':
      case L'テ':
      case L'ト':
        builder->append(L't');
        goto mainBreak;
      case L'パ':
      case L'ピ':
      case L'プ':
      case L'ペ':
      case L'ポ':
        builder->append(L'p');
        goto mainBreak;
      }
      break;
    case L'ア':
      builder->append(L'a');
      break;
    case L'イ':
      if (ch2 == L'ィ') {
        builder->append(L"yi");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"ye");
        i++;
      } else {
        builder->append(L'i');
      }
      break;
    case L'ウ':
      switch (ch2) {
      case L'ァ':
        builder->append(L"wa");
        i++;
        break;
      case L'ィ':
        builder->append(L"wi");
        i++;
        break;
      case L'ゥ':
        builder->append(L"wu");
        i++;
        break;
      case L'ェ':
        builder->append(L"we");
        i++;
        break;
      case L'ォ':
        builder->append(L"wo");
        i++;
        break;
      case L'ュ':
        builder->append(L"wyu");
        i++;
        break;
      default:
        builder->append(L'u');
        break;
      }
      break;
    case L'エ':
      builder->append(L'e');
      break;
    case L'オ':
      if (ch2 == L'ウ') {
        builder->append(L'ō');
        i++;
      } else {
        builder->append(L'o');
      }
      break;
    case L'カ':
      builder->append(L"ka");
      break;
    case L'キ':
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"kyō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"kyū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"kya");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"kyo");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"kyu");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"kye");
        i++;
      } else {
        builder->append(L"ki");
      }
      break;
    case L'ク':
      switch (ch2) {
      case L'ァ':
        builder->append(L"kwa");
        i++;
        break;
      case L'ィ':
        builder->append(L"kwi");
        i++;
        break;
      case L'ェ':
        builder->append(L"kwe");
        i++;
        break;
      case L'ォ':
        builder->append(L"kwo");
        i++;
        break;
      case L'ヮ':
        builder->append(L"kwa");
        i++;
        break;
      default:
        builder->append(L"ku");
        break;
      }
      break;
    case L'ケ':
      builder->append(L"ke");
      break;
    case L'コ':
      if (ch2 == L'ウ') {
        builder->append(L"kō");
        i++;
      } else {
        builder->append(L"ko");
      }
      break;
    case L'サ':
      builder->append(L"sa");
      break;
    case L'シ':
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"shō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"shū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"sha");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"sho");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"shu");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"she");
        i++;
      } else {
        builder->append(L"shi");
      }
      break;
    case L'ス':
      if (ch2 == L'ィ') {
        builder->append(L"si");
        i++;
      } else {
        builder->append(L"su");
      }
      break;
    case L'セ':
      builder->append(L"se");
      break;
    case L'ソ':
      if (ch2 == L'ウ') {
        builder->append(L"sō");
        i++;
      } else {
        builder->append(L"so");
      }
      break;
    case L'タ':
      builder->append(L"ta");
      break;
    case L'チ':
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"chō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"chū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"cha");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"cho");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"chu");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"che");
        i++;
      } else {
        builder->append(L"chi");
      }
      break;
    case L'ツ':
      if (ch2 == L'ァ') {
        builder->append(L"tsa");
        i++;
      } else if (ch2 == L'ィ') {
        builder->append(L"tsi");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"tse");
        i++;
      } else if (ch2 == L'ォ') {
        builder->append(L"tso");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"tsyu");
        i++;
      } else {
        builder->append(L"tsu");
      }
      break;
    case L'テ':
      if (ch2 == L'ィ') {
        builder->append(L"ti");
        i++;
      } else if (ch2 == L'ゥ') {
        builder->append(L"tu");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"tyu");
        i++;
      } else {
        builder->append(L"te");
      }
      break;
    case L'ト':
      if (ch2 == L'ウ') {
        builder->append(L"tō");
        i++;
      } else if (ch2 == L'ゥ') {
        builder->append(L"tu");
        i++;
      } else {
        builder->append(L"to");
      }
      break;
    case L'ナ':
      builder->append(L"na");
      break;
    case L'ニ':
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"nyō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"nyū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"nya");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"nyo");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"nyu");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"nye");
        i++;
      } else {
        builder->append(L"ni");
      }
      break;
    case L'ヌ':
      builder->append(L"nu");
      break;
    case L'ネ':
      builder->append(L"ne");
      break;
    case L'ノ':
      if (ch2 == L'ウ') {
        builder->append(L"nō");
        i++;
      } else {
        builder->append(L"no");
      }
      break;
    case L'ハ':
      builder->append(L"ha");
      break;
    case L'ヒ':
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"hyō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"hyū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"hya");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"hyo");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"hyu");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"hye");
        i++;
      } else {
        builder->append(L"hi");
      }
      break;
    case L'フ':
      if (ch2 == L'ャ') {
        builder->append(L"fya");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"fyu");
        i++;
      } else if (ch2 == L'ィ' && ch3 == L'ェ') {
        builder->append(L"fye");
        i += 2;
      } else if (ch2 == L'ョ') {
        builder->append(L"fyo");
        i++;
      } else if (ch2 == L'ァ') {
        builder->append(L"fa");
        i++;
      } else if (ch2 == L'ィ') {
        builder->append(L"fi");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"fe");
        i++;
      } else if (ch2 == L'ォ') {
        builder->append(L"fo");
        i++;
      } else {
        builder->append(L"fu");
      }
      break;
    case L'ヘ':
      builder->append(L"he");
      break;
    case L'ホ':
      if (ch2 == L'ウ') {
        builder->append(L"hō");
        i++;
      } else if (ch2 == L'ゥ') {
        builder->append(L"hu");
        i++;
      } else {
        builder->append(L"ho");
      }
      break;
    case L'マ':
      builder->append(L"ma");
      break;
    case L'ミ':
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"myō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"myū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"mya");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"myo");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"myu");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"mye");
        i++;
      } else {
        builder->append(L"mi");
      }
      break;
    case L'ム':
      builder->append(L"mu");
      break;
    case L'メ':
      builder->append(L"me");
      break;
    case L'モ':
      if (ch2 == L'ウ') {
        builder->append(L"mō");
        i++;
      } else {
        builder->append(L"mo");
      }
      break;
    case L'ヤ':
      builder->append(L"ya");
      break;
    case L'ユ':
      builder->append(L"yu");
      break;
    case L'ヨ':
      if (ch2 == L'ウ') {
        builder->append(L"yō");
        i++;
      } else {
        builder->append(L"yo");
      }
      break;
    case L'ラ':
      if (ch2 == L'゜') {
        builder->append(L"la");
        i++;
      } else {
        builder->append(L"ra");
      }
      break;
    case L'リ':
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"ryō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"ryū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"rya");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"ryo");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"ryu");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"rye");
        i++;
      } else if (ch2 == L'゜') {
        builder->append(L"li");
        i++;
      } else {
        builder->append(L"ri");
      }
      break;
    case L'ル':
      if (ch2 == L'゜') {
        builder->append(L"lu");
        i++;
      } else {
        builder->append(L"ru");
      }
      break;
    case L'レ':
      if (ch2 == L'゜') {
        builder->append(L"le");
        i++;
      } else {
        builder->append(L"re");
      }
      break;
    case L'ロ':
      if (ch2 == L'ウ') {
        builder->append(L"rō");
        i++;
      } else if (ch2 == L'゜') {
        builder->append(L"lo");
        i++;
      } else {
        builder->append(L"ro");
      }
      break;
    case L'ワ':
      builder->append(L"wa");
      break;
    case L'ヰ':
      builder->append(L"i");
      break;
    case L'ヱ':
      builder->append(L"e");
      break;
    case L'ヲ':
      builder->append(L"o");
      break;
    case L'ン':
      switch (ch2) {
      case L'バ':
      case L'ビ':
      case L'ブ':
      case L'ベ':
      case L'ボ':
      case L'パ':
      case L'ピ':
      case L'プ':
      case L'ペ':
      case L'ポ':
      case L'マ':
      case L'ミ':
      case L'ム':
      case L'メ':
      case L'モ':
        builder->append(L'm');
        goto mainBreak;
      case L'ヤ':
      case L'ユ':
      case L'ヨ':
      case L'ア':
      case L'イ':
      case L'ウ':
      case L'エ':
      case L'オ':
        builder->append(L"n'");
        goto mainBreak;
      default:
        builder->append(L"n");
        goto mainBreak;
      }
    case L'ガ':
      builder->append(L"ga");
      break;
    case L'ギ':
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"gyō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"gyū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"gya");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"gyo");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"gyu");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"gye");
        i++;
      } else {
        builder->append(L"gi");
      }
      break;
    case L'グ':
      switch (ch2) {
      case L'ァ':
        builder->append(L"gwa");
        i++;
        break;
      case L'ィ':
        builder->append(L"gwi");
        i++;
        break;
      case L'ェ':
        builder->append(L"gwe");
        i++;
        break;
      case L'ォ':
        builder->append(L"gwo");
        i++;
        break;
      case L'ヮ':
        builder->append(L"gwa");
        i++;
        break;
      default:
        builder->append(L"gu");
        break;
      }
      break;
    case L'ゲ':
      builder->append(L"ge");
      break;
    case L'ゴ':
      if (ch2 == L'ウ') {
        builder->append(L"gō");
        i++;
      } else {
        builder->append(L"go");
      }
      break;
    case L'ザ':
      builder->append(L"za");
      break;
    case L'ジ':
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"jō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"jū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"ja");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"jo");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"ju");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"je");
        i++;
      } else {
        builder->append(L"ji");
      }
      break;
    case L'ズ':
      if (ch2 == L'ィ') {
        builder->append(L"zi");
        i++;
      } else {
        builder->append(L"zu");
      }
      break;
    case L'ゼ':
      builder->append(L"ze");
      break;
    case L'ゾ':
      if (ch2 == L'ウ') {
        builder->append(L"zō");
        i++;
      } else {
        builder->append(L"zo");
      }
      break;
    case L'ダ':
      builder->append(L"da");
      break;
    case L'ヂ':
      // TODO: investigate all this
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"jō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"jū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"ja");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"jo");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"ju");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"je");
        i++;
      } else {
        builder->append(L"ji");
      }
      break;
    case L'ヅ':
      builder->append(L"zu");
      break;
    case L'デ':
      if (ch2 == L'ィ') {
        builder->append(L"di");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"dyu");
        i++;
      } else {
        builder->append(L"de");
      }
      break;
    case L'ド':
      if (ch2 == L'ウ') {
        builder->append(L"dō");
        i++;
      } else if (ch2 == L'ゥ') {
        builder->append(L"du");
        i++;
      } else {
        builder->append(L"do");
      }
      break;
    case L'バ':
      builder->append(L"ba");
      break;
    case L'ビ':
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"byō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"byū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"bya");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"byo");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"byu");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"bye");
        i++;
      } else {
        builder->append(L"bi");
      }
      break;
    case L'ブ':
      builder->append(L"bu");
      break;
    case L'ベ':
      builder->append(L"be");
      break;
    case L'ボ':
      if (ch2 == L'ウ') {
        builder->append(L"bō");
        i++;
      } else {
        builder->append(L"bo");
      }
      break;
    case L'パ':
      builder->append(L"pa");
      break;
    case L'ピ':
      if (ch2 == L'ョ' && ch3 == L'ウ') {
        builder->append(L"pyō");
        i += 2;
      } else if (ch2 == L'ュ' && ch3 == L'ウ') {
        builder->append(L"pyū");
        i += 2;
      } else if (ch2 == L'ャ') {
        builder->append(L"pya");
        i++;
      } else if (ch2 == L'ョ') {
        builder->append(L"pyo");
        i++;
      } else if (ch2 == L'ュ') {
        builder->append(L"pyu");
        i++;
      } else if (ch2 == L'ェ') {
        builder->append(L"pye");
        i++;
      } else {
        builder->append(L"pi");
      }
      break;
    case L'プ':
      builder->append(L"pu");
      break;
    case L'ペ':
      builder->append(L"pe");
      break;
    case L'ポ':
      if (ch2 == L'ウ') {
        builder->append(L"pō");
        i++;
      } else {
        builder->append(L"po");
      }
      break;
    case L'ヷ':
      builder->append(L"va");
      break;
    case L'ヸ':
      builder->append(L"vi");
      break;
    case L'ヹ':
      builder->append(L"ve");
      break;
    case L'ヺ':
      builder->append(L"vo");
      break;
    case L'ヴ':
      if (ch2 == L'ィ' && ch3 == L'ェ') {
        builder->append(L"vye");
        i += 2;
      } else {
        builder->append(L'v');
      }
      break;
    case L'ァ':
      builder->append(L'a');
      break;
    case L'ィ':
      builder->append(L'i');
      break;
    case L'ゥ':
      builder->append(L'u');
      break;
    case L'ェ':
      builder->append(L'e');
      break;
    case L'ォ':
      builder->append(L'o');
      break;
    case L'ヮ':
      builder->append(L"wa");
      break;
    case L'ャ':
      builder->append(L"ya");
      break;
    case L'ュ':
      builder->append(L"yu");
      break;
    case L'ョ':
      builder->append(L"yo");
      break;
    case L'ー':
      break;
    default:
      builder->append(ch);
    }
  mainBreak:;
  }
}
} // namespace org::apache::lucene::analysis::ja::util