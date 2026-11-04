#include "type.h"

const char* basic_type_to_string(BasicType type) {
    switch (type) {
        case BASIC_TYPE_INT:
            return "int";
        case BASIC_TYPE_FLOAT:
            return "float";
        case BASIC_TYPE_BOOL:
            return "bool";
        case BASIC_TYPE_CHAR:
            return "char";
    }

    return "";
}
