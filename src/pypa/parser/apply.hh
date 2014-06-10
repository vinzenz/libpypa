// Copyright 2014 Vinzenz Feenstra
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef GUARD_PYPA_PARSER_APPLY_HH_INCLUDED
#define GUARD_PYPA_PARSER_APPLY_HH_INCLUDED

#include <pypa/parser/state.hh>

namespace pypa {

template< typename TargetT, typename BaseT>
bool apply(State & s, BaseT & b, bool(*f)(State &, TargetT&))
{
    TargetT t;
    if(f(s, t)) {
        b = t;
        return true;
    }
    return false;
}

template< typename TargetT, typename ContainerT>
bool push_apply(State & s, ContainerT & c, bool(*f)(State &, TargetT&)) {
    typename ContainerT::value_type v;
    if(apply<TargetT>(s, v, f)) {
        c.push_back(v);
        return true;
    }
    return false;
}

}

#endif // GUARD_PYPA_PARSER_APPLY_HH_INCLUDED
