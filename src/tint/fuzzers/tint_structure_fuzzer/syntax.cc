#include "syntax.h"
#include <cmath>

namespace tint::fuzzers::structure_fuzzer {

namespace {

struct Context;

using fn = void (*)(Context&);

std::string genIdent(uint32_t n, char c = 'x') {
    return std::string(1, c) + std::to_string(n);
}

struct Context {
    std::stringstream& buffer;
    const uint8_t* data;
    size_t size;
    size_t offset = 0;

    uint8_t byte() {
        if (size == 0) {
            return 0;
        }
        uint8_t result = data[0];
        ++data;
        --size;
        return result;
    }

    // returns 0..7 with exponential distribution
    uint32_t exp_prob() {
        uint32_t r = 0;
        uint8_t value = byte();
        while (value >>= 1) {
            r++;
        }
        return 7 - r;
    }

    [[maybe_unused]] uint32_t range(uint32_t min, uint32_t limit) {
        return range(limit - min) + min;
    }

    uint32_t range(uint32_t limit) {
        return std::min(static_cast<uint32_t>(byte()) * limit / 256u, limit - 1);
    }

    Context& ident(char c = 'x') {
        buffer << genIdent(exp_prob(), c);
        return *this;
    }
    Context& raw(std::string_view s) {
        buffer << s;
        return *this;
    }

    template <typename... Args>
    Context& options(Args&&... args) {
        constexpr uint32_t N = sizeof...(Args);
        static_assert(N > 0);
        size_t x = range(N);
        std::array<fn, N>{static_cast<fn>(args)...}[x](*this);
        return *this;
    }

    template <int min, int max>
    Context& repeat(fn func, fn separator = nullptr) {
#ifndef EXPLICIT_RANGE
        for (uint32_t i = 0;; i++) {
            if (i >= min) {
                if (i >= max) {
                    break;
                }
                uint32_t pred = range(2);
                if (pred == 0) {
                    break;
                }
            }

            if (i > 0 && separator) {
                separator(*this);
            }
            func(*this);
        }

#else
        uint32_t x = range(min, max + 1);
        for (uint32_t i = 0; i < x; i++) {
            if (i > 0 && separator) {
                separator(*this);
            }
            func(*this);
        }
#endif
        return *this;
    }
    Context& optional(fn func) { return repeat<0, 1>(func, nullptr); }
    Context& once(fn func) {
        func(*this);
        return *this;
    }
};

enum class Keyword {
    kw_true,
    kw_false,
    Last = kw_false,
};
constexpr std::string_view keywords[] = {
    "true",
    "false",
};
static_assert(static_cast<int>(Keyword::Last) + 1 == std::size(keywords));

constexpr int maxArguments = 3;
constexpr int maxFields = 4;
constexpr int maxAttributes = 2;

void empty(Context&) {}

template <Keyword kw>
void keyword(Context& ctx) {
    ctx.raw(keywords[static_cast<int>(kw)]);
}
template <char ch0, char ch1 = 0>
void character(Context& ctx) {
    if constexpr (ch1 != 0) {
        const char value[] = {ch0, ch1};
        ctx.raw(std::string_view(value, 2));
    } else {
        const char value = ch0;
        ctx.raw(std::string_view(&value, 1));
    }
}

void decimal_literal(Context& ctx) {
    ctx.raw("123");
}
void hex_literal(Context& ctx) {
    ctx.raw("0xFFFF");
}
void float_literal(Context& ctx) {
    ctx.raw("3.14159");
}
void bool_literal(Context& ctx) {
    ctx.options(&keyword<Keyword::kw_true>, &keyword<Keyword::kw_false>);
}
void comma(Context& ctx) {
    ctx.raw(",");
}
void semicolon(Context& ctx) {
    ctx.raw("; ");
}

void literal(Context& ctx) {
    ctx.options(&decimal_literal, &float_literal, &hex_literal, &bool_literal);
}

void expression(Context& ctx);

void type(Context& ctx);
void type_cast(Context& ctx) {
    ctx.once(&type).raw("(").once(&expression).raw(")");
}

void func_call(Context& ctx) {
    ctx.ident('f').raw("(").repeat<0, maxArguments>(&expression, &comma).raw(")");
}

void parens(Context& ctx) {
    ctx.raw("(").once(&expression).raw(")");
}

void primary_expression(Context& ctx) {
    ctx.options(
        &literal, [](Context& ctx1) { ctx1.ident('v'); }, &func_call, &type_cast, &parens);
}

void unary_expression(Context& ctx);

void unary_op(Context& ctx) {
    ctx.options(&character<'!'>, &character<'&'>, &character<'*'>, &character<'-'>, &character<'~'>)
        .once(&unary_expression);
}

void swizzle(Context& ctx) {
    ctx.options([](Context& ctx1) { ctx1.raw(".g"); }, [](Context& ctx1) { ctx1.raw(".xx"); },
                [](Context& ctx1) { ctx1.raw(".bgr"); }, [](Context& ctx1) { ctx1.raw(".argb"); },
                [](Context& ctx1) { ctx1.raw(".w"); }, [](Context& ctx1) { ctx1.raw(".yy"); },
                [](Context& ctx1) { ctx1.raw(".xyy"); }, [](Context& ctx1) { ctx1.raw(".wzyx"); });
}

void primary_postfix(Context& ctx) {
    ctx.options([](Context& ctx1) { ctx1.raw(".").ident(); },
                [](Context& ctx1) { ctx1.raw("[").once(&expression).raw("]"); });
    ctx.optional(&swizzle);
}

void primary_expression_postfix(Context& ctx) {
    ctx.once(&primary_expression).repeat<0, 2>(&primary_postfix);
}

void unary_expression(Context& ctx) {
    ctx.options(&primary_expression_postfix, &unary_op);
}

void bitwise_op(Context& ctx) {
    ctx.optional([](Context& ctx1) {
        ctx1.options(&character<'&'>, &character<'^'>, &character<'|'>).once(&unary_expression);
    });
}
void mul_op(Context& ctx) {
    ctx.options(&character<'%'>, &character<'/'>, &character<'*'>).once(&unary_expression);
}
void add_op(Context& ctx) {
    ctx.options(&character<'+'>, &character<'-'>).once(&unary_expression).optional(&mul_op);
}
void multiplicative_op(Context& ctx) {
    ctx.optional(&mul_op);
    ctx.optional(&add_op);
}
void shift_op(Context& ctx) {
    ctx.options(&character<'<', '<'>, &character<'>', '>'>);
    unary_expression(ctx);
}
void relational_op(Context& ctx) {
    ctx.options(&multiplicative_op, &shift_op);
    ctx.options(&character<'>', '='>, &character<'>'>, &character<'<', '='>, &character<'<'>,
                &character<'=', '='>, &character<'!', '='>)
        .once(&unary_expression);
}

void expression(Context& ctx) {
    unary_expression(ctx);
    ctx.options(&bitwise_op, &relational_op);
}
void attribute(Context& ctx) {
    ctx.raw("@");
    ctx.options(
        [](Context& ctx1) { ctx1.raw("align(").once(&expression).raw(")"); },
        [](Context& ctx1) { ctx1.raw("binding(").once(&expression).raw(")"); },
        [](Context& ctx1) { ctx1.raw("builtin(").once(&expression).raw(")"); },
        [](Context& ctx1) { ctx1.raw("group(").once(&expression).raw(")"); },
        [](Context& ctx1) { ctx1.raw("id(").once(&expression).raw(")"); },
        [](Context& ctx1) { ctx1.raw("interpolate(").once(&expression).raw(")"); },
        [](Context& ctx1) { ctx1.raw("location(").once(&expression).raw(")"); },
        [](Context& ctx1) { ctx1.raw("size(").once(&expression).raw(")"); },
        [](Context& ctx1) { ctx1.raw("workgroup_size(").once(&expression).raw(")"); },
        [](Context& ctx1) { ctx1.raw("compute"); }, [](Context& ctx1) { ctx1.raw("fragment"); },
        [](Context& ctx1) { ctx1.raw("invariant"); }, [](Context& ctx1) { ctx1.raw("must_use"); },
        [](Context& ctx1) { ctx1.raw("vertex"); });
    ctx.raw(" ");
}
void attributes(Context& ctx) {
    ctx.repeat<0, maxAttributes>(&attribute);
}

void return_statement(Context& ctx) {
    ctx.raw("return ").optional(&expression).raw(";");
}
void discard(Context& ctx) {
    ctx.raw("discard");
    ctx.raw(";");
}
void continue_statement(Context& ctx) {
    ctx.raw("continue");
    ctx.raw(";");
}
void break_statement(Context& ctx) {
    ctx.raw("break");
    ctx.raw(";");
}
void const_assert(Context& ctx) {
    ctx.raw("const_assert ").once(&expression);
}

void statement(Context& ctx);

void compound_statement(Context& ctx) {
    ctx.raw("{").once(&statement).raw("}");
}
void type(Context& ctx) {
    ctx.options([](Context& ctx1) { ctx1.ident('t'); }, [](Context& ctx1) { ctx1.raw("i32"); },
                [](Context& ctx1) { ctx1.raw("u32"); }, [](Context& ctx1) { ctx1.raw("f32"); },
                [](Context& ctx1) { ctx1.raw("f16"); }, [](Context& ctx1) { ctx1.raw("bool"); },
                [](Context& ctx1) { ctx1.raw("vec2<").once(&type).raw(">"); },
                [](Context& ctx1) { ctx1.raw("vec3<").once(&type).raw(">"); },
                [](Context& ctx1) { ctx1.raw("vec4<").once(&type).raw(">"); });
}
void typed(Context& ctx) {
    ctx.raw(":").once(&type);
}

void local_const(Context& ctx) {
    ctx.raw("const ").ident('v').optional(&typed).raw("=").once(&expression);
}
void local_let(Context& ctx) {
    ctx.raw("let ").ident('v').optional(&typed).raw("=").once(&expression);
}
void var_assign(Context& ctx) {
    ctx.options([](Context& ctx1) { ctx1.ident('v').raw("=").once(&expression); },
                [](Context& ctx1) { ctx1.ident('v').raw("++"); },
                [](Context& ctx1) { ctx1.ident('v').raw("--"); },
                [](Context& ctx1) { ctx1.raw("_").raw("=").once(&expression); });
}

void if_statement(Context& ctx);

void else_branch(Context& ctx) {
    ctx.raw("else ");
    ctx.options(&compound_statement, &if_statement);
}

void if_statement(Context& ctx) {
    ctx.raw("if ").once(&expression).once(&compound_statement).optional(&else_branch);
}
void while_statement(Context& ctx) {
    ctx.raw("while ").once(&expression).once(&compound_statement);
}
void for_statement(Context& ctx) {
    ctx.raw("for (")
        .options(&var_assign, &local_let, &empty)
        .raw(";")
        .optional(&expression)
        .raw(";")
        .once(&var_assign)
        .raw(")")
        .once(&compound_statement);
}

void statement(Context& ctx) {
    ctx.options(&return_statement, &discard, &local_const, &local_let, &var_assign, &if_statement,
                &while_statement, &for_statement, &continue_statement, &break_statement,
                &const_assert, &compound_statement);
}

void argument(Context& ctx) {
    ctx.once(&attributes).ident('v').raw(":").ident('t');
}

void argument_list(Context& ctx) {
    ctx.repeat<0, maxArguments>(&argument, &comma);
}
void return_type(Context& ctx) {
    ctx.raw("->").ident('t');
}
void global_fn(Context& ctx) {
    ctx.once(&attributes)
        .raw("fn ")
        .ident('f')
        .raw("(")
        .once(&argument_list)
        .raw(")")
        .optional(&return_type)
        .raw("{")
        .once(&statement)
        .raw("}");
}
void global_var(Context& ctx) {
    ctx.once(&attributes).raw("var ").ident('v').optional(&typed).raw("=").once(&expression);
}
void global_const(Context& ctx) {
    ctx.raw("const ").ident('v').optional(&typed).raw("=").once(&expression);
}
void field(Context& ctx) {
    ctx.once(&attributes).ident('v').raw(":").ident('t');
}
void field_list(Context& ctx) {
    ctx.repeat<0, maxFields>(&field, &comma);
}
void global_struct(Context& ctx) {
    ctx.raw("struct ").ident('t').raw("{").once(&field_list).raw("}");
}

void global_decl(Context& ctx) {
    ctx.options(&global_fn, &global_var, &global_const, &global_struct, &const_assert);
}
}  // namespace

void WGSLSource(std::stringstream& buffer, const uint8_t* data, size_t size) {
    Context ctx{buffer, data, size};
    ctx.repeat<2, 5>(&global_decl, &semicolon);
}

}  // namespace tint::fuzzers::structure_fuzzer
