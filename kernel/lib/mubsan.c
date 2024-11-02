#include <stdint.h>

extern int mubsan_log(const char* format, ...);

const char* const mubsan_type_check_kinds[] = {
    "load of",
    "store to",
    "reference binding to",
    "member access within",
    "member call on",
    "constructor call on",
    "downcast of",
    "downcast of",
    "upcast of",
    "cast to virtual base of",
    "_Nonnull binding to",
    "dynamic operation on"
};

typedef struct {
    const char* file;
    uint32_t line;
    uint32_t col;
} mubsan_source_location;

typedef struct {
    uint16_t kind;
    uint16_t info;
    char name[];
} mubsan_type_description;

typedef struct {
    mubsan_source_location loc;
    mubsan_type_description* type;
    uint8_t alignment;
    uint8_t check_kind;
} mubsan_type_mismatch_info_v1;

typedef struct {
    mubsan_source_location loc;
    mubsan_type_description* type;
} mubsan_overflow;

typedef struct {
    mubsan_source_location loc;
} mubsan_pointer_overflow;

typedef struct {
    mubsan_source_location loc;
    mubsan_type_description* array_type;
    mubsan_type_description* index_type;
} mubsan_out_of_bounds;

typedef struct {
    mubsan_source_location loc;
    //mubsan_source_location attr_loc;
    //int arg_index;
} mubsan_not_null_arg;

typedef struct {
    mubsan_source_location loc;
    mubsan_type_description* type;
} mubsan_invalid_value;

typedef struct {
    mubsan_source_location loc;
    mubsan_type_description* lhs_type;
    mubsan_type_description* rhs_type;
} mubsan_shift_out_of_bounds;

typedef struct {
    mubsan_source_location loc;
} mubsan_unreachable;

typedef struct {
    mubsan_source_location loc;
    mubsan_type_description* expected_type;
    mubsan_type_description* actual_type;
    void *function;
} mubsan_function_type_mismatch;

void __ubsan_handle_type_mismatch_v1(mubsan_type_mismatch_info_v1* data, uintptr_t ptr) {
    const char* reason = "type mismatch";

    if (ptr == 0) {
        reason = "dereference of a null pointer";
    } else if (data->alignment && (ptr & (data->alignment - 1))) {
        reason = "use of a misaligned pointer";
    }

    mubsan_log("%s:%d:%d: %s, %s type %s at alignment %d at address 0x%x\n",
               data->loc.file,
               data->loc.line,
               data->loc.col,
               reason,
               mubsan_type_check_kinds[data->check_kind],
               data->type->name,
               data->alignment,
               ptr);
}
void __ubsan_handle_add_overflow(mubsan_overflow* data, uintptr_t lhs, uintptr_t rhs) {
    mubsan_log("%s:%d:%d: addition overflow, for type %s, expression %d + %d\n",
               data->loc.file,
               data->loc.line,
               data->loc.col,
               data->type->name,
               lhs,
               rhs);
}
void __ubsan_handle_sub_overflow(mubsan_overflow* data, uintptr_t lhs, uintptr_t rhs) {
    mubsan_log("%s:%d:%d: subtraction overflow, for type %s, expression %d - %d\n",
               data->loc.file,
               data->loc.line,
               data->loc.col,
               data->type->name,
               lhs,
               rhs);
}
void __ubsan_handle_mul_overflow(mubsan_overflow* data, uintptr_t lhs, uintptr_t rhs) {
    mubsan_log("%s:%d:%d: multiplication overflow, for type %s, expression %d * %d\n",
               data->loc.file,
               data->loc.line,
               data->loc.col,
               data->type->name,
               lhs,
               rhs);
}
void __ubsan_handle_negate_overflow(mubsan_overflow* data, uintptr_t val) {
    mubsan_log("%s:%d:%d: negate overflow, for type %s, value %d\n",
               data->loc.file,
               data->loc.line,
               data->loc.col,
               data->type->name,
               val);
}
void __ubsan_handle_divrem_overflow(mubsan_overflow* data, uintptr_t lhs, uintptr_t rhs) {
    mubsan_log("%s:%d:%d: division overflow, for type %s, expression %d / %d\n",
               data->loc.file,
               data->loc.line,
               data->loc.col,
               data->type->name,
               lhs,
               rhs);
}
void __ubsan_handle_pointer_overflow(mubsan_pointer_overflow* data, uintptr_t base, uintptr_t result) {
    mubsan_log("%s:%d:%d: pointer overflow, base 0x%x, result 0x%x\n",
               data->loc.file,
               data->loc.line,
               data->loc.col,
               base,
               result);
}
void __ubsan_handle_out_of_bounds(mubsan_out_of_bounds* data, uintptr_t index) {
    mubsan_log("%s:%d:%d: array out of bounds, for type %s, by index type %s %d\n",
               data->loc.file,
               data->loc.line,
               data->loc.col,
               data->array_type->name,
               data->index_type->name,
               index);
}
void __ubsan_handle_nonnull_arg(mubsan_not_null_arg* data) {
    mubsan_log("%s:%d:%d: not-null argument is null\n",
               data->loc.file,
               data->loc.line,
               data->loc.col);
}
void __ubsan_handle_load_invalid_value(mubsan_invalid_value* data, uintptr_t val) {
    mubsan_log("%s:%d:%d: load of invalid value, for type %s, value %d\n",
               data->loc.file,
               data->loc.line,
               data->loc.col,
               data->type->name,
               val);
}
void __ubsan_handle_shift_out_of_bounds(mubsan_shift_out_of_bounds* data, uintptr_t lhs, uintptr_t rhs) {
    mubsan_log("%s:%d:%d: shift out of bounds, of type %s and %s, value %d and %d\n",
               data->loc.file,
               data->loc.line,
               data->loc.col,
               data->lhs_type->name,
               data->rhs_type->name,
               lhs,
               rhs);
}
void __ubsan_handle_builtin_unreachable(mubsan_unreachable* data) {
    mubsan_log("%s:%d:%d: unreachable code was reached\n",
               data->loc.file,
               data->loc.line,
               data->loc.col);
}
void __ubsan_handle_function_type_mismatch(mubsan_function_type_mismatch* data) {
    mubsan_log("%s:%d:%d: function type mismatch, expected %s but got %s at address %x\n",
               data->loc.file,
               data->loc.line,
               data->loc.col,
               data->expected_type->name,
               data->actual_type->name,
               data->function);
}