#include "Scope.h"


using namespace std;
using namespace cpm::sc;

Scope *Scope::getValueScope(const std::string &id) {
    Scope *scope = this;
    while (scope) {
        if (scope->contains(id))
            break;
        scope = scope->parent;
    }
    return scope;
}