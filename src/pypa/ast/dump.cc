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
#include <cassert>

#include <pypa/ast/visitor.hh>
#include <pypa/ast/tree_walker.hh>

namespace pypa {

void dump(AstPtr p) {
    if(p) {
        pypa::detail::visit_dump_internal(0, *p);
    }
}

}
