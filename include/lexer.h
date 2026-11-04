#pragma once

#include "string_view.h"
#include "token.h"

bool lex_program(StringView src, Tokens* token_buf);
