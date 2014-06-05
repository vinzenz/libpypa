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
