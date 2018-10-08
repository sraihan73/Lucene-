using namespace std;

#include "RomanianStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    RomanianStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    RomanianStemmer::a_0 = {make_shared<org::tartarus::snowball::Among>(
                                L"", -1, 3, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"I", 0, 1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"U", 0, 2, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    RomanianStemmer::a_1 = {
        make_shared<org::tartarus::snowball::Among>(L"ea", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"a\u0163ia", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aua", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iua", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"a\u0163ie", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ele", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ile", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iile", 6, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iei", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atei", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ii", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ului", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ul", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"elor", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ilor", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iilor", 14, 4, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    RomanianStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"icala", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iciva", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ativa", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itiva", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icale", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"a\u0163iune", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u0163iune", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atoare", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itoare", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0103toare", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icitate", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abilitate", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibilitate", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivitate", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icive", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ative", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itive", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icali", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atori", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icatori", 18, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itori", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0103tori", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icitati", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abilitati", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivitati", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icivi", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ativi", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itivi", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icit\u0103i", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abilit\u0103i", -1, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivit\u0103i", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icit\u0103\u0163i", -1, 4,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abilit\u0103\u0163i", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivit\u0103\u0163i", -1, 3,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ical", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ator", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icator", 35, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itor", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0103tor", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iciv", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ativ", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itiv", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ical\u0103", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iciv\u0103", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ativ\u0103", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itiv\u0103", -1, 6, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    RomanianStemmer::a_3 = {
        make_shared<org::tartarus::snowball::Among>(L"ica", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abila", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibila", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oasa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ata", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ita", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"anta", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ista", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uta", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iva", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ic", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ice", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abile", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibile", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isme", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iune", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oase", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ate", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itate", 17, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ite", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ante", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iste", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ute", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ive", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ici", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abili", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibili", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iuni", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atori", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ati", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itati", 30, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"anti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isti", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u015Fti", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"it\u0103i", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"o\u015Fi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"it\u0103\u0163i", -1, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abil", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibil", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ism", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ator", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"os", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"at", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"it", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ant", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ist", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ut", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iv", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ic\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abil\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibil\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oas\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"at\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"it\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ant\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ist\u0103", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ut\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iv\u0103", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    RomanianStemmer::a_4 = {
        make_shared<org::tartarus::snowball::Among>(L"ea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esc", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0103sc", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ind", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2nd", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"are", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ere", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ire", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2re", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"se", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ase", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sese", 10, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ise", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"use", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2se", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"e\u015Fte", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0103\u015Fte", -1, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eze", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ai", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eai", 19, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iai", 19, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sei", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"e\u015Fti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0103\u015Fti", -1, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ui", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ezi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2i", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"a\u015Fi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"se\u015Fi", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ase\u015Fi", 29, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sese\u015Fi", 29, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ise\u015Fi", 29, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"use\u015Fi", 29, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2se\u015Fi", 29, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u015Fi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"u\u015Fi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2\u015Fi", -1, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"a\u0163i", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ea\u0163i", 38, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ia\u0163i", 38, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"e\u0163i", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u0163i", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2\u0163i", -1, 2,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u0103\u0163i", -1, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ser\u0103\u0163i", -1, 2,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aser\u0103\u0163i", 45, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"seser\u0103\u0163i", 45,
                                                    2, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iser\u0103\u0163i", 45, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"user\u0103\u0163i", 45, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2ser\u0103\u0163i",
                                                    45, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u0103\u0163i", -1, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ur\u0103\u0163i", -1, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2r\u0103\u0163i", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"am", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eam", 54, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iam", 54, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"em", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asem", 57, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sesem", 57, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isem", 57, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"usem", 57, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2sem", 57, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"im", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2m", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0103m", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u0103m", 65, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ser\u0103m", 65, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aser\u0103m", 67, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"seser\u0103m", 67, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iser\u0103m", 67, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"user\u0103m", 67, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2ser\u0103m", 67, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u0103m", 65, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ur\u0103m", 65, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2r\u0103m", 65, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"au", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eau", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iau", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"indu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2ndu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ez", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"easc\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ser\u0103", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aser\u0103", 84, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"seser\u0103", 84, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iser\u0103", 84, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"user\u0103", 84, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2ser\u0103", 84, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ur\u0103", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2r\u0103", -1, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eaz\u0103", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    RomanianStemmer::a_5 = {
        make_shared<org::tartarus::snowball::Among>(L"a", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"e", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ie", 1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0103", -1, 1, L"",
                                                    methodObject)};
std::deque<wchar_t> const RomanianStemmer::g_v = {
    17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 32, 0, 0, 4};

void RomanianStemmer::copy_from(shared_ptr<RomanianStemmer> other)
{
  B_standard_suffix_removed = other->B_standard_suffix_removed;
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  I_pV = other->I_pV;
  SnowballProgram::copy_from(other);
}

bool RomanianStemmer::r_prelude()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 31
  // repeat, line 32
  while (true) {
    v_1 = cursor;
    do {
      // goto, line 32
      while (true) {
        v_2 = cursor;
        do {
          // (, line 32
          if (!(in_grouping(g_v, 97, 259))) {
            goto lab3Break;
          }
          // [, line 33
          bra = cursor;
          // or, line 33
          do {
            v_3 = cursor;
            do {
              // (, line 33
              // literal, line 33
              if (!(eq_s(1, L"u"))) {
                goto lab5Break;
              }
              // ], line 33
              ket = cursor;
              if (!(in_grouping(g_v, 97, 259))) {
                goto lab5Break;
              }
              // <-, line 33
              slice_from(L"U");
              goto lab4Break;
            } while (false);
          lab5Continue:;
          lab5Break:
            cursor = v_3;
            // (, line 34
            // literal, line 34
            if (!(eq_s(1, L"i"))) {
              goto lab3Break;
            }
            // ], line 34
            ket = cursor;
            if (!(in_grouping(g_v, 97, 259))) {
              goto lab3Break;
            }
            // <-, line 34
            slice_from(L"I");
          } while (false);
        lab4Continue:;
        lab4Break:
          cursor = v_2;
          goto golab2Break;
        } while (false);
      lab3Continue:;
      lab3Break:
        cursor = v_2;
        if (cursor >= limit) {
          goto lab1Break;
        }
        cursor++;
      golab2Continue:;
      }
    golab2Break:
      goto replab0Continue;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_1;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  return true;
}

bool RomanianStemmer::r_mark_regions()
{
  int v_1;
  int v_2;
  int v_3;
  int v_6;
  int v_8;
  // (, line 38
  I_pV = limit;
  I_p1 = limit;
  I_p2 = limit;
  // do, line 44
  v_1 = cursor;
  do {
    // (, line 44
    // or, line 46
    do {
      v_2 = cursor;
      do {
        // (, line 45
        if (!(in_grouping(g_v, 97, 259))) {
          goto lab2Break;
        }
        // or, line 45
        do {
          v_3 = cursor;
          do {
            // (, line 45
            if (!(out_grouping(g_v, 97, 259))) {
              goto lab4Break;
            }
            // gopast, line 45
            while (true) {
              do {
                if (!(in_grouping(g_v, 97, 259))) {
                  goto lab6Break;
                }
                goto golab5Break;
              } while (false);
            lab6Continue:;
            lab6Break:
              if (cursor >= limit) {
                goto lab4Break;
              }
              cursor++;
            golab5Continue:;
            }
          golab5Break:
            goto lab3Break;
          } while (false);
        lab4Continue:;
        lab4Break:
          cursor = v_3;
          // (, line 45
          if (!(in_grouping(g_v, 97, 259))) {
            goto lab2Break;
          }
          // gopast, line 45
          while (true) {
            do {
              if (!(out_grouping(g_v, 97, 259))) {
                goto lab8Break;
              }
              goto golab7Break;
            } while (false);
          lab8Continue:;
          lab8Break:
            if (cursor >= limit) {
              goto lab2Break;
            }
            cursor++;
          golab7Continue:;
          }
        golab7Break:;
        } while (false);
      lab3Continue:;
      lab3Break:
        goto lab1Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = v_2;
      // (, line 47
      if (!(out_grouping(g_v, 97, 259))) {
        goto lab0Break;
      }
      // or, line 47
      do {
        v_6 = cursor;
        do {
          // (, line 47
          if (!(out_grouping(g_v, 97, 259))) {
            goto lab10Break;
          }
          // gopast, line 47
          while (true) {
            do {
              if (!(in_grouping(g_v, 97, 259))) {
                goto lab12Break;
              }
              goto golab11Break;
            } while (false);
          lab12Continue:;
          lab12Break:
            if (cursor >= limit) {
              goto lab10Break;
            }
            cursor++;
          golab11Continue:;
          }
        golab11Break:
          goto lab9Break;
        } while (false);
      lab10Continue:;
      lab10Break:
        cursor = v_6;
        // (, line 47
        if (!(in_grouping(g_v, 97, 259))) {
          goto lab0Break;
        }
        // next, line 47
        if (cursor >= limit) {
          goto lab0Break;
        }
        cursor++;
      } while (false);
    lab9Continue:;
    lab9Break:;
    } while (false);
  lab1Continue:;
  lab1Break:
    // setmark pV, line 48
    I_pV = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 50
  v_8 = cursor;
  do {
    // (, line 50
    // gopast, line 51
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 259))) {
          goto lab15Break;
        }
        goto golab14Break;
      } while (false);
    lab15Continue:;
    lab15Break:
      if (cursor >= limit) {
        goto lab13Break;
      }
      cursor++;
    golab14Continue:;
    }
  golab14Break:
    // gopast, line 51
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 259))) {
          goto lab17Break;
        }
        goto golab16Break;
      } while (false);
    lab17Continue:;
    lab17Break:
      if (cursor >= limit) {
        goto lab13Break;
      }
      cursor++;
    golab16Continue:;
    }
  golab16Break:
    // setmark p1, line 51
    I_p1 = cursor;
    // gopast, line 52
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 259))) {
          goto lab19Break;
        }
        goto golab18Break;
      } while (false);
    lab19Continue:;
    lab19Break:
      if (cursor >= limit) {
        goto lab13Break;
      }
      cursor++;
    golab18Continue:;
    }
  golab18Break:
    // gopast, line 52
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 259))) {
          goto lab21Break;
        }
        goto golab20Break;
      } while (false);
    lab21Continue:;
    lab21Break:
      if (cursor >= limit) {
        goto lab13Break;
      }
      cursor++;
    golab20Continue:;
    }
  golab20Break:
    // setmark p2, line 52
    I_p2 = cursor;
  } while (false);
lab13Continue:;
lab13Break:
  cursor = v_8;
  return true;
}

bool RomanianStemmer::r_postlude()
{
  int among_var;
  int v_1;
  // repeat, line 56
  while (true) {
    v_1 = cursor;
    do {
      // (, line 56
      // [, line 58
      bra = cursor;
      // substring, line 58
      among_var = find_among(a_0, 3);
      if (among_var == 0) {
        goto lab1Break;
      }
      // ], line 58
      ket = cursor;
      switch (among_var) {
      case 0:
        goto lab1Break;
      case 1:
        // (, line 59
        // <-, line 59
        slice_from(L"i");
        break;
      case 2:
        // (, line 60
        // <-, line 60
        slice_from(L"u");
        break;
      case 3:
        // (, line 61
        // next, line 61
        if (cursor >= limit) {
          goto lab1Break;
        }
        cursor++;
        break;
      }
      goto replab0Continue;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_1;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  return true;
}

bool RomanianStemmer::r_RV()
{
  if (!(I_pV <= cursor)) {
    return false;
  }
  return true;
}

bool RomanianStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool RomanianStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool RomanianStemmer::r_step_0()
{
  int among_var;
  int v_1;
  // (, line 72
  // [, line 73
  ket = cursor;
  // substring, line 73
  among_var = find_among_b(a_1, 16);
  if (among_var == 0) {
    return false;
  }
  // ], line 73
  bra = cursor;
  // call R1, line 73
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 75
    // delete, line 75
    slice_del();
    break;
  case 2:
    // (, line 77
    // <-, line 77
    slice_from(L"a");
    break;
  case 3:
    // (, line 79
    // <-, line 79
    slice_from(L"e");
    break;
  case 4:
    // (, line 81
    // <-, line 81
    slice_from(L"i");
    break;
  case 5:
    // (, line 83
    // not, line 83
    {
      v_1 = limit - cursor;
      do {
        // literal, line 83
        if (!(eq_s_b(2, L"ab"))) {
          goto lab0Break;
        }
        return false;
      } while (false);
    lab0Continue:;
    lab0Break:
      cursor = limit - v_1;
    }
    // <-, line 83
    slice_from(L"i");
    break;
  case 6:
    // (, line 85
    // <-, line 85
    slice_from(L"at");
    break;
  case 7:
    // (, line 87
    // <-, line 87
    slice_from(L"a\u0163i");
    break;
  }
  return true;
}

bool RomanianStemmer::r_combo_suffix()
{
  int among_var;
  int v_1;
  // test, line 91
  v_1 = limit - cursor;
  // (, line 91
  // [, line 92
  ket = cursor;
  // substring, line 92
  among_var = find_among_b(a_2, 46);
  if (among_var == 0) {
    return false;
  }
  // ], line 92
  bra = cursor;
  // call R1, line 92
  if (!r_R1()) {
    return false;
  }
  // (, line 92
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 100
    // <-, line 101
    slice_from(L"abil");
    break;
  case 2:
    // (, line 103
    // <-, line 104
    slice_from(L"ibil");
    break;
  case 3:
    // (, line 106
    // <-, line 107
    slice_from(L"iv");
    break;
  case 4:
    // (, line 112
    // <-, line 113
    slice_from(L"ic");
    break;
  case 5:
    // (, line 117
    // <-, line 118
    slice_from(L"at");
    break;
  case 6:
    // (, line 121
    // <-, line 122
    slice_from(L"it");
    break;
  }
  // set standard_suffix_removed, line 125
  B_standard_suffix_removed = true;
  cursor = limit - v_1;
  return true;
}

bool RomanianStemmer::r_standard_suffix()
{
  int among_var;
  int v_1;
  // (, line 129
  // unset standard_suffix_removed, line 130
  B_standard_suffix_removed = false;
  // repeat, line 131
  while (true) {
    v_1 = limit - cursor;
    do {
      // call combo_suffix, line 131
      if (!r_combo_suffix()) {
        goto lab1Break;
      }
      goto replab0Continue;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  // [, line 132
  ket = cursor;
  // substring, line 132
  among_var = find_among_b(a_3, 62);
  if (among_var == 0) {
    return false;
  }
  // ], line 132
  bra = cursor;
  // call R2, line 132
  if (!r_R2()) {
    return false;
  }
  // (, line 132
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 148
    // delete, line 149
    slice_del();
    break;
  case 2:
    // (, line 151
    // literal, line 152
    if (!(eq_s_b(1, L"\u0163"))) {
      return false;
    }
    // ], line 152
    bra = cursor;
    // <-, line 152
    slice_from(L"t");
    break;
  case 3:
    // (, line 155
    // <-, line 156
    slice_from(L"ist");
    break;
  }
  // set standard_suffix_removed, line 160
  B_standard_suffix_removed = true;
  return true;
}

bool RomanianStemmer::r_verb_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  // setlimit, line 164
  v_1 = limit - cursor;
  // tomark, line 164
  if (cursor < I_pV) {
    return false;
  }
  cursor = I_pV;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 164
  // [, line 165
  ket = cursor;
  // substring, line 165
  among_var = find_among_b(a_4, 94);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 165
  bra = cursor;
  switch (among_var) {
  case 0:
    limit_backward = v_2;
    return false;
  case 1:
    // (, line 200
    // or, line 200
    do {
      v_3 = limit - cursor;
      do {
        if (!(out_grouping_b(g_v, 97, 259))) {
          goto lab1Break;
        }
        goto lab0Break;
      } while (false);
    lab1Continue:;
    lab1Break:
      cursor = limit - v_3;
      // literal, line 200
      if (!(eq_s_b(1, L"u"))) {
        limit_backward = v_2;
        return false;
      }
    } while (false);
  lab0Continue:;
  lab0Break:
    // delete, line 200
    slice_del();
    break;
  case 2:
    // (, line 214
    // delete, line 214
    slice_del();
    break;
  }
  limit_backward = v_2;
  return true;
}

bool RomanianStemmer::r_vowel_suffix()
{
  int among_var;
  // (, line 218
  // [, line 219
  ket = cursor;
  // substring, line 219
  among_var = find_among_b(a_5, 5);
  if (among_var == 0) {
    return false;
  }
  // ], line 219
  bra = cursor;
  // call RV, line 219
  if (!r_RV()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 220
    // delete, line 220
    slice_del();
    break;
  }
  return true;
}

bool RomanianStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  int v_8;
  // (, line 225
  // do, line 226
  v_1 = cursor;
  do {
    // call prelude, line 226
    if (!r_prelude()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 227
  v_2 = cursor;
  do {
    // call mark_regions, line 227
    if (!r_mark_regions()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = v_2;
  // backwards, line 228
  limit_backward = cursor;
  cursor = limit;
  // (, line 228
  // do, line 229
  v_3 = limit - cursor;
  do {
    // call step_0, line 229
    if (!r_step_0()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 230
  v_4 = limit - cursor;
  do {
    // call standard_suffix, line 230
    if (!r_standard_suffix()) {
      goto lab3Break;
    }
  } while (false);
lab3Continue:;
lab3Break:
  cursor = limit - v_4;
  // do, line 231
  v_5 = limit - cursor;
  do {
    // (, line 231
    // or, line 231
    do {
      v_6 = limit - cursor;
      do {
        // Boolean test standard_suffix_removed, line 231
        if (!(B_standard_suffix_removed)) {
          goto lab6Break;
        }
        goto lab5Break;
      } while (false);
    lab6Continue:;
    lab6Break:
      cursor = limit - v_6;
      // call verb_suffix, line 231
      if (!r_verb_suffix()) {
        goto lab4Break;
      }
    } while (false);
  lab5Continue:;
  lab5Break:;
  } while (false);
lab4Continue:;
lab4Break:
  cursor = limit - v_5;
  // do, line 232
  v_7 = limit - cursor;
  do {
    // call vowel_suffix, line 232
    if (!r_vowel_suffix()) {
      goto lab7Break;
    }
  } while (false);
lab7Continue:;
lab7Break:
  cursor = limit - v_7;
  cursor = limit_backward; // do, line 234
  v_8 = cursor;
  do {
    // call postlude, line 234
    if (!r_postlude()) {
      goto lab8Break;
    }
  } while (false);
lab8Continue:;
lab8Break:
  cursor = v_8;
  return true;
}

bool RomanianStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<RomanianStemmer>(o) != nullptr;
}

int RomanianStemmer::hashCode()
{
  return RomanianStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext