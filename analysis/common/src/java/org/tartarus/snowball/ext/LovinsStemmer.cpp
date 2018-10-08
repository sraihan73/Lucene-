using namespace std;

#include "LovinsStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    LovinsStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    LovinsStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"d", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"f", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ph", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"th", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"l", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"or", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"es", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t", -1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    LovinsStemmer::a_1 = {
        make_shared<org::tartarus::snowball::Among>(L"s'", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"a", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ia", 1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ata", 1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ic", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aic", 4, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"allic", 4, 1, L"r_BB",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aric", 4, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atic", 4, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itic", 4, 1, L"r_H",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"antic", 4, 1, L"r_C",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"istic", 4, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alistic", 11, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aristic", 11, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivistic", 11, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ed", -1, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"anced", 15, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enced", 15, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ished", 15, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ied", 15, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ened", 15, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ioned", 15, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ated", 15, 1, L"r_I",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ented", 15, 1, L"r_C",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ized", 15, 1, L"r_F",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arized", 24, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oid", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aroid", 26, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hood", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ehood", 28, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ihood", 28, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"elihood", 30, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ward", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"e", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ae", 33, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ance", 33, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icance", 35, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ence", 33, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ide", 33, 1, L"r_L",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icide", 38, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"otide", 38, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"age", 33, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"able", 33, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atable", 42, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"izable", 42, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arizable", 44, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ible", 33, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"encible", 46, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ene", 33, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ine", 33, 1, L"r_M",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"idine", 49, 1, L"r_I",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"one", 33, 1, L"r_R",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ature", 33, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eature", 52, 1, L"r_Z",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ese", 33, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"wise", 33, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ate", 33, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entiate", 56, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"inate", 56, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ionate", 56, 1, L"r_D",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ite", 33, 1, L"r_AA",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ive", 33, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ative", 61, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ize", 33, 1, L"r_F",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alize", 63, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icalize", 64, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ialize", 64, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entialize", 66, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ionalize", 64, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arize", 63, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ing", -1, 1, L"r_N",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ancing", 70, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"encing", 70, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aging", 70, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ening", 70, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ioning", 70, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ating", 70, 1, L"r_I",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enting", 70, 1, L"r_C",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ying", 70, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"izing", 70, 1, L"r_F",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arizing", 79, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ish", -1, 1, L"r_C",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"yish", 81, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"al", -1, 1, L"r_BB",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ical", 84, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aical", 85, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"istical", 85, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oidal", 84, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eal", 84, 1, L"r_Y",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ial", 84, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ancial", 90, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arial", 90, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ential", 90, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ional", 84, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ational", 94, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"izational", 95, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ental", 84, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ful", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eful", 98, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iful", 98, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"yl", -1, 1, L"r_R",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ism", -1, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icism", 102, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oidism", 102, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alism", 102, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icalism", 105, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ionalism", 105, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"inism", 102, 1, L"r_J",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ativism", 102, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"um", -1, 1, L"r_U",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ium", 110, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ian", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ician", 112, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"en", -1, 1, L"r_F",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ogen", 114, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"on", -1, 1, L"r_S",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ion", 116, 1, L"r_Q",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ation", 117, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ication", 118, 1, L"r_G",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entiation", 118, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ination", 118, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isation", 118, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arisation", 122, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entation", 118, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ization", 118, 1, L"r_F",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arization", 125, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"action", 117, 1, L"r_G",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"o", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar", -1, 1, L"r_X",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ear", 129, 1, L"r_Y",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ier", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ariser", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"izer", -1, 1, L"r_F",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arizer", 133, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"or", -1, 1, L"r_T",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ator", 135, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"s", -1, 1, L"r_W",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"'s", 137, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"as", 137, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ics", 137, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"istics", 140, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"es", 137, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ances", 142, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ences", 142, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ides", 142, 1, L"r_L",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oides", 145, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ages", 142, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ies", 142, 1, L"r_P",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"acies", 148, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ancies", 148, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"encies", 148, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aries", 148, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ities", 148, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alities", 153, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivities", 153, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ines", 142, 1, L"r_M",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nesses", 142, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ates", 142, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atives", 142, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ings", 137, 1, L"r_N",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"is", 137, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"als", 137, 1, L"r_BB",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ials", 162, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entials", 163, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ionals", 162, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isms", 137, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ians", 137, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icians", 167, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ions", 137, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ations", 169, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arisations", 170, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entations", 170, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"izations", 170, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arizations", 173, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ars", 137, 1, L"r_O",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iers", 137, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"izers", 137, 1, L"r_F",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ators", 137, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"less", 137, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eless", 179, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ness", 137, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eness", 181, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ableness", 182, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eableness", 183, 1,
                                                    L"r_E", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibleness", 182, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ateness", 182, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iteness", 182, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iveness", 182, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ativeness", 188, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ingness", 181, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ishness", 181, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iness", 181, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ariness", 192, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alness", 181, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icalness", 194, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"antialness", 194, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entialness", 194, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ionalness", 194, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"fulness", 181, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lessness", 181, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ousness", 181, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eousness", 201, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iousness", 201, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itousness", 201, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entness", 181, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ants", 137, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ists", 137, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icists", 207, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"us", 137, 1, L"r_V",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ous", 209, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eous", 210, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aceous", 211, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"antaneous", 211, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ious", 210, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"acious", 214, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itous", 210, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ant", -1, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icant", 217, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ent", -1, 1, L"r_C",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ement", 219, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"izement", 220, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ist", -1, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icist", 222, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alist", 222, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icalist", 224, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ialist", 224, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ionist", 222, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entist", 222, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"y", -1, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"acy", 229, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ancy", 229, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ency", 229, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ly", 229, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ealy", 233, 1, L"r_Y",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ably", 233, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibly", 233, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"edly", 233, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iedly", 237, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ely", 233, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ately", 239, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ively", 239, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atively", 241, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ingly", 233, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atingly", 243, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ily", 233, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lily", 245, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arily", 245, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ally", 233, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ically", 248, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aically", 249, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"allically", 249, 1,
                                                    L"r_C", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"istically", 249, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alistically", 252, 1,
                                                    L"r_B", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oidally", 248, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ially", 248, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entially", 255, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ionally", 248, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ationally", 257, 1,
                                                    L"r_B", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"izationally", 258, 1,
                                                    L"r_B", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entally", 248, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"fully", 233, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"efully", 261, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ifully", 261, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enly", 233, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arly", 233, 1, L"r_K",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"early", 265, 1, L"r_Y",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lessly", 233, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ously", 233, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eously", 268, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iously", 268, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ently", 233, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ary", 229, 1, L"r_F",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ery", 229, 1, L"r_E",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icianry", 229, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atory", 229, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ity", 229, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"acity", 276, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icity", 276, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eity", 276, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ality", 276, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icality", 280, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iality", 280, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"antiality", 282, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entiality", 282, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ionality", 280, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"elity", 276, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ability", 276, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"izability", 287, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arizability", 288, 1,
                                                    L"r_A", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibility", 276, 1, L"r_A",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"inity", 276, 1, L"r_CC",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arity", 276, 1, L"r_B",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivity", 276, 1, L"r_A",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    LovinsStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"bb", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dd", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gg", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ll", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mm", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nn", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pp", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"rr", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ss", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tt", -1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    LovinsStemmer::a_3 = {
        make_shared<org::tartarus::snowball::Among>(L"uad", -1, 18, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"vad", -1, 19, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"cid", -1, 20, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lid", -1, 21, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erid", -1, 22, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pand", -1, 23, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"end", -1, 24, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ond", -1, 25, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lud", -1, 26, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"rud", -1, 27, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ul", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"her", -1, 28, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"metr", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"istr", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"urs", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uct", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"et", -1, 32, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mit", -1, 29, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ent", -1, 30, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"umpt", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"rpt", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ert", -1, 31, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"yt", -1, 33, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iev", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"olv", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ax", -1, 14, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ex", -1, 15, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bex", 26, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dex", 26, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pex", 26, 12, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tex", 26, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ix", -1, 16, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lux", -1, 17, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"yz", -1, 34, L"",
                                                    methodObject)};

void LovinsStemmer::copy_from(shared_ptr<LovinsStemmer> other)
{
  SnowballProgram::copy_from(other);
}

bool LovinsStemmer::r_A()
{
  // (, line 21
  // hop, line 21
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  return true;
}

bool LovinsStemmer::r_B()
{
  // (, line 22
  // hop, line 22
  {
    int c = cursor - 3;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  return true;
}

bool LovinsStemmer::r_C()
{
  // (, line 23
  // hop, line 23
  {
    int c = cursor - 4;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  return true;
}

bool LovinsStemmer::r_D()
{
  // (, line 24
  // hop, line 24
  {
    int c = cursor - 5;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  return true;
}

bool LovinsStemmer::r_E()
{
  int v_1;
  int v_2;
  // (, line 25
  // test, line 25
  v_1 = limit - cursor;
  // hop, line 25
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // not, line 25
  {
    v_2 = limit - cursor;
    do {
      // literal, line 25
      if (!(eq_s_b(1, L"e"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  return true;
}

bool LovinsStemmer::r_F()
{
  int v_1;
  int v_2;
  // (, line 26
  // test, line 26
  v_1 = limit - cursor;
  // hop, line 26
  {
    int c = cursor - 3;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // not, line 26
  {
    v_2 = limit - cursor;
    do {
      // literal, line 26
      if (!(eq_s_b(1, L"e"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  return true;
}

bool LovinsStemmer::r_G()
{
  int v_1;
  // (, line 27
  // test, line 27
  v_1 = limit - cursor;
  // hop, line 27
  {
    int c = cursor - 3;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // literal, line 27
  if (!(eq_s_b(1, L"f"))) {
    return false;
  }
  return true;
}

bool LovinsStemmer::r_H()
{
  int v_1;
  int v_2;
  // (, line 28
  // test, line 28
  v_1 = limit - cursor;
  // hop, line 28
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // or, line 28
  do {
    v_2 = limit - cursor;
    do {
      // literal, line 28
      if (!(eq_s_b(1, L"t"))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    // literal, line 28
    if (!(eq_s_b(2, L"ll"))) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool LovinsStemmer::r_I()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 29
  // test, line 29
  v_1 = limit - cursor;
  // hop, line 29
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // not, line 29
  {
    v_2 = limit - cursor;
    do {
      // literal, line 29
      if (!(eq_s_b(1, L"o"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  {
    // not, line 29
    v_3 = limit - cursor;
    do {
      // literal, line 29
      if (!(eq_s_b(1, L"e"))) {
        goto lab1Break;
      }
      return false;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_3;
  }
  return true;
}

bool LovinsStemmer::r_J()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 30
  // test, line 30
  v_1 = limit - cursor;
  // hop, line 30
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // not, line 30
  {
    v_2 = limit - cursor;
    do {
      // literal, line 30
      if (!(eq_s_b(1, L"a"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  {
    // not, line 30
    v_3 = limit - cursor;
    do {
      // literal, line 30
      if (!(eq_s_b(1, L"e"))) {
        goto lab1Break;
      }
      return false;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_3;
  }
  return true;
}

bool LovinsStemmer::r_K()
{
  int v_1;
  int v_2;
  // (, line 31
  // test, line 31
  v_1 = limit - cursor;
  // hop, line 31
  {
    int c = cursor - 3;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // or, line 31
  do {
    v_2 = limit - cursor;
    do {
      // literal, line 31
      if (!(eq_s_b(1, L"l"))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    do {
      // literal, line 31
      if (!(eq_s_b(1, L"i"))) {
        goto lab2Break;
      }
      goto lab0Break;
    } while (false);
  lab2Continue:;
  lab2Break:
    cursor = limit - v_2;
    // (, line 31
    // literal, line 31
    if (!(eq_s_b(1, L"e"))) {
      return false;
    }
    // next, line 31
    if (cursor <= limit_backward) {
      return false;
    }
    cursor--;
    // literal, line 31
    if (!(eq_s_b(1, L"u"))) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool LovinsStemmer::r_L()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  // (, line 32
  // test, line 32
  v_1 = limit - cursor;
  // hop, line 32
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // not, line 32
  {
    v_2 = limit - cursor;
    do {
      // literal, line 32
      if (!(eq_s_b(1, L"u"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  {
    // not, line 32
    v_3 = limit - cursor;
    do {
      // literal, line 32
      if (!(eq_s_b(1, L"x"))) {
        goto lab1Break;
      }
      return false;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_3;
  }
  {
    // not, line 32
    v_4 = limit - cursor;
    do {
      // (, line 32
      // literal, line 32
      if (!(eq_s_b(1, L"s"))) {
        goto lab2Break;
      }
      {
        // not, line 32
        v_5 = limit - cursor;
        do {
          // literal, line 32
          if (!(eq_s_b(1, L"o"))) {
            goto lab3Break;
          }
          goto lab2Break;
        } while (false);
      lab3Continue:;
      lab3Break:
        cursor = limit - v_5;
      }
      return false;
    } while (false);
  lab2Continue:;
  lab2Break:
    cursor = limit - v_4;
  }
  return true;
}

bool LovinsStemmer::r_M()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  // (, line 33
  // test, line 33
  v_1 = limit - cursor;
  // hop, line 33
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // not, line 33
  {
    v_2 = limit - cursor;
    do {
      // literal, line 33
      if (!(eq_s_b(1, L"a"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  {
    // not, line 33
    v_3 = limit - cursor;
    do {
      // literal, line 33
      if (!(eq_s_b(1, L"c"))) {
        goto lab1Break;
      }
      return false;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_3;
  }
  {
    // not, line 33
    v_4 = limit - cursor;
    do {
      // literal, line 33
      if (!(eq_s_b(1, L"e"))) {
        goto lab2Break;
      }
      return false;
    } while (false);
  lab2Continue:;
  lab2Break:
    cursor = limit - v_4;
  }
  {
    // not, line 33
    v_5 = limit - cursor;
    do {
      // literal, line 33
      if (!(eq_s_b(1, L"m"))) {
        goto lab3Break;
      }
      return false;
    } while (false);
  lab3Continue:;
  lab3Break:
    cursor = limit - v_5;
  }
  return true;
}

bool LovinsStemmer::r_N()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 34
  // test, line 34
  v_1 = limit - cursor;
  // hop, line 34
  {
    int c = cursor - 3;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // (, line 34
  // hop, line 34
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  // or, line 34
  do {
    v_2 = limit - cursor;
    do {
      {
        // not, line 34
        v_3 = limit - cursor;
        do {
          // literal, line 34
          if (!(eq_s_b(1, L"s"))) {
            goto lab2Break;
          }
          goto lab1Break;
        } while (false);
      lab2Continue:;
      lab2Break:
        cursor = limit - v_3;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    // hop, line 34
    {
      int c = cursor - 2;
      if (limit_backward > c || c > limit) {
        return false;
      }
      cursor = c;
    }
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool LovinsStemmer::r_O()
{
  int v_1;
  int v_2;
  // (, line 35
  // test, line 35
  v_1 = limit - cursor;
  // hop, line 35
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // or, line 35
  do {
    v_2 = limit - cursor;
    do {
      // literal, line 35
      if (!(eq_s_b(1, L"l"))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    // literal, line 35
    if (!(eq_s_b(1, L"i"))) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool LovinsStemmer::r_P()
{
  int v_1;
  int v_2;
  // (, line 36
  // test, line 36
  v_1 = limit - cursor;
  // hop, line 36
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // not, line 36
  {
    v_2 = limit - cursor;
    do {
      // literal, line 36
      if (!(eq_s_b(1, L"c"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  return true;
}

bool LovinsStemmer::r_Q()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // (, line 37
  // test, line 37
  v_1 = limit - cursor;
  // hop, line 37
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // test, line 37
  v_2 = limit - cursor;
  // hop, line 37
  {
    int c = cursor - 3;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_2;
  // not, line 37
  {
    v_3 = limit - cursor;
    do {
      // literal, line 37
      if (!(eq_s_b(1, L"l"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_3;
  }
  {
    // not, line 37
    v_4 = limit - cursor;
    do {
      // literal, line 37
      if (!(eq_s_b(1, L"n"))) {
        goto lab1Break;
      }
      return false;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_4;
  }
  return true;
}

bool LovinsStemmer::r_R()
{
  int v_1;
  int v_2;
  // (, line 38
  // test, line 38
  v_1 = limit - cursor;
  // hop, line 38
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // or, line 38
  do {
    v_2 = limit - cursor;
    do {
      // literal, line 38
      if (!(eq_s_b(1, L"n"))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    // literal, line 38
    if (!(eq_s_b(1, L"r"))) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool LovinsStemmer::r_S()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 39
  // test, line 39
  v_1 = limit - cursor;
  // hop, line 39
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // or, line 39
  do {
    v_2 = limit - cursor;
    do {
      // literal, line 39
      if (!(eq_s_b(2, L"dr"))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    // (, line 39
    // literal, line 39
    if (!(eq_s_b(1, L"t"))) {
      return false;
    }
    {
      // not, line 39
      v_3 = limit - cursor;
      do {
        // literal, line 39
        if (!(eq_s_b(1, L"t"))) {
          goto lab2Break;
        }
        return false;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_3;
    }
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool LovinsStemmer::r_T()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 40
  // test, line 40
  v_1 = limit - cursor;
  // hop, line 40
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // or, line 40
  do {
    v_2 = limit - cursor;
    do {
      // literal, line 40
      if (!(eq_s_b(1, L"s"))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    // (, line 40
    // literal, line 40
    if (!(eq_s_b(1, L"t"))) {
      return false;
    }
    {
      // not, line 40
      v_3 = limit - cursor;
      do {
        // literal, line 40
        if (!(eq_s_b(1, L"o"))) {
          goto lab2Break;
        }
        return false;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_3;
    }
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool LovinsStemmer::r_U()
{
  int v_1;
  int v_2;
  // (, line 41
  // test, line 41
  v_1 = limit - cursor;
  // hop, line 41
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // or, line 41
  do {
    v_2 = limit - cursor;
    do {
      // literal, line 41
      if (!(eq_s_b(1, L"l"))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    do {
      // literal, line 41
      if (!(eq_s_b(1, L"m"))) {
        goto lab2Break;
      }
      goto lab0Break;
    } while (false);
  lab2Continue:;
  lab2Break:
    cursor = limit - v_2;
    do {
      // literal, line 41
      if (!(eq_s_b(1, L"n"))) {
        goto lab3Break;
      }
      goto lab0Break;
    } while (false);
  lab3Continue:;
  lab3Break:
    cursor = limit - v_2;
    // literal, line 41
    if (!(eq_s_b(1, L"r"))) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool LovinsStemmer::r_V()
{
  int v_1;
  // (, line 42
  // test, line 42
  v_1 = limit - cursor;
  // hop, line 42
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // literal, line 42
  if (!(eq_s_b(1, L"c"))) {
    return false;
  }
  return true;
}

bool LovinsStemmer::r_W()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 43
  // test, line 43
  v_1 = limit - cursor;
  // hop, line 43
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // not, line 43
  {
    v_2 = limit - cursor;
    do {
      // literal, line 43
      if (!(eq_s_b(1, L"s"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  {
    // not, line 43
    v_3 = limit - cursor;
    do {
      // literal, line 43
      if (!(eq_s_b(1, L"u"))) {
        goto lab1Break;
      }
      return false;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_3;
  }
  return true;
}

bool LovinsStemmer::r_X()
{
  int v_1;
  int v_2;
  // (, line 44
  // test, line 44
  v_1 = limit - cursor;
  // hop, line 44
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // or, line 44
  do {
    v_2 = limit - cursor;
    do {
      // literal, line 44
      if (!(eq_s_b(1, L"l"))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    do {
      // literal, line 44
      if (!(eq_s_b(1, L"i"))) {
        goto lab2Break;
      }
      goto lab0Break;
    } while (false);
  lab2Continue:;
  lab2Break:
    cursor = limit - v_2;
    // (, line 44
    // literal, line 44
    if (!(eq_s_b(1, L"e"))) {
      return false;
    }
    // next, line 44
    if (cursor <= limit_backward) {
      return false;
    }
    cursor--;
    // literal, line 44
    if (!(eq_s_b(1, L"u"))) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool LovinsStemmer::r_Y()
{
  int v_1;
  // (, line 45
  // test, line 45
  v_1 = limit - cursor;
  // hop, line 45
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // literal, line 45
  if (!(eq_s_b(2, L"in"))) {
    return false;
  }
  return true;
}

bool LovinsStemmer::r_Z()
{
  int v_1;
  int v_2;
  // (, line 46
  // test, line 46
  v_1 = limit - cursor;
  // hop, line 46
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // not, line 46
  {
    v_2 = limit - cursor;
    do {
      // literal, line 46
      if (!(eq_s_b(1, L"f"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  return true;
}

bool LovinsStemmer::r_AA()
{
  int v_1;
  // (, line 47
  // test, line 47
  v_1 = limit - cursor;
  // hop, line 47
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // among, line 47
  if (find_among_b(a_0, 9) == 0) {
    return false;
  }
  return true;
}

bool LovinsStemmer::r_BB()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 49
  // test, line 49
  v_1 = limit - cursor;
  // hop, line 49
  {
    int c = cursor - 3;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // not, line 49
  {
    v_2 = limit - cursor;
    do {
      // literal, line 49
      if (!(eq_s_b(3, L"met"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  {
    // not, line 49
    v_3 = limit - cursor;
    do {
      // literal, line 49
      if (!(eq_s_b(4, L"ryst"))) {
        goto lab1Break;
      }
      return false;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_3;
  }
  return true;
}

bool LovinsStemmer::r_CC()
{
  int v_1;
  // (, line 50
  // test, line 50
  v_1 = limit - cursor;
  // hop, line 50
  {
    int c = cursor - 2;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = limit - v_1;
  // literal, line 50
  if (!(eq_s_b(1, L"l"))) {
    return false;
  }
  return true;
}

bool LovinsStemmer::r_endings()
{
  int among_var;
  // (, line 55
  // [, line 56
  ket = cursor;
  // substring, line 56
  among_var = find_among_b(a_1, 294);
  if (among_var == 0) {
    return false;
  }
  // ], line 56
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 145
    // delete, line 145
    slice_del();
    break;
  }
  return true;
}

bool LovinsStemmer::r_undouble()
{
  int v_1;
  // (, line 151
  // test, line 152
  v_1 = limit - cursor;
  // substring, line 152
  if (find_among_b(a_2, 10) == 0) {
    return false;
  }
  cursor = limit - v_1;
  // [, line 154
  ket = cursor;
  // next, line 154
  if (cursor <= limit_backward) {
    return false;
  }
  cursor--;
  // ], line 154
  bra = cursor;
  // delete, line 154
  slice_del();
  return true;
}

bool LovinsStemmer::r_respell()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  int v_8;
  // (, line 159
  // [, line 160
  ket = cursor;
  // substring, line 160
  among_var = find_among_b(a_3, 34);
  if (among_var == 0) {
    return false;
  }
  // ], line 160
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 161
    // <-, line 161
    slice_from(L"ief");
    break;
  case 2:
    // (, line 162
    // <-, line 162
    slice_from(L"uc");
    break;
  case 3:
    // (, line 163
    // <-, line 163
    slice_from(L"um");
    break;
  case 4:
    // (, line 164
    // <-, line 164
    slice_from(L"rb");
    break;
  case 5:
    // (, line 165
    // <-, line 165
    slice_from(L"ur");
    break;
  case 6:
    // (, line 166
    // <-, line 166
    slice_from(L"ister");
    break;
  case 7:
    // (, line 167
    // <-, line 167
    slice_from(L"meter");
    break;
  case 8:
    // (, line 168
    // <-, line 168
    slice_from(L"olut");
    break;
  case 9:
    // (, line 169
    // not, line 169
    {
      v_1 = limit - cursor;
      do {
        // literal, line 169
        if (!(eq_s_b(1, L"a"))) {
          goto lab0Break;
        }
        return false;
      } while (false);
    lab0Continue:;
    lab0Break:
      cursor = limit - v_1;
    }
    {
      // not, line 169
      v_2 = limit - cursor;
      do {
        // literal, line 169
        if (!(eq_s_b(1, L"i"))) {
          goto lab1Break;
        }
        return false;
      } while (false);
    lab1Continue:;
    lab1Break:
      cursor = limit - v_2;
    }
    {
      // not, line 169
      v_3 = limit - cursor;
      do {
        // literal, line 169
        if (!(eq_s_b(1, L"o"))) {
          goto lab2Break;
        }
        return false;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_3;
    }
    // <-, line 169
    slice_from(L"l");
    break;
  case 10:
    // (, line 170
    // <-, line 170
    slice_from(L"bic");
    break;
  case 11:
    // (, line 171
    // <-, line 171
    slice_from(L"dic");
    break;
  case 12:
    // (, line 172
    // <-, line 172
    slice_from(L"pic");
    break;
  case 13:
    // (, line 173
    // <-, line 173
    slice_from(L"tic");
    break;
  case 14:
    // (, line 174
    // <-, 