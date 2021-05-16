/* Source-language-related definitions for GDB.

   Copyright (C) 1991-2021 Free Software Foundation, Inc.

   Contributed by the Department of Computer Science at the State University
   of New York at Buffalo.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#if !defined (LANGUAGE_H)
#define LANGUAGE_H 1

#include "symtab.h"
#include "gdbsupport/function-view.h"
#include "expression.h"

/* Forward decls for prototypes.  */
struct value;
struct objfile;
struct frame_info;
struct ui_file;
struct value_print_options;
struct type_print_options;
struct lang_varobj_ops;
struct parser_state;
class compile_instance;
struct completion_match_for_lcd;
class innermost_block_tracker;

#define MAX_FORTRAN_DIMS  7	/* Maximum number of F77 array dims.  */

/* range_mode ==
   range_mode_auto:   range_check set automatically to default of language.
   range_mode_manual: range_check set manually by user.  */

extern enum range_mode
  {
    range_mode_auto, range_mode_manual
  }
range_mode;

/* range_check ==
   range_check_on:    Ranges are checked in GDB expressions, producing errors.
   range_check_warn:  Ranges are checked, producing warnings.
   range_check_off:   Ranges are not checked in GDB expressions.  */

extern enum range_check
  {
    range_check_off, range_check_warn, range_check_on
  }
range_check;

/* case_mode ==
   case_mode_auto:   case_sensitivity set upon selection of scope.
   case_mode_manual: case_sensitivity set only by user.  */

extern enum case_mode
  {
    case_mode_auto, case_mode_manual
  }
case_mode;

/* array_ordering ==
   array_row_major:     Arrays are in row major order.
   array_column_major:  Arrays are in column major order.  */

extern enum array_ordering
  {
    array_row_major, array_column_major
  } 
array_ordering;


/* case_sensitivity ==
   case_sensitive_on:   Case sensitivity in name matching is used.
   case_sensitive_off:  Case sensitivity in name matching is not used.  */

extern enum case_sensitivity
  {
    case_sensitive_on, case_sensitive_off
  }
case_sensitivity;


/* macro_expansion ==
   macro_expansion_no:  No macro expansion is available.
   macro_expansion_c:   C-like macro expansion is available.  */

enum macro_expansion
  {
    macro_expansion_no, macro_expansion_c
  };


/* Per architecture (OS/ABI) language information.  */

struct language_arch_info
{
  /* Its primitive types.  This is a vector ended by a NULL pointer.
     These types can be specified by name in parsing types in
     expressions, regardless of whether the program being debugged
     actually defines such a type.  */
  struct type **primitive_type_vector;

  /* Symbol wrappers around primitive_type_vector, so that the symbol lookup
     machinery can return them.  */
  struct symbol **primitive_type_symbols;

  /* Type of elements of strings.  */
  struct type *string_char_type;

  /* Symbol name of type to use as boolean type, if defined.  */
  const char *bool_type_symbol;
  /* Otherwise, this is the default boolean builtin type.  */
  struct type *bool_type_default;
};

/* In a language (particularly C++) a function argument of an aggregate
   type (i.e.  class/struct/union) may be implicitly passed by reference
   even though it is declared a call-by-value argument in the source.
   The struct below puts together necessary information for GDB to be
   able to detect and carry out pass-by-reference semantics for a
   particular type.  This type is referred as T in the inlined comments
   below.

   The default values of the fields are chosen to give correct semantics
   for primitive types and for simple aggregate types, such as

   class T {
     int x;
   };  */

struct language_pass_by_ref_info
{
  /* True if an argument of type T can be passed to a function by value
     (i.e.  not through an implicit reference).  False, otherwise.  */
  bool trivially_copyable = true;

  /* True if a copy of a value of type T can be initialized by
     memcpy'ing the value bit-by-bit.  False, otherwise.
     E.g.  If T has a user-defined copy ctor, this should be false.  */
  bool trivially_copy_constructible = true;

  /* True if a value of type T can be destructed simply by reclaiming
     the memory area occupied by the value.  False, otherwise.
     E.g.  If T has a user-defined destructor, this should be false.  */
  bool trivially_destructible = true;

  /* True if it is allowed to create a copy of a value of type T.
     False, otherwise.
     E.g.  If T has a deleted copy ctor, this should be false.  */
  bool copy_constructible = true;

  /* True if a value of type T can be destructed.  False, otherwise.
     E.g.  If T has a deleted destructor, this should be false.  */
  bool destructible = true;
};

/* Splitting strings into words.  */
extern const char *default_word_break_characters (void);

/* Structure tying together assorted information about a language.

   As we move over from the old structure based languages to a class
   hierarchy of languages this structure will continue to contain a
   mixture of both data and function pointers.

   Once the class hierarchy of languages in place the first task is to
   remove the function pointers from this structure and convert them into
   member functions on the different language classes.

   The current plan it to keep the constant data that describes a language
   in this structure, and have each language pass in an instance of this
   structure at construction time.  */

struct language_data
  {
    /* Name of the language.  */

    const char *la_name;

    /* Natural or official name of the language.  */

    const char *la_natural_name;

    /* its symtab language-enum (defs.h).  */

    enum language la_language;

    /* Default range checking.  */

    enum range_check la_range_check;

    /* Default case sensitivity.  */
    enum case_sensitivity la_case_sensitivity;

    /* Multi-dimensional array ordering.  */
    enum array_ordering la_array_ordering;

    /* Style of macro expansion, if any, supported by this language.  */
    enum macro_expansion la_macro_expansion;

    /* A NULL-terminated array of file extensions for this language.
       The extension must include the ".", like ".c".  If this
       language doesn't need to provide any filename extensions, this
       may be NULL.  */

    const char *const *la_filename_extensions;

    /* Definitions related to expression printing, prefixifying, and
       dumping.  */

    const struct exp_descriptor *la_exp_desc;

    /* Now come some hooks for lookup_symbol.  */

    /* If this is non-NULL, specifies the name that of the implicit
       local variable that refers to the current object instance.  */

    const char *la_name_of_this;

    /* True if the symbols names should be stored in GDB's data structures
       for minimal/partial/full symbols using their linkage (aka mangled)
       form; false if the symbol names should be demangled first.

       Most languages implement symbol lookup by comparing the demangled
       names, in which case it is advantageous to store that information
       already demangled, and so would set this field to false.

       On the other hand, some languages have opted for doing symbol
       lookups by comparing mangled names instead, for reasons usually
       specific to the language.  Those languages should set this field
       to true.

       And finally, other languages such as C or Asm do not have
       the concept of mangled vs demangled name, so those languages
       should set this field to true as well, to prevent any accidental
       demangling through an unrelated language's demangler.  */

    const bool la_store_sym_names_in_linkage_form_p;

    /* Table for printing expressions.  */

    const struct op_print *la_op_print_tab;

    /* Zero if the language has first-class arrays.  True if there are no
       array values, and array objects decay to pointers, as in C.  */

    char c_style_arrays;

    /* Index to use for extracting the first element of a string.  */
    char string_lower_bound;

    /* Various operations on varobj.  */
    const struct lang_varobj_ops *la_varobj_ops;

    /* This string is used by the 'set print max-depth' setting.  When GDB
       replaces a struct or union (during value printing) that is "too
       deep" this string is displayed instead.  */
    const char *la_struct_too_deep_ellipsis;

  };

/* Base class from which all other language classes derive.  */

struct language_defn : language_data
{
  language_defn (enum language lang, const language_data &init_data)
    : language_data (init_data)
  {
    /* We should only ever create one instance of each language.  */
    gdb_assert (languages[lang] == nullptr);
    languages[lang] = this;
  }

  /* Print the index of an element of an array.  This default
     implementation prints using C99 syntax.  */

  virtual void print_array_index (struct type *index_type,
				  LONGEST index_value,
				  struct ui_file *stream,
				  const value_print_options *options) const;

  /* Given a symbol VAR, the corresponding block VAR_BLOCK (if any) and a
     stack frame id FRAME, read the value of the variable and return (pointer
     to a) struct value containing the value.

     VAR_BLOCK is needed if there's a possibility for VAR to be outside
     FRAME.  This is what happens if FRAME correspond to a nested function
     and VAR is defined in the outer function.  If callers know that VAR is
     located in FRAME or is global/static, NULL can be passed as VAR_BLOCK.

     Throw an error if the variable cannot be found.  */

  virtual struct value *read_var_value (struct symbol *var,
					const struct block *var_block,
					struct frame_info *frame) const;

  /* Return information about whether TYPE should be passed
     (and returned) by reference at the language level.  The default
     implementation returns a LANGUAGE_PASS_BY_REF_INFO initialised in its
     default state.  */

  virtual struct language_pass_by_ref_info pass_by_reference_info
	(struct type *type) const
  {
    return {};
  }

  /* The per-architecture (OS/ABI) language information.  */

  virtual void language_arch_info (struct gdbarch *,
				   struct language_arch_info *) const = 0;

  /* Find the definition of the type with the given name.  */

  virtual struct type *lookup_transparent_type (const char *name) const
  {
    return basic_lookup_transparent_type (name);
  }

  /* Find all symbols in the current program space matching NAME in
     DOMAIN, according to this language's rules.

     The search is done in BLOCK only.
     The caller is responsible for iterating up through superblocks
     if desired.

     For each one, call CALLBACK with the symbol.  If CALLBACK
     returns false, the iteration ends at that point.

     This field may not be NULL.  If the language does not need any
     special processing here, 'iterate_over_symbols' should be
     used as the definition.  */
  virtual bool iterate_over_symbols
	(const struct block *block, const lookup_name_info &name,
	 domain_enum domain,
	 gdb::function_view<symbol_found_callback_ftype> callback) const
  {
    return ::iterate_over_symbols (block, name, domain, callback);
  }

  /* Return a pointer to the function that should be used to match a
     symbol name against LOOKUP_NAME, according to this language's
     rules.  The matching algorithm depends on LOOKUP_NAME.  For
     example, on Ada, the matching algorithm depends on the symbol
     name (wild/full/verbatim matching), and on whether we're doing
     a normal lookup or a completion match lookup.

     As Ada wants to capture symbol matching for all languages in some
     cases, then this method is a non-overridable interface.  Languages
     should override GET_SYMBOL_NAME_MATCHER_INNER if they need to.  */

  symbol_name_matcher_ftype *get_symbol_name_matcher
	(const lookup_name_info &lookup_name) const;

  /* If this language allows compilation from the gdb command line, then
     this method will return an instance of struct gcc_context appropriate
     to the language.  If compilation for this language is generally
     supported, but something goes wrong then an exception is thrown.  The
     returned compiler instance is owned by its caller and must be
     deallocated by the caller.  If compilation is not supported for this
     language then this method returns NULL.  */

  virtual compile_instance *get_compile_instance () const
  {
    return nullptr;
  }

  /* This method must be overridden if 'get_compile_instance' is
     overridden.

     This takes the user-supplied text and returns a new bit of code
     to compile.

     INST is the compiler instance being used.
     INPUT is the user's input text.
     GDBARCH is the architecture to use.
     EXPR_BLOCK is the block in which the expression is being
     parsed.
     EXPR_PC is the PC at which the expression is being parsed.  */

  virtual std::string compute_program (compile_instance *inst,
				       const char *input,
				       struct gdbarch *gdbarch,
				       const struct block *expr_block,
				       CORE_ADDR expr_pc) const
  {
    gdb_assert_not_reached ("language_defn::compute_program");
  }

  /* Hash the given symbol search name.  */
  virtual unsigned int search_name_hash (const char *name) const;

  /* Demangle a symbol according to this language's rules.  Unlike
     la_demangle, this does not take any options.

     *DEMANGLED will be set by this function.

     If this function returns false, then *DEMANGLED must always be set
     to NULL.

     If this function returns true, the implementation may set this to
     a xmalloc'd string holding the demangled form.  However, it is
     not required to.  The string, if any, is owned by the caller.

     The resulting string should be of the form that will be
     installed into a symbol.  */
  virtual bool sniff_from_mangled_name (const char *mangled,
					char **demangled) const
  {
    *demangled = nullptr;
    return false;
  }

  /* Return demangled language symbol version of MANGLED, or NULL.  */
  virtual char *demangle (const char *mangled, int options) const
  {
    return nullptr;
  }

  /* Print a type using syntax appropriate for this language.  */

  virtual void print_type (struct type *, const char *, struct ui_file *, int,
			   int, const struct type_print_options *) const = 0;

  /* PC is possibly an unknown languages trampoline.
     If that PC falls in a trampoline belonging to this language, return
     the address of the first pc in the real function, or 0 if it isn't a
     language tramp for this language.  */
  virtual CORE_ADDR skip_trampoline (struct frame_info *fi, CORE_ADDR pc) const
  {
    return (CORE_ADDR) 0;
  }

  /* Return class name of a mangled method name or NULL.  */
  virtual char *class_name_from_physname (const char *physname) const
  {
    return nullptr;
  }

  /* The list of characters forming word boundaries.  */
  virtual const char *word_break_characters (void) const
  {
    return default_word_break_characters ();
  }

  /* Add to the completion tracker all symbols which are possible
     completions for TEXT.  WORD is the entire command on which the
     completion is being made.  If CODE is TYPE_CODE_UNDEF, then all
     symbols should be examined; otherwise, only STRUCT_DOMAIN symbols
     whose type has a code of CODE should be matched.  */

  virtual void collect_symbol_completion_matches
	(completion_tracker &tracker,
	 complete_symbol_mode mode,
	 symbol_name_match_type name_match_type,
	 const char *text,
	 const char *word,
	 enum type_code code) const
  {
    return default_collect_symbol_completion_matches_break_on
      (tracker, mode, name_match_type, text, word, "", code);
  }

  /* This is a function that lookup_symbol will call when it gets to
     the part of symbol lookup where C looks up static and global
     variables.  This default implements the basic C lookup rules.  */

  virtual struct block_symbol lookup_symbol_nonlocal
	(const char *name,
	 const struct block *block,
	 const domain_enum domain) const;

  /* Return an expression that can be used for a location
     watchpoint.  TYPE is a pointer type that points to the memory
     to watch, and ADDR is the address of the watched memory.  */
  virtual gdb::unique_xmalloc_ptr<char> watch_location_expression
	(struct type *type, CORE_ADDR addr) const;

  /* List of all known languages.  */
  static const struct language_defn *languages[nr_languages];

  /* Print a top-level value using syntax appropriate for this language.  */
  virtual void value_print (struct value *val, struct ui_file *stream,
			    const struct value_print_options *options) const;

  /* Print a value using syntax appropriate for this language.  RECURSE is
     the recursion depth.  It is zero-based.  */
  virtual void value_print_inner
	(struct value *val, struct ui_file *stream, int recurse,
	 const struct value_print_options *options) const;

  /* Parser function.  */

  virtual int parser (struct parser_state *ps) const;

  /* Given an expression *EXPP created by prefixifying the result of
     la_parser, perform any remaining processing necessary to complete its
     translation.  *EXPP may change; la_post_parser is responsible for
     releasing its previous contents, if necessary.  If VOID_CONTEXT_P,
     then no value is expected from the expression.  If COMPLETING is
     non-zero, then the expression has been parsed for completion, not
     evaluation.  */

  virtual void post_parser (expression_up *expp, int void_context_p,
			    int completing,
			    innermost_block_tracker *tracker) const
  {
    /* By default the post-parser does nothing.  */
  }

  /* Print the character CH (of type CHTYPE) on STREAM as part of the
     contents of a literal string whose delimiter is QUOTER.  */

  virtual void emitchar (int ch, struct type *chtype,
			 struct ui_file *stream, int quoter) const;

  virtual void printchar (int ch, struct type *chtype,
			  struct ui_file * stream) const;

/* Print the character string STRING, printing at most LENGTH characters.
   Printing stops early if the number hits print_max; repeat counts
   are printed as appropriate.  Print ellipses at the end if we
   had to stop before printing LENGTH characters, or if FORCE_ELLIPSES.  */

  virtual void printstr (struct ui_file *stream, struct type *elttype,
			 const gdb_byte *string, unsigned int length,
			 const char *encoding, int force_ellipses,
			 const struct value_print_options *options) const;


  /* Print a typedef using syntax appropriate for this language.
     TYPE is the underlying type.  NEW_SYMBOL is the symbol naming
     the type.  STREAM is the output stream on which to print.  */

  virtual void print_typedef (struct type *type, struct symbol *new_symbol,
			      struct ui_file *stream) const;

  /* Return true if TYPE is a string type.  */
  virtual bool is_string_type_p (struct type *type) const;

protected:

  /* This is the overridable part of the GET_SYMBOL_NAME_MATCHER method.
     See that method for a description of the arguments.  */

  virtual symbol_name_matcher_ftype *get_symbol_name_matcher_inner
	  (const lookup_name_info &lookup_name) const;
};

/* Pointer to the language_defn for our current language.  This pointer
   always points to *some* valid struct; it can be used without checking
   it for validity.

   The current language affects expression parsing and evaluation
   (FIXME: it might be cleaner to make the evaluation-related stuff
   separate exp_opcodes for each different set of semantics.  We
   should at least think this through more clearly with respect to
   what happens if the language is changed between parsing and
   evaluation) and printing of things like types and arrays.  It does
   *not* affect symbol-reading-- each source file in a symbol-file has
   its own language and we should keep track of that regardless of the
   language when symbols are read.  If we want some manual setting for
   the language of symbol files (e.g. detecting when ".c" files are
   C++), it should be a separate setting from the current_language.  */

extern const struct language_defn *current_language;

/* Pointer to the language_defn expected by the user, e.g. the language
   of main(), or the language we last mentioned in a message, or C.  */

extern const struct language_defn *expected_language;

/* Warning issued when current_language and the language of the current
   frame do not match.  */

extern const char lang_frame_mismatch_warn[];

/* language_mode == 
   language_mode_auto:   current_language automatically set upon selection
   of scope (e.g. stack frame)
   language_mode_manual: current_language set only by user.  */

extern enum language_mode
  {
    language_mode_auto, language_mode_manual
  }
language_mode;

struct type *language_bool_type (const struct language_defn *l,
				 struct gdbarch *gdbarch);

struct type *language_string_char_type (const struct language_defn *l,
					struct gdbarch *gdbarch);

/* Look up type NAME in language L, and return its definition for architecture
   GDBARCH.  Returns NULL if not found.  */

struct type *language_lookup_primitive_type (const struct language_defn *l,
					     struct gdbarch *gdbarch,
					     const char *name);

/* Wrapper around language_lookup_primitive_type to return the
   corresponding symbol.  */

struct symbol *
  language_lookup_primitive_type_as_symbol (const struct language_defn *l,
					    struct gdbarch *gdbarch,
					    const char *name);


/* These macros define the behaviour of the expression 
   evaluator.  */

/* Should we range check values against the domain of their type?  */
#define RANGE_CHECK (range_check != range_check_off)

/* "cast" really means conversion.  */
/* FIXME -- should be a setting in language_defn.  */
#define CAST_IS_CONVERSION(LANG) ((LANG)->la_language == language_c  || \
				  (LANG)->la_language == language_cplus || \
				  (LANG)->la_language == language_objc)

extern void language_info (int);

extern enum language set_language (enum language);


/* This page contains functions that return things that are
   specific to languages.  Each of these functions is based on
   the current setting of working_lang, which the user sets
   with the "set language" command.  */

#define LA_PRINT_TYPE(type,varstring,stream,show,level,flags)		\
  (current_language->print_type(type,varstring,stream,show,level,flags))

#define LA_PRINT_TYPEDEF(type,new_symbol,stream) \
  (current_language->print_typedef (type,new_symbol,stream))

#define LA_VALUE_PRINT(val,stream,options) \
  (current_language->value_print (val,stream,options))

#define LA_PRINT_CHAR(ch, type, stream) \
  (current_language->printchar (ch, type, stream))
#define LA_PRINT_STRING(stream, elttype, string, length, encoding, force_ellipses, options) \
  (current_language->printstr (stream, elttype, string, length, \
			       encoding, force_ellipses,options))
#define LA_EMIT_CHAR(ch, type, stream, quoter) \
  (current_language->emitchar (ch, type, stream, quoter))

#define LA_PRINT_ARRAY_INDEX(index_type, index_value, stream, options)	\
  (current_language->print_array_index(index_type, index_value, stream, \
				       options))

#define LA_ITERATE_OVER_SYMBOLS(BLOCK, NAME, DOMAIN, CALLBACK) \
  (current_language->iterate_over_symbols (BLOCK, NAME, DOMAIN, CALLBACK))

/* Test a character to decide whether it can be printed in literal form
   or needs to be printed in another representation.  For example,
   in C the literal form of the character with octal value 141 is 'a'
   and the "other representation" is '\141'.  The "other representation"
   is program language dependent.  */

#define PRINT_LITERAL_FORM(c)		\
  ((c) >= 0x20				\
   && ((c) < 0x7F || (c) >= 0xA0)	\
   && (!sevenbit_strings || (c) < 0x80))

/* Type predicates */

extern int pointer_type (struct type *);

/* Error messages */

extern void range_error (const char *, ...) ATTRIBUTE_PRINTF (1, 2);

/* Data:  Does this value represent "truth" to the current language?  */

extern int value_true (struct value *);

/* Misc:  The string representing a particular enum language.  */

extern enum language language_enum (const char *str);

extern const struct language_defn *language_def (enum language);

extern const char *language_str (enum language);

/* Check for a language-specific trampoline.  */

extern CORE_ADDR skip_language_trampoline (struct frame_info *, CORE_ADDR pc);

/* Return demangled language symbol, or NULL.  */
extern char *language_demangle (const struct language_defn *current_language, 
				const char *mangled, int options);

/* Return information about whether TYPE should be passed
   (and returned) by reference at the language level.  */
struct language_pass_by_ref_info language_pass_by_reference (struct type *type);

void c_get_string (struct value *value,
		   gdb::unique_xmalloc_ptr<gdb_byte> *buffer,
		   int *length, struct type **char_type,
		   const char **charset);

/* Get LANG's symbol_name_matcher method for LOOKUP_NAME.  Returns
   default_symbol_name_matcher if not set.  LANG is used as a hint;
   the function may ignore it depending on the current language and
   LOOKUP_NAME.  Specifically, if the current language is Ada, this
   may return an Ada matcher regardless of LANG.  */
symbol_name_matcher_ftype *get_symbol_name_matcher
  (const language_defn *lang, const lookup_name_info &lookup_name);

/* Save the current language and restore it upon destruction.  */

class scoped_restore_current_language
{
public:

  explicit scoped_restore_current_language ()
    : m_lang (current_language->la_language)
  {
  }

  ~scoped_restore_current_language ()
  {
    set_language (m_lang);
  }

  scoped_restore_current_language (const scoped_restore_current_language &)
      = delete;
  scoped_restore_current_language &operator=
      (const scoped_restore_current_language &) = delete;

private:

  enum language m_lang;
};

/* If language_mode is language_mode_auto,
   then switch current language to the language of SYM
   and restore current language upon destruction.

   Else do nothing.  */

class scoped_switch_to_sym_language_if_auto
{
public:

  explicit scoped_switch_to_sym_language_if_auto (const struct symbol *sym)
  {
    if (language_mode == language_mode_auto)
      {
	m_lang = current_language->la_language;
	m_switched = true;
	set_language (sym->language ());
      }
    else
      {
	m_switched = false;
	/* Assign to m_lang to silence a GCC warning.  See
	   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635.  */
	m_lang = language_unknown;
      }
  }

  ~scoped_switch_to_sym_language_if_auto ()
  {
    if (m_switched)
      set_language (m_lang);
  }

  DISABLE_COPY_AND_ASSIGN (scoped_switch_to_sym_language_if_auto);

private:
  bool m_switched;
  enum language m_lang;
};

#endif /* defined (LANGUAGE_H) */
