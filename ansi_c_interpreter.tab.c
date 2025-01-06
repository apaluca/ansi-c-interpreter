/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "ansi_c_interpreter.y"

#  include <stdio.h>
#  include <stdlib.h>
#  include <unistd.h>
#  include "ansi_c_interpreter.h"
int yylex(void);
int yyparse(void);
extern char yytext[];
extern int column;
struct ast *root = NULL;
enum value_type current_type = TYPE_INT;

#line 84 "ansi_c_interpreter.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "ansi_c_interpreter.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_CONSTANT = 3,                   /* CONSTANT  */
  YYSYMBOL_IDENTIFIER = 4,                 /* IDENTIFIER  */
  YYSYMBOL_STRING_LITERAL = 5,             /* STRING_LITERAL  */
  YYSYMBOL_LE_OP = 6,                      /* LE_OP  */
  YYSYMBOL_GE_OP = 7,                      /* GE_OP  */
  YYSYMBOL_EQ_OP = 8,                      /* EQ_OP  */
  YYSYMBOL_NE_OP = 9,                      /* NE_OP  */
  YYSYMBOL_INT = 10,                       /* INT  */
  YYSYMBOL_FLOAT = 11,                     /* FLOAT  */
  YYSYMBOL_DOUBLE = 12,                    /* DOUBLE  */
  YYSYMBOL_VOID = 13,                      /* VOID  */
  YYSYMBOL_IF = 14,                        /* IF  */
  YYSYMBOL_ELSE = 15,                      /* ELSE  */
  YYSYMBOL_WHILE = 16,                     /* WHILE  */
  YYSYMBOL_RETURN = 17,                    /* RETURN  */
  YYSYMBOL_UMINUS = 18,                    /* UMINUS  */
  YYSYMBOL_19_ = 19,                       /* '('  */
  YYSYMBOL_20_ = 20,                       /* ')'  */
  YYSYMBOL_21_ = 21,                       /* ','  */
  YYSYMBOL_22_ = 22,                       /* '-'  */
  YYSYMBOL_23_ = 23,                       /* '*'  */
  YYSYMBOL_24_ = 24,                       /* '/'  */
  YYSYMBOL_25_ = 25,                       /* '%'  */
  YYSYMBOL_26_ = 26,                       /* '+'  */
  YYSYMBOL_27_ = 27,                       /* '<'  */
  YYSYMBOL_28_ = 28,                       /* '>'  */
  YYSYMBOL_29_ = 29,                       /* '='  */
  YYSYMBOL_30_ = 30,                       /* ';'  */
  YYSYMBOL_31_ = 31,                       /* '{'  */
  YYSYMBOL_32_ = 32,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 33,                  /* $accept  */
  YYSYMBOL_primary_expression = 34,        /* primary_expression  */
  YYSYMBOL_argument_expression_list = 35,  /* argument_expression_list  */
  YYSYMBOL_function_identifier = 36,       /* function_identifier  */
  YYSYMBOL_postfix_expression = 37,        /* postfix_expression  */
  YYSYMBOL_unary_expression = 38,          /* unary_expression  */
  YYSYMBOL_cast_expression = 39,           /* cast_expression  */
  YYSYMBOL_multiplicative_expression = 40, /* multiplicative_expression  */
  YYSYMBOL_additive_expression = 41,       /* additive_expression  */
  YYSYMBOL_relational_expression = 42,     /* relational_expression  */
  YYSYMBOL_equality_expression = 43,       /* equality_expression  */
  YYSYMBOL_assignment_expression = 44,     /* assignment_expression  */
  YYSYMBOL_expression = 45,                /* expression  */
  YYSYMBOL_declaration = 46,               /* declaration  */
  YYSYMBOL_init_declarator = 47,           /* init_declarator  */
  YYSYMBOL_type_specifier = 48,            /* type_specifier  */
  YYSYMBOL_statement = 49,                 /* statement  */
  YYSYMBOL_compound_statement = 50,        /* compound_statement  */
  YYSYMBOL_block_item_list = 51,           /* block_item_list  */
  YYSYMBOL_block_item = 52,                /* block_item  */
  YYSYMBOL_expression_statement = 53,      /* expression_statement  */
  YYSYMBOL_selection_statement = 54,       /* selection_statement  */
  YYSYMBOL_iteration_statement = 55,       /* iteration_statement  */
  YYSYMBOL_jump_statement = 56,            /* jump_statement  */
  YYSYMBOL_translation_unit = 57,          /* translation_unit  */
  YYSYMBOL_external_declaration = 58,      /* external_declaration  */
  YYSYMBOL_function_definition = 59,       /* function_definition  */
  YYSYMBOL_parameter_list = 60,            /* parameter_list  */
  YYSYMBOL_parameter_declaration = 61      /* parameter_declaration  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  68
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   194

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  33
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  29
/* YYNRULES -- Number of rules.  */
#define YYNRULES  68
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  116

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   273


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    25,     2,     2,
      19,    20,    23,    26,    21,    22,     2,    24,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    30,
      27,    29,    28,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    31,     2,    32,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    51,    51,    61,    77,    81,    88,    89,    94,    98,
      99,   101,   106,   110,   119,   120,   125,   126,   128,   130,
     135,   136,   138,   143,   144,   146,   148,   150,   155,   156,
     158,   163,   164,   174,   175,   180,   185,   201,   223,   224,
     225,   226,   230,   231,   232,   233,   234,   238,   239,   243,
     244,   249,   250,   254,   255,   259,   261,   266,   271,   272,
     276,   307,   341,   342,   343,   347,   355,   357,   362
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "CONSTANT",
  "IDENTIFIER", "STRING_LITERAL", "LE_OP", "GE_OP", "EQ_OP", "NE_OP",
  "INT", "FLOAT", "DOUBLE", "VOID", "IF", "ELSE", "WHILE", "RETURN",
  "UMINUS", "'('", "')'", "','", "'-'", "'*'", "'/'", "'%'", "'+'", "'<'",
  "'>'", "'='", "';'", "'{'", "'}'", "$accept", "primary_expression",
  "argument_expression_list", "function_identifier", "postfix_expression",
  "unary_expression", "cast_expression", "multiplicative_expression",
  "additive_expression", "relational_expression", "equality_expression",
  "assignment_expression", "expression", "declaration", "init_declarator",
  "type_specifier", "statement", "compound_statement", "block_item_list",
  "block_item", "expression_statement", "selection_statement",
  "iteration_statement", "jump_statement", "translation_unit",
  "external_declaration", "function_definition", "parameter_list",
  "parameter_declaration", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-37)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-9)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     137,   -37,    13,   -37,   -37,   -37,   -37,   -37,    16,    20,
       7,   161,    87,   -37,    91,    29,    44,   -37,   -37,   -37,
     165,     5,   151,    15,   -37,   -12,   -37,    62,   -37,   -37,
     -37,   -37,   -37,   -37,    57,   -37,   -37,    87,    87,   -37,
       0,    21,    52,   -37,   -37,   -37,   -37,    73,   -37,   114,
     -37,    87,   172,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,   -37,   -15,    48,   -37,   -37,
      60,    77,   -37,   -37,    87,    53,   -37,   -37,   -37,   -37,
      94,   -37,   -37,   -37,   -37,   165,   165,     5,     5,     5,
       5,   151,   151,   -37,   150,    87,   -37,     3,     3,   -37,
     -37,    87,    82,   149,   -37,   -37,    74,   -37,   -37,   -37,
      85,   150,     3,   -37,   -37,   -37
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     3,     2,     4,    39,    40,    41,    38,     0,     0,
       0,     0,     0,    53,     0,     9,     0,    12,    14,    16,
      20,    23,    28,    31,    33,     0,    63,     0,    64,    42,
      43,    44,    45,    46,     0,    60,    62,     0,     0,    58,
       0,     0,     0,     9,    13,    47,    51,     0,    52,     0,
      49,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    54,    36,     0,     1,    61,
       0,     0,    59,     5,     0,    36,    48,    50,    32,    10,
       0,     6,    17,    18,    19,    22,    21,    26,    27,    24,
      25,    29,    30,    34,     0,     0,    35,     0,     0,    15,
      11,     0,     0,     0,    66,    37,    55,    57,     7,    68,
       0,     0,     0,    65,    67,    56
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -37,   -10,   -37,   -37,   -37,   -37,     1,    78,   126,   119,
     -37,   -36,   101,    -9,   -37,   -11,   -13,   -17,   -37,    71,
     -37,   -37,   -37,   -37,   -37,    95,   -37,   -37,    26
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,    15,    80,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    67,    27,    28,    29,    49,    50,
      30,    31,    32,    33,    34,    35,    36,   103,   104
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      42,    48,    43,    47,    94,    46,     1,     2,     3,    64,
       1,     2,     3,    44,    95,    78,    81,     8,    65,     9,
      10,    64,    11,    62,    63,    12,    11,    56,    93,    12,
      72,    57,    -8,    13,    14,    37,    48,    39,    47,    38,
      46,    73,    64,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    82,    83,    84,    68,    51,   105,
       1,     2,     3,    52,    43,   108,    66,     4,     5,     6,
       7,     8,    74,     9,    10,    99,    11,    75,    96,    12,
      97,    64,    95,   102,   106,   107,   109,    13,    14,   112,
       1,     2,     3,   113,     1,     2,     3,    98,    64,   115,
     102,     4,     5,     6,     7,     8,    11,     9,    10,    12,
      11,    40,    41,    12,   100,   101,    14,     1,     2,     3,
      77,    13,    14,    45,     4,     5,     6,     7,     8,    69,
       9,    10,     0,    11,    85,    86,    12,   114,    70,    71,
       1,     2,     3,     0,    13,    14,    76,     4,     5,     6,
       7,     8,     0,     9,    10,     0,    11,    58,    59,    12,
       4,     5,     6,     7,     1,     2,     3,    13,    14,   110,
     111,     4,     5,     6,     7,     1,     2,     3,    60,    61,
      11,    91,    92,    12,    87,    88,    89,    90,    53,    54,
      55,    11,    79,     0,    12
};

static const yytype_int8 yycheck[] =
{
      11,    14,    12,    14,    19,    14,     3,     4,     5,    21,
       3,     4,     5,    12,    29,    51,    52,    14,    30,    16,
      17,    21,    19,     8,     9,    22,    19,    22,    64,    22,
      30,    26,    19,    30,    31,    19,    49,    30,    49,    19,
      49,    20,    21,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    53,    54,    55,     0,    29,    95,
       3,     4,     5,    19,    74,   101,     4,    10,    11,    12,
      13,    14,    20,    16,    17,    74,    19,     4,    30,    22,
      20,    21,    29,    94,    97,    98,     4,    30,    31,    15,
       3,     4,     5,   110,     3,     4,     5,    20,    21,   112,
     111,    10,    11,    12,    13,    14,    19,    16,    17,    22,
      19,    10,    11,    22,    20,    21,    31,     3,     4,     5,
      49,    30,    31,    32,    10,    11,    12,    13,    14,    34,
      16,    17,    -1,    19,    56,    57,    22,   111,    37,    38,
       3,     4,     5,    -1,    30,    31,    32,    10,    11,    12,
      13,    14,    -1,    16,    17,    -1,    19,     6,     7,    22,
      10,    11,    12,    13,     3,     4,     5,    30,    31,    20,
      21,    10,    11,    12,    13,     3,     4,     5,    27,    28,
      19,    62,    63,    22,    58,    59,    60,    61,    23,    24,
      25,    19,    20,    -1,    22
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,    10,    11,    12,    13,    14,    16,
      17,    19,    22,    30,    31,    34,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    48,    49,    50,
      53,    54,    55,    56,    57,    58,    59,    19,    19,    30,
      45,    45,    48,    34,    39,    32,    46,    48,    49,    51,
      52,    29,    19,    23,    24,    25,    22,    26,     6,     7,
      27,    28,     8,     9,    21,    30,     4,    47,     0,    58,
      45,    45,    30,    20,    20,     4,    32,    52,    44,    20,
      35,    44,    39,    39,    39,    40,    40,    41,    41,    41,
      41,    42,    42,    44,    19,    29,    30,    20,    20,    39,
      20,    21,    48,    60,    61,    44,    49,    49,    44,     4,
      20,    21,    15,    50,    61,    49
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    33,    34,    34,    34,    34,    35,    35,    36,    37,
      37,    37,    38,    38,    39,    39,    40,    40,    40,    40,
      41,    41,    41,    42,    42,    42,    42,    42,    43,    43,
      43,    44,    44,    45,    45,    46,    47,    47,    48,    48,
      48,    48,    49,    49,    49,    49,    49,    50,    50,    51,
      51,    52,    52,    53,    53,    54,    54,    55,    56,    56,
      57,    57,    58,    58,    58,    59,    60,    60,    61
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     1,     3,     1,     1,
       3,     4,     1,     2,     1,     4,     1,     3,     3,     3,
       1,     3,     3,     1,     3,     3,     3,     3,     1,     3,
       3,     1,     3,     1,     3,     3,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     1,
       2,     1,     1,     1,     2,     5,     7,     5,     2,     3,
       1,     2,     1,     1,     1,     6,     1,     3,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* primary_expression: IDENTIFIER  */
#line 51 "ansi_c_interpreter.y"
                        { 
        debug_print("primary_expression", "Processing IDENTIFIER");
        if ((yyvsp[0].s) == NULL) {
            printf("ERROR: NULL identifier\n");
        } else {
            printf("DEBUG: Creating ref for identifier: %s\n", (yyvsp[0].s)->name);
        }
        (yyval.a) = newref((yyvsp[0].s)); 
        debug_print("primary_expression", "Created reference node");
    }
#line 1234 "ansi_c_interpreter.tab.c"
    break;

  case 3: /* primary_expression: CONSTANT  */
#line 61 "ansi_c_interpreter.y"
                       { 
        debug_print("primary_expression", "Processing CONSTANT");
        printf("DEBUG: Creating number node with value: ");
        switch((yyvsp[0].v).type) {
            case TYPE_INT:
                printf("%d\n", (yyvsp[0].v).value.i_val);
                break;
            case TYPE_FLOAT:
                printf("%f\n", (yyvsp[0].v).value.f_val);
                break;
            case TYPE_DOUBLE:
                printf("%g\n", (yyvsp[0].v).value.d_val);
                break;
        }
        (yyval.a) = newnum((yyvsp[0].v).type, (yyvsp[0].v).value);
    }
#line 1255 "ansi_c_interpreter.tab.c"
    break;

  case 4: /* primary_expression: STRING_LITERAL  */
#line 77 "ansi_c_interpreter.y"
                       { 
        debug_print("primary_expression", "Processing STRING_LITERAL (not implemented)");
        (yyval.a) = NULL; 
    }
#line 1264 "ansi_c_interpreter.tab.c"
    break;

  case 5: /* primary_expression: '(' expression ')'  */
#line 81 "ansi_c_interpreter.y"
                            { 
        debug_print("primary_expression", "Processing parenthesized expression");
        (yyval.a) = (yyvsp[-1].a); 
    }
#line 1273 "ansi_c_interpreter.tab.c"
    break;

  case 6: /* argument_expression_list: assignment_expression  */
#line 88 "ansi_c_interpreter.y"
                               { (yyval.a) = (yyvsp[0].a); }
#line 1279 "ansi_c_interpreter.tab.c"
    break;

  case 7: /* argument_expression_list: argument_expression_list ',' assignment_expression  */
#line 90 "ansi_c_interpreter.y"
        { (yyval.a) = newast('L', (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1285 "ansi_c_interpreter.tab.c"
    break;

  case 8: /* function_identifier: IDENTIFIER  */
#line 94 "ansi_c_interpreter.y"
                    { (yyval.s) = (yyvsp[0].s); }
#line 1291 "ansi_c_interpreter.tab.c"
    break;

  case 9: /* postfix_expression: primary_expression  */
#line 98 "ansi_c_interpreter.y"
                            { (yyval.a) = (yyvsp[0].a); }
#line 1297 "ansi_c_interpreter.tab.c"
    break;

  case 10: /* postfix_expression: function_identifier '(' ')'  */
#line 100 "ansi_c_interpreter.y"
        { (yyval.a) = newcall((yyvsp[-2].s), NULL); }
#line 1303 "ansi_c_interpreter.tab.c"
    break;

  case 11: /* postfix_expression: function_identifier '(' argument_expression_list ')'  */
#line 102 "ansi_c_interpreter.y"
        { (yyval.a) = newcall((yyvsp[-3].s), (yyvsp[-1].a)); }
#line 1309 "ansi_c_interpreter.tab.c"
    break;

  case 12: /* unary_expression: postfix_expression  */
#line 106 "ansi_c_interpreter.y"
                            { 
        debug_print("unary_expression", "Processing postfix expression");
        (yyval.a) = (yyvsp[0].a); 
    }
#line 1318 "ansi_c_interpreter.tab.c"
    break;

  case 13: /* unary_expression: '-' cast_expression  */
#line 111 "ansi_c_interpreter.y"
        { 
            debug_print("unary_expression", "Processing unary minus");
            printf("DEBUG: Creating unary minus node\n");
            (yyval.a) = newast('M', (yyvsp[0].a), NULL); 
        }
#line 1328 "ansi_c_interpreter.tab.c"
    break;

  case 14: /* cast_expression: unary_expression  */
#line 119 "ansi_c_interpreter.y"
                          { (yyval.a) = (yyvsp[0].a); }
#line 1334 "ansi_c_interpreter.tab.c"
    break;

  case 15: /* cast_expression: '(' type_specifier ')' cast_expression  */
#line 121 "ansi_c_interpreter.y"
        { (yyval.a) = (yyvsp[0].a); /* Handle type casting later */ }
#line 1340 "ansi_c_interpreter.tab.c"
    break;

  case 16: /* multiplicative_expression: cast_expression  */
#line 125 "ansi_c_interpreter.y"
                         { (yyval.a) = (yyvsp[0].a); }
#line 1346 "ansi_c_interpreter.tab.c"
    break;

  case 17: /* multiplicative_expression: multiplicative_expression '*' cast_expression  */
#line 127 "ansi_c_interpreter.y"
        { (yyval.a) = newast('*', (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1352 "ansi_c_interpreter.tab.c"
    break;

  case 18: /* multiplicative_expression: multiplicative_expression '/' cast_expression  */
#line 129 "ansi_c_interpreter.y"
        { (yyval.a) = newast('/', (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1358 "ansi_c_interpreter.tab.c"
    break;

  case 19: /* multiplicative_expression: multiplicative_expression '%' cast_expression  */
#line 131 "ansi_c_interpreter.y"
        { (yyval.a) = newast('%', (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1364 "ansi_c_interpreter.tab.c"
    break;

  case 20: /* additive_expression: multiplicative_expression  */
#line 135 "ansi_c_interpreter.y"
                                   { (yyval.a) = (yyvsp[0].a); }
#line 1370 "ansi_c_interpreter.tab.c"
    break;

  case 21: /* additive_expression: additive_expression '+' multiplicative_expression  */
#line 137 "ansi_c_interpreter.y"
        { (yyval.a) = newast('+', (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1376 "ansi_c_interpreter.tab.c"
    break;

  case 22: /* additive_expression: additive_expression '-' multiplicative_expression  */
#line 139 "ansi_c_interpreter.y"
        { (yyval.a) = newast('-', (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1382 "ansi_c_interpreter.tab.c"
    break;

  case 23: /* relational_expression: additive_expression  */
#line 143 "ansi_c_interpreter.y"
                             { (yyval.a) = (yyvsp[0].a); }
#line 1388 "ansi_c_interpreter.tab.c"
    break;

  case 24: /* relational_expression: relational_expression '<' additive_expression  */
#line 145 "ansi_c_interpreter.y"
        { (yyval.a) = newcmp(2, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1394 "ansi_c_interpreter.tab.c"
    break;

  case 25: /* relational_expression: relational_expression '>' additive_expression  */
#line 147 "ansi_c_interpreter.y"
        { (yyval.a) = newcmp(1, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1400 "ansi_c_interpreter.tab.c"
    break;

  case 26: /* relational_expression: relational_expression LE_OP additive_expression  */
#line 149 "ansi_c_interpreter.y"
        { (yyval.a) = newcmp(6, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1406 "ansi_c_interpreter.tab.c"
    break;

  case 27: /* relational_expression: relational_expression GE_OP additive_expression  */
#line 151 "ansi_c_interpreter.y"
        { (yyval.a) = newcmp(5, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1412 "ansi_c_interpreter.tab.c"
    break;

  case 28: /* equality_expression: relational_expression  */
#line 155 "ansi_c_interpreter.y"
                               { (yyval.a) = (yyvsp[0].a); }
#line 1418 "ansi_c_interpreter.tab.c"
    break;

  case 29: /* equality_expression: equality_expression EQ_OP relational_expression  */
#line 157 "ansi_c_interpreter.y"
        { (yyval.a) = newcmp(4, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1424 "ansi_c_interpreter.tab.c"
    break;

  case 30: /* equality_expression: equality_expression NE_OP relational_expression  */
#line 159 "ansi_c_interpreter.y"
        { (yyval.a) = newcmp(3, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1430 "ansi_c_interpreter.tab.c"
    break;

  case 31: /* assignment_expression: equality_expression  */
#line 163 "ansi_c_interpreter.y"
                             { (yyval.a) = (yyvsp[0].a); }
#line 1436 "ansi_c_interpreter.tab.c"
    break;

  case 32: /* assignment_expression: primary_expression '=' assignment_expression  */
#line 165 "ansi_c_interpreter.y"
        { 
            if ((yyvsp[-2].a)->nodetype == 'N')  // Must be an identifier reference
                (yyval.a) = newasgn(((struct symref *)(yyvsp[-2].a))->s, (yyvsp[0].a));
            else
                yyerror("Invalid assignment target");
        }
#line 1447 "ansi_c_interpreter.tab.c"
    break;

  case 33: /* expression: assignment_expression  */
#line 174 "ansi_c_interpreter.y"
                               { (yyval.a) = (yyvsp[0].a); }
#line 1453 "ansi_c_interpreter.tab.c"
    break;

  case 34: /* expression: expression ',' assignment_expression  */
#line 176 "ansi_c_interpreter.y"
        { (yyval.a) = newast('L', (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1459 "ansi_c_interpreter.tab.c"
    break;

  case 35: /* declaration: type_specifier init_declarator ';'  */
#line 181 "ansi_c_interpreter.y"
        { (yyval.a) = (yyvsp[-1].a); }
#line 1465 "ansi_c_interpreter.tab.c"
    break;

  case 36: /* init_declarator: IDENTIFIER  */
#line 186 "ansi_c_interpreter.y"
        { 
            debug_print("init_declarator", "Processing IDENTIFIER declaration");
            if ((yyvsp[0].s) == NULL) {
                printf("ERROR: NULL identifier in declaration\n");
            } else {
                printf("DEBUG: Setting type for identifier: %s to %s\n", 
                    (yyvsp[0].s)->name, 
                    current_type == TYPE_INT ? "INT" :
                    current_type == TYPE_FLOAT ? "FLOAT" : "DOUBLE");
            }
            settype((yyvsp[0].s), current_type);
            printf("DEBUG: Creating declaration node\n");
            (yyval.a) = newdecl((yyvsp[0].s)); 
            debug_print("init_declarator", "Created declaration node");
        }
#line 1485 "ansi_c_interpreter.tab.c"
    break;

  case 37: /* init_declarator: IDENTIFIER '=' assignment_expression  */
#line 202 "ansi_c_interpreter.y"
        {
            debug_print("init_declarator", "Processing IDENTIFIER with initialization");
            if ((yyvsp[-2].s) == NULL) {
                printf("ERROR: NULL identifier in declaration with init\n");
            } else {
                printf("DEBUG: Setting type for identifier with init: %s to %s\n", 
                    (yyvsp[-2].s)->name,
                    current_type == TYPE_INT ? "INT" :
                    current_type == TYPE_FLOAT ? "FLOAT" : "DOUBLE");
            }
            settype((yyvsp[-2].s), current_type);
            printf("DEBUG: Creating declaration and assignment nodes\n");
            struct ast *decl = newdecl((yyvsp[-2].s));
            struct ast *asgn = newasgn((yyvsp[-2].s), (yyvsp[0].a));
            (yyval.a) = newast('L', decl, asgn);
            debug_print("init_declarator", "Created declaration with initialization");
        }
#line 1507 "ansi_c_interpreter.tab.c"
    break;

  case 38: /* type_specifier: VOID  */
#line 223 "ansi_c_interpreter.y"
              { current_type = TYPE_INT; /* Handle void type later */ }
#line 1513 "ansi_c_interpreter.tab.c"
    break;

  case 39: /* type_specifier: INT  */
#line 224 "ansi_c_interpreter.y"
              { current_type = TYPE_INT; }
#line 1519 "ansi_c_interpreter.tab.c"
    break;

  case 40: /* type_specifier: FLOAT  */
#line 225 "ansi_c_interpreter.y"
              { current_type = TYPE_FLOAT; }
#line 1525 "ansi_c_interpreter.tab.c"
    break;

  case 41: /* type_specifier: DOUBLE  */
#line 226 "ansi_c_interpreter.y"
              { current_type = TYPE_DOUBLE; }
#line 1531 "ansi_c_interpreter.tab.c"
    break;

  case 42: /* statement: compound_statement  */
#line 230 "ansi_c_interpreter.y"
                             { (yyval.a) = (yyvsp[0].a); }
#line 1537 "ansi_c_interpreter.tab.c"
    break;

  case 43: /* statement: expression_statement  */
#line 231 "ansi_c_interpreter.y"
                             { (yyval.a) = (yyvsp[0].a); }
#line 1543 "ansi_c_interpreter.tab.c"
    break;

  case 44: /* statement: selection_statement  */
#line 232 "ansi_c_interpreter.y"
                             { (yyval.a) = (yyvsp[0].a); }
#line 1549 "ansi_c_interpreter.tab.c"
    break;

  case 45: /* statement: iteration_statement  */
#line 233 "ansi_c_interpreter.y"
                             { (yyval.a) = (yyvsp[0].a); }
#line 1555 "ansi_c_interpreter.tab.c"
    break;

  case 46: /* statement: jump_statement  */
#line 234 "ansi_c_interpreter.y"
                            { (yyval.a) = (yyvsp[0].a); }
#line 1561 "ansi_c_interpreter.tab.c"
    break;

  case 47: /* compound_statement: '{' '}'  */
#line 238 "ansi_c_interpreter.y"
                            { (yyval.a) = NULL; }
#line 1567 "ansi_c_interpreter.tab.c"
    break;

  case 48: /* compound_statement: '{' block_item_list '}'  */
#line 239 "ansi_c_interpreter.y"
                               { (yyval.a) = (yyvsp[-1].a); }
#line 1573 "ansi_c_interpreter.tab.c"
    break;

  case 49: /* block_item_list: block_item  */
#line 243 "ansi_c_interpreter.y"
                    { (yyval.a) = (yyvsp[0].a); }
#line 1579 "ansi_c_interpreter.tab.c"
    break;

  case 50: /* block_item_list: block_item_list block_item  */
#line 245 "ansi_c_interpreter.y"
        { (yyval.a) = newast('L', (yyvsp[-1].a), (yyvsp[0].a)); }
#line 1585 "ansi_c_interpreter.tab.c"
    break;

  case 51: /* block_item: declaration  */
#line 249 "ansi_c_interpreter.y"
                      { (yyval.a) = (yyvsp[0].a); }
#line 1591 "ansi_c_interpreter.tab.c"
    break;

  case 52: /* block_item: statement  */
#line 250 "ansi_c_interpreter.y"
                     { (yyval.a) = (yyvsp[0].a); }
#line 1597 "ansi_c_interpreter.tab.c"
    break;

  case 53: /* expression_statement: ';'  */
#line 254 "ansi_c_interpreter.y"
                    { (yyval.a) = NULL; }
#line 1603 "ansi_c_interpreter.tab.c"
    break;

  case 54: /* expression_statement: expression ';'  */
#line 255 "ansi_c_interpreter.y"
                      { (yyval.a) = (yyvsp[-1].a); }
#line 1609 "ansi_c_interpreter.tab.c"
    break;

  case 55: /* selection_statement: IF '(' expression ')' statement  */
#line 260 "ansi_c_interpreter.y"
        { (yyval.a) = newflow('I', (yyvsp[-2].a), (yyvsp[0].a), NULL); }
#line 1615 "ansi_c_interpreter.tab.c"
    break;

  case 56: /* selection_statement: IF '(' expression ')' statement ELSE statement  */
#line 262 "ansi_c_interpreter.y"
        { (yyval.a) = newflow('I', (yyvsp[-4].a), (yyvsp[-2].a), (yyvsp[0].a)); }
#line 1621 "ansi_c_interpreter.tab.c"
    break;

  case 57: /* iteration_statement: WHILE '(' expression ')' statement  */
#line 267 "ansi_c_interpreter.y"
        { (yyval.a) = newflow('W', (yyvsp[-2].a), (yyvsp[0].a), NULL); }
#line 1627 "ansi_c_interpreter.tab.c"
    break;

  case 58: /* jump_statement: RETURN ';'  */
#line 271 "ansi_c_interpreter.y"
                            { (yyval.a) = NULL; }
#line 1633 "ansi_c_interpreter.tab.c"
    break;

  case 59: /* jump_statement: RETURN expression ';'  */
#line 272 "ansi_c_interpreter.y"
                             { (yyval.a) = (yyvsp[-1].a); }
#line 1639 "ansi_c_interpreter.tab.c"
    break;

  case 60: /* translation_unit: external_declaration  */
#line 277 "ansi_c_interpreter.y"
        { 
            debug_print("translation_unit", "Processing single external declaration");
            root = (yyvsp[0].a);
            if (root) {
                printf("DEBUG: Evaluating AST\n");
                struct value result = eval(root);
                printf("DEBUG: Evaluation complete\n");
                switch(result.type) {
                    case TYPE_INT:
                        printf("= %d\n", result.value.i_val);
                        break;
                    case TYPE_FLOAT:
                        printf("= %f\n", result.value.f_val);
                        break;
                    case TYPE_DOUBLE:
                        printf("= %g\n", result.value.d_val);
                        break;
                }
                printf("DEBUG: Freeing AST\n");
                treefree(root);
                printf("DEBUG: AST freed\n");
            } else {
                printf("DEBUG: Null root AST\n");
            }
            printf("> ");
            fflush(stdout);
            if (isatty(0)) {
                fflush(stdin);
            }
        }
#line 1674 "ansi_c_interpreter.tab.c"
    break;

  case 61: /* translation_unit: translation_unit external_declaration  */
#line 308 "ansi_c_interpreter.y"
        {
            debug_print("translation_unit", "Processing additional external declaration");
            root = (yyvsp[0].a);
            if (root) {
                printf("DEBUG: Evaluating AST\n");
                struct value result = eval(root);
                printf("DEBUG: Evaluation complete\n");
                switch(result.type) {
                    case TYPE_INT:
                        printf("= %d\n", result.value.i_val);
                        break;
                    case TYPE_FLOAT:
                        printf("= %f\n", result.value.f_val);
                        break;
                    case TYPE_DOUBLE:
                        printf("= %g\n", result.value.d_val);
                        break;
                }
                printf("DEBUG: Freeing AST\n");
                treefree(root);
                printf("DEBUG: AST freed\n");
            } else {
                printf("DEBUG: Null root AST\n");
            }
            printf("> ");
            fflush(stdout);
            if (isatty(0)) {
                fflush(stdin);
            }
        }
#line 1709 "ansi_c_interpreter.tab.c"
    break;

  case 62: /* external_declaration: function_definition  */
#line 341 "ansi_c_interpreter.y"
                            { (yyval.a) = (yyvsp[0].a); }
#line 1715 "ansi_c_interpreter.tab.c"
    break;

  case 63: /* external_declaration: declaration  */
#line 342 "ansi_c_interpreter.y"
                            { (yyval.a) = (yyvsp[0].a); }
#line 1721 "ansi_c_interpreter.tab.c"
    break;

  case 64: /* external_declaration: statement  */
#line 343 "ansi_c_interpreter.y"
                            { (yyval.a) = (yyvsp[0].a); }
#line 1727 "ansi_c_interpreter.tab.c"
    break;

  case 65: /* function_definition: type_specifier IDENTIFIER '(' parameter_list ')' compound_statement  */
#line 348 "ansi_c_interpreter.y"
        {
            dodef((yyvsp[-4].s), (yyvsp[-2].sl), (yyvsp[0].a));
            (yyval.a) = NULL;
        }
#line 1736 "ansi_c_interpreter.tab.c"
    break;

  case 66: /* parameter_list: parameter_declaration  */
#line 356 "ansi_c_interpreter.y"
        { (yyval.sl) = newsymlist((yyvsp[0].s), NULL); }
#line 1742 "ansi_c_interpreter.tab.c"
    break;

  case 67: /* parameter_list: parameter_list ',' parameter_declaration  */
#line 358 "ansi_c_interpreter.y"
        { (yyval.sl) = newsymlist((yyvsp[0].s), (yyvsp[-2].sl)); }
#line 1748 "ansi_c_interpreter.tab.c"
    break;

  case 68: /* parameter_declaration: type_specifier IDENTIFIER  */
#line 363 "ansi_c_interpreter.y"
        { (yyval.s) = (yyvsp[0].s); }
#line 1754 "ansi_c_interpreter.tab.c"
    break;


#line 1758 "ansi_c_interpreter.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 366 "ansi_c_interpreter.y"


void yyerror(char const *s)
{
    fflush(stdout);
    printf("\n%*s\n%*s\n", column, "^", column, s);
}
