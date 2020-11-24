#pragma once

#include <parameter.h>
#include <set>

namespace nap
{

    constexpr char dataPath[] = "/Users/macbook/Documents/Repositories/nap/apps/napaudioapp/data/fmsynth.json";
    constexpr char pythonHomePath[] = "/Users/macbook/Documents/Repositories/thirdparty/python/osx/install";

    struct Global
    {
        static ParameterGroup* parameters;
    };

}

