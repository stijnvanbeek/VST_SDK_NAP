#pragma once

#include "version.h"
#include <parameter.h>
#include <set>

namespace nap
{

    struct Global
    {
        static std::vector<nap::Parameter*> parameters;
    };

}

