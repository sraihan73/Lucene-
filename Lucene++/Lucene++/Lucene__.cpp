//
//  Lucene__.cpp
//  Lucene++
//
//  Created by Syed Mamun Raihan on 10/21/18.
//  Copyright © 2018 Syed Mamun Raihan. All rights reserved.
//

#include <iostream>
#include "Lucene__.hpp"
#include "Lucene__Priv.hpp"

void Lucene__::HelloWorld(const char * s)
{
    Lucene__Priv *theObj = new Lucene__Priv;
    theObj->HelloWorldPriv(s);
    delete theObj;
};

void Lucene__Priv::HelloWorldPriv(const char * s) 
{
    std::cout << s << std::endl;
};

