#include "Prototype.h"

Util::Reflection::Prototype *Util::Reflection::Prototype::clone() const {
    Exception::throwException(Exception::UNSUPPORTED_OPERATION, "Prototype class does not implement clone()!");
}
