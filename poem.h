#define VERSION "0.0.1"

#define TAB '\t'
#define EOL '\n'
#define EOS '\0'
#define BACKSLASH '\\'
#define NULL_CHAR '\0'

#define INLINE __inline__

#define false 0
#define true  1

#define _(X) gettext(X)

#define INITIAL_BUFFER_SIZE 1000
#define INITIAL_STACK_SIZE 2

#define DEFAULT_RIGHT_MARGIN 78
#define DEFAULT_RIGHT_COMMENT_MARGIN 78
#define DEFAULT_LABEL_INDENT -2

/* Warning messages:  poem continues */
#define WARNING(s, a, b) \
    message (_("Warning"), s, (unsigned int *)(a), (unsigned int *)(b))

/* Error messages: poem stops processing the current file. */
#define ERROR(s, a, b) \
    message (_("Error"), s, (unsigned int *)(a), (unsigned int *)(b))

#define BACKUP_SUFFIX_STR    "~"
#define BACKUP_SUFFIX_CHAR   '~'
#define BACKUP_SUFFIX_FORMAT "%s.~%0*d~"

#define KR_SETTINGS_STRING (int *) \
     "-nbad\0-bap\0-nbc\0-bbo\0-hnl\0-br\0-brs\0-c33\0-cd33\0" \
     "-ncdb\0-ce\0-ci4\0-cli0\0-d0\0-di1\0-nfc1\0-i4\0-ip0\0-l75\0-lp\0" \
     "-npcs\0-nprs\0-npsl\0-sai\0-saf\0-saw\0-cs\0-nsc\0-nsob\0-nfca\0-cp33\0-nss\0"

#define GNU_SETTINGS_STRING  (int *) \
     "-nbad\0-bap\0-bbo\0-hnl\0-nbc\0-bl\0-bls\0-ncdb\0-cs\0-nce\0" \
     "-di2\0-ndj\0-nfc1\0-i2\0-ip5\0-lp\0-pcs\0-nprs\0-psl\0-nsc\0-sai\0-saf\0-saw\0-nsob\0" \
     "-bli2\0-cp1\0-nfca\0"

#define ORIG_SETTINGS_STRING (int *) \
     "-nbap\0-nbad\0-bbo\0-hnl\0-bc\0-br\0-brs\0-c33\0-cd33\0-cdb\0" \
     "-ce\0-ci4\0-cli0\0-cp33\0-di16\0-fc1\0-fca\0-i4\0-l75\0-lp\0-npcs\0-nprs\0" \
     "-psl\0-sc\0-sai\0-saf\0-saw\0-nsob\0-nss\0-ts8\0"

#define LINUX_SETTINGS_STRING (int *) \
     "-nbad\0-bap\0-nbc\0-bbo\0-hnl\0-br\0-brs\0-c33\0-cd33\0" \
     "-ncdb\0-ce\0-ci4\0-cli0\0-d0\0-di1\0-nfc1\0-i8\0-ip0\0-l80\0-lp\0" \
     "-npcs\0-nprs\0-npsl\0-sai\0-saf\0-saw\0-ncs\0-nsc\0-sob\0-nfca\0-cp33\0-ss\0" \
     "-ts8\0-il1\0"

/* This should come from stdio.h and be some system-optimal number */
#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

#define alphanum 1
#define opchar 3

#ifndef isascii
#define ISDIGIT(c) (isdigit ((unsigned char) (c)))
#else
#define ISDIGIT(c) (isascii (c) && isdigit (c))
#endif

/* The name of the profile file if the user doesn't supply an explicit one. N.B. Some operating systems don't allow more than one dot in a filename. */
#if defined (ONE_DOT_PER_FILENAME)
#define INDENT_PROFILE "poem.pro"
#else
#define INDENT_PROFILE ".poem.pro"
#endif

/* an sprintf format to use to generate the full profile path from a directory and a file name. */
#ifndef PROFILE_FORMAT
#define PROFILE_FORMAT "%s/%s"
#endif

/* The name of the environment variable the user can set to supply the name of the profile file. */
#define PROFILE_ENV_NAME "INDENT_PROFILE"

/* Check the limits of the comment, code or label buffer, and expand as neccessary. */
#define CHECK_SIZE(__x) \
	if (e_##__x >= l_##__x) { \
		int nsize = l_##__x - s_##__x + 400; \
		__x##buf = (char *)xrealloc(__x##buf, nsize); \
		e_##__x = __x##buf + (e_##__x - s_##__x) + 1; \
		l_##__x = __x##buf + nsize - 5; \
		s_##__x = __x##buf + 1; \
	}

/* round up P to be a multiple of SIZE. */
#ifndef ROUND_UP
#define ROUND_UP(p, size) (((unsigned long)(p) + (size) - 1) & ~((size) - 1))
#endif

#define ENTER_PROCE fprintf(stderr, "Enter procedure : %s...\n", __FUNCTION__)
#define EXIT_PROCE fprintf(stderr, "Exit procedure : %s...\n", __FUNCTION__)

typedef unsigned char BOOLEAN;

/* Token is an ident and is a reserved word, remember the type. */
typedef enum rwcodes {
	rw_none,
	
	rw_operator,
	
	rw_break,
	
	rw_switch,
	
	rw_case,
	
	rw_struct_like,
	
	rw_enum,

	rw_decl,
	
	rw_sp_paren,
	
	rw_sp_nparen,
	
	rw_sp_else,
	
	rw_sizeof,
	
	rw_return
} rwcodes_ty;

/* Parsers */
typedef enum codes {
	code_eof = 0,
	
	newline,
	
	/* '(' or '[', also '{' in an initialization. */
	lparen,
	
	/* ')' or ']', also '}' in an initialization. */
	rparen,
	
	start_token,

	unary_op,

	binary_op,

	postop,

	question,

	casestmt,

	colon,
	
	doublecolon,
	
	semicolon,

	lbrace,
	
	rbrace,

	ident,

	overloaded,
	
	cpp_operator,

	comma,
	
	comment,
	
	cplus_comment,
	
	swstmt,

	preesc,

	form_feed,

	decl,

	sp_paren,
	
	sp_nparen,
	
	sp_else,
	
	ifstmt,

	elseifstmt,

	whilestmt,

	forstmt,

	stmt,

	stmtl,
	
	elselit,

	dolit,

	dohead,

	dostmt,

	ifhead,

	elsehead,
	
	/* '.' or "->" */
	struct_delim,

	/* the '__attribute__' qualifier in GNU C */
	attribute,
	
	number_of_codes
} codes_ty;

/*
 * Values that `poem' can return for exit status.
 *
 * `total_success' means no errors or warnings were found during a successful invocation of the program.
 *
 * `invocation_error' is returned if an invocation problem (like an incorrect option) prevents any formatting to occur.
 *
 * `poem_error' is returned if errors occur during formatting which do not prevent completion
 * of the formatting, but which appear to be manifested by incorrect code (i.e, code which wouldn't compile).
 *
 * `poem_punt' is returned if formatting of a file is halted because of an error with the file
 * which prevents completion of formatting. If more than one input file was specified, poem continues to the next file.
 *
 * `poem_fatal' is returned if a serious internal problem occurs and the entire poem process is terminated,
 * even if all specified files have not been processed.
 *
 * `system_error' is returned if a system level problem occurs, such as virtual memory exhausted.
 */
typedef enum exit_values {
	total_success = 0,
	invocation_error = 1,
	poem_error = 2,
	poem_punt = 3,
	poem_fatal = 4,
	system_error = 5
} exit_values_ty;

typedef struct file_buffer {
	char *name;
	unsigned long size;
	char *data;
} file_buffer_ty;

typedef enum bb_code {
	bb_none,

	bb_comma,

	bb_embedded_comment_start,

	bb_embedded_comment_end,

	bb_proc_call,

	bb_dec_ind,
	
	bb_unary_op,
	
	bb_binary_op,
	
	bb_before_boolean_binary_op,
	bb_after_boolean_binary_op,

	bb_after_equal_sign,

	bb_comparisation,

	bb_question,

	bb_colon,
	
	bb_label,

	bb_semicolon,
	
	bb_lbrace,

	bb_rbrace,
	
	bb_overloaded,
	
	/* Constant variable qualifier */
	bb_const_qualifier,

	bb_ident,

	/* __attribute__ force the next label to be unary  */
	bb_attribute,

	/* Struct delimter, or enum can be include */
	bb_struct_delim,
	
	/* Member selection (bb_struct_delim `.' or `->') */
	bb_operator2,
	
	/* Member selection (bb_struct_delim `.*' or `->*') */
	bb_operator4,
	
	/* Multiply, divide or modulo */
	bb_operator5,
	
	/* Add or subtract */
	bb_operator6,
	
	bb_doublecolon,

	bb_cast
} bb_code_ty;

/* Used to keep track of buffers. */
typedef struct buf {
	char *ptr;
	
	char *end;
	
	int size;
	
	int len;
	
	/* Corresponding column of first character in the buffer. */
	int start_column;
	
	/* Column we were in when we switched buffers. */
	int column;
} buf_ty;

/* This structure stores all the user options that come from (e.g.) command line flags. */
typedef struct user_options_st {

	int verbose;
	
	int use_tabs;
	
	int tabsize;
	
	int use_stdout;
	
	/* If true, a space is inserted between if, while, or for, and a semicolon for example while (*p++ == ' ') ; */
	int space_sp_semicolon;

	int swallow_optional_blanklines;
	
	int star_comment_cont;
	
	/* Indentation level to be used for a '{' directly following a struct, union or enum */
	int struct_brace_indent;
	
	int space_after_while;
	int space_after_if;
	int space_after_for;
	
	/*
	 * If true, the names of procedures being defined get placed in column 1
	 * (ie. a newline is placed between the type of the procedure and its name)
	 */
	int procnames_start_line;
	
	int parentheses_space;
	
	int preserve_mtime;
	
	/* Set to the indentation per open parens */
	int paren_indent;
	
	int proc_calls_space;
	
	int leave_preproc_space;
	
	int force_preproc_width;
	
	/* If true, continued code within parens will be lined up to the open paren */
	int lineup_to_parens;
	
	/*
	 * True when positions at which we read a newline in the input file,
	 * should get a high priority to break long lines at.
	 */
	int honour_newlines;
	
	/* If any comments are to be reformatted */
	int format_comments;
	
	/* If comments which start in column 1 are to be magically reformatted */
	int format_col1_comments;
	
	/*
	 * True if continuation lines from the expression part of "if(e)",
	 * "while(e)", "for(e;e;e)" should be indented an extra tab stop
	 * so that they don't conflict with the code that follows
	 */
	int extra_expression_indent;
	
	/* True if declarations should be left justified */
	int ljust_decl;
	
	int cast_space;
	
	int cuddle_else;
	
	int cuddle_do_while;
	
	int comment_delimiter_on_blankline;
	
	int blank_after_sizeof;
	
	/* True if declarations should have args on new lines */
	int break_function_decl_args;
	
	/* True if declarations should have ")" after args on new lines */
	int break_function_decl_args_end;
	
	/* If true, never break declarations after commas */
	int leave_comma;
	
	/* True when we prefer to break a long line before a '&&' or '||', instead of behind it. */
	int break_before_boolean_operator;
	
	int blanklines_before_blockcomments;

	int blanklines_after_declarations;
	
	int blanklines_after_procs;
	
	/*
	 * This is vaguely similar to blanklines_after_declarations except that
	 * it only applies to the first set of declarations in a procedure (just after the first '{')
	 * and it causes a blank line to be generated even if there are no declarations
	 */
	int blanklines_after_declarations_at_proctop;

	int blanklines_around_conditional_compilation;
	
	int comment_max_col;

	int max_col;
	
	int label_offset;
	
	/* The size of one indentation level in spaces. */
	int ind_size;
	
	int indent_parameters;
	
	/* Column to poem declared identifiers to */
	int decl_indent;
	
	/* Comments not to the right of code will be placed this many indentation levels to the left of code */
	int unindent_displace;
	
	/* The column in which comments to the right of # else and # endif should start. */
	int else_endif_col;
	
	/* The distance to poem case labels from the switch statement */
	int case_indent;
	
	/* Set to the indentation between the edge of code and continuation lines in spaces */
	int continuation_indent;
	
	/* The column in which comments after declarations should be put */
	int decl_com_ind;

	/* Indentation level to be used for a '{' directly following a case label. */
	int case_brace_indent;
	
	/* True if we're handling C++ code. */
	int c_plus_plus;
	
	/* The column in which comments to the right of code should start */
	int com_ind;
	
	/* When true, brace should be on same line as the struct declaration */
	int braces_on_struct_decl_line;
	
	/* When true, brace should be on same line as the function definition */
	int braces_on_func_def_line;
	
	/* When true, brace should be on same line as if, while, etc */
	int btype_2;
	
	/* Number of spaces to poem braces from the suround if, while, etc. in -bl (bype_2 == 0) code */
	int brace_indent;
	
	/* Means "-o" was specified. */
	int expect_output_file;
} user_options_ty;

/*
 * This structure contains information relating to the state of parsing the code.
 * The difference is that the state is saved on # if and restored on # else.
 */
typedef struct parser_state {

	struct parser_state *next;
	
	codes_ty last_token;

	/* This is the parser's stack, and the current allocated size. */
	codes_ty *p_stack;
	int p_stack_size;

	/* This stack stores indentation levels. Currently allocated size is stored in p_stack_size as well */
	int *il;

	/* if the last token was an ident and is a reserved word, remember the type. */
	rwcodes_ty last_rw;

	/* also, remember its depth in parentheses '(' */
	int last_rw_depth;

	/* This stack stores case statement indentation levels. Currently allocated size is stored in p_stack_size as well */
	int *cstk;

	/* pointer to the top of stack of the p_stack, il and cstk arrays. */
	int tos;

	/*
	 * set to true when we are in a "boxed" comment.
	 * In that case, the first non-blank char should be lined up with the / in the comment closing delimiter
	 */
	int box_com;

	/* Indicates which close parens close off casts */
	int cast_mask;

	/* A bit for each paren level, set if the open paren was in a context which indicates that this pair of parentheses is not a cast. */
	int noncast_mask;

	/* Indicates which close parens close off sizeof's */
	int sizeof_mask;
	
	/* Set to 1 if we are inside a block initialization, set to 2 if we are inside an enum declaration */
	int block_init;

	/* The level of brace nesting in an initialization (0 in an enum decl) */
	int block_init_level;
	
	int last_nl;
	
	int last_saw_nl;

	/* Set when we see a ::, reset at first semicolon or left brace */
	int saw_double_colon;

	/* True when a line was broken at a place where there was no newline in the input file */
	int broken_at_non_nl;
	
	/*
	 * Set to true if there has been a declarator (e.g. int or char) and no left paren since the last semicolon.
	 * When true, a '{' is starting a structure definition or an initialization list
	 */
	int in_or_st;
	
	/* Set to true if the last token started in column 1 */
	int col_1;

	/* This is the column in which the current coment should start */
	int com_col;
	
	/* Current nesting level for structure or init */
	int dec_nest;

	/* Set to true if this line of code has part of a declaration on it */
	int decl_on_line;

	/* The level in spaces to which ind_level should be set after the current line is printed */
	int i_l_follow;
	
	/* Set to true when we are in a declaration statement. The processing of braces is then slightly different */
	BOOLEAN in_decl;
	
	/* Set to 1 while in a statement */
	int in_stmt;
	
	/* Set to 1 while in a parameter declaration */
	int in_parameter_declaration;
	
	/* The current indentation level in spaces */
	int ind_level;
	
	/* Set to 1 if next line should have an extra indentation level because we are in the middle of a statement */
	int ind_stmt;
	
	/* Set to true after scanning a token which forces a following operator to be unary */
	int last_u_d;
	
	/* Used to remember how to poem following statement */
	int p_l_follow;

	/* Parenthesization level. Used to poem within statements */
	int paren_level;
	
	/* Depth of paren nesting anywhere. */
	int paren_depth;

	/*
	 * Column positions of paren at each level. If positive, it contains just the number of
	 * characters of code on the line up to and including the right parenthesis character.
	 * If negative, it contains the opposite of the actual level of indentation in characters
	 * (that is, the indentation of the line has been added to the number of characters
	 * and the sign has been reversed to indicate that this has been done).
	 */
	/* Column positions of each paren */
	short *paren_indents;
	
	/* Currently allocated size of paren_indents */
	int paren_indents_size;

	/* Set to 1 if the current line label is a case. It is printed differently from a regular label */
	int pcase;
	
	/* Set to true by parser when it is necessary to buffer up all info up to the start of a statement after an if, while, etc. */
	int search_brace;
	
	int use_ff;
	
	/* Set to true when the following token should be prefixed by a blank. (Said prefixing is ignored in some cases.) */
	int want_blank;
	
	/* Set when a break is ok before the following token (is automatically implied by `want_blank'. */
	bb_code_ty can_break;
	
	/* Keywords like auto, break, case, ..., etc. */
	int its_a_keyword;
	
	int sizeof_keyword;
	
	char *procname;
	char *procname_end;
	
	char *classname;
	char *classname_end;
	
	/* Just saw a declaration */
	int just_saw_decl;

	/* Set to a 0 <= value if the the current '}' has a matching '{' on the same input line */
	int matching_brace_on_same_line;
} parser_state_ty;

/* Here we have the token scanner for poem. It scans of one token and puts it in the global variable "token". It returns a code, indicating the type of token scanned. */
typedef struct {
	char *rwd;
	rwcodes_ty rwcode;
} templ_ty;

typedef struct buf_break_st {
	/* The first possible break point to the right or left, if any. */
	struct buf_break_st *next, *prev;
	
	/* The break point: the first character in the buffer that will not be put on this line any more. */
	int offset;
	
	/* If ptr equals s_code and this equals s_code_corresponds_to, then ppst->procname is valid. */
	char *corresponds_to;
	
	/* Indentation column if we would break the line here. */
	int target_col;
	
	int first_level;
	
	/* Number of open '(' and '['. */
	int level;
	
	/* The number of columns left of the break point, before the break. */
	int col;
	
	/* Used to calculate the priority of this break point: */
	int priority_code_length;
	
	bb_code_ty priority_code;
	
	/* Set when in the input file there was a newline at this place. */
	int priority_newline;
	
	int priority;
} buf_break_st_ty;

/* When to make backup files. Analogous to 'version-control' in Emacs. */
typedef enum {
	unknown,
	none,
	simple,
	numbered_existing,
	numbered
} backup_mode_ty;

typedef struct {
	backup_mode_ty value;
	char *name;
} version_control_values_ty;

/* Profile types. These identify what kind of switches and arguments can be passed to poem, and how to process them. */
typedef enum {
	PRO_BOOL,
	PRO_INT,
	PRO_IGN,
	/* -T switch */
	PRO_KEY,
	PRO_SETTINGS,
	PRO_PRSTRING,
	PRO_FUNCTION
} profile_ty;

typedef enum {
	ONOFF_NA,
	OFF,
	ON
} on_or_off_ty;

/*
 * N.B.: because of the way the table here is scanned, options whose names are substrings of
 * other options must occur later; that is, with -lp vs -l, -lp must be first.
 * Also, while (most) booleans occur more than once, the last default value is the one actually assigned.
 */
typedef struct {
	char *p_name;
	
	profile_ty p_type;
	
	int p_default;

	on_or_off_ty p_special;	

	/*
	 * If p_type == PRO_SETTINGS, a (char *) pointing to a list of the switches to set, separated by NULLs, terminated by 2 NULLs.
	 * If p_type == PRO_BOOL or PRO_INT, the address of the variable that gets set by the option.
	 * If p_type == PRO_PRSTRING, a (char *) pointing to the string.
	 * If p_type == PRO_FUNCTION, a pointer to a function to be called.
	 */
	int *p_obj;

	/*
	 * Points to a nonzero value (allocated statically for all options) if the option has been specified explicitly.
	 * This is necessary because for boolean options, the options to set and reset the variable must share the explicit flag.
	 */
	int *p_explicit;
} pro_ty;

static void usage(void);

char *xmalloc(unsigned int size);

char *xrealloc(char *ptr, unsigned int size);

void fatal(const char *string, const char *a0);

void message(char *kind, char *string, unsigned int *a0, unsigned int *a1);

char *skip_horiz_space(const char *p);

file_buffer_ty *read_file(char *filename, struct stat *);

file_buffer_ty *read_stdin(void);

int current_column(void);

void fill_buffer(void);

void skip_buffered_space(void);

void addkey(char *key, rwcodes_ty val);

backup_mode_ty version_control_value(void);

void clear_buf_break_list(BOOLEAN * pbreak_line);

int compute_code_target(int paren_targ);

int compute_label_target(void);

int count_columns(int column, char *bp, int stop_char);

void set_buf_break(bb_code_ty code, int paren_targ);

void dump_line(int force_nl, int *paren_targ, BOOLEAN * break_line);

void flush_output(void);

static __inline templ_ty *is_reserved(const char *str, unsigned int len);

static __inline templ_ty * is_reserved_cc(register const char *str, register unsigned int len);

void open_output(const char *filename, const char *mode);

void reopen_output_trunc(const char *filename);

void close_output(struct stat *file_stats, const char *filename);

void inhibit_poeming(BOOLEAN flag);

int output_line_length(void);

void print_comment(int *paren_targ, BOOLEAN *pbreak_line);

void set_defaults(void);

int set_option(const char *option, const char *param, int explicit, const char *option_source);

char *set_profile(void);

void DieError(int errval, const char *fmt, ...);

void initialize_backups(void);

void make_backup(file_buffer_ty * file, const struct stat *file_stats);

int inc_pstack(void);

void parse_lparen_in_decl(void);

exit_values_ty parse(codes_ty tk);

void init_parser(void);

void reset_parser(void);

void reduce(void);

codes_ty lexi(void);

/* C code produced by gperf version 3.0.2 */
/* Command-line: gperf -D -c -l -p -t -T -g -j1 -o -K rwd -N is_reserved poem.gperf  */
/* Computed positions: -k'2-3' */

#define TOTAL_KEYWORDS 32
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 8
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 41
/* maximum key range = 39, duplicates = 0 */

static __inline unsigned int hash(register const char *str, register unsigned int len)
{
	static unsigned char asso_values[] = {
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 15, 42, 12, 42, 42,
		42,  9, 25,  9,  8,  7, 42, 42,  0, 42,
		 5,  1, 13, 42, 15,  0,  0, 16, 42, 18,
		24, 12, 15, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		42, 42, 42, 42, 42, 42
	};
	register int hval = len;

	switch (hval) {
	default:
		hval += asso_values[(unsigned char)str[2]];
	case 2:
		hval += asso_values[(unsigned char)str[1]];
		break;
	}
	return hval;
}

static __inline templ_ty *is_reserved(register const char *str, register unsigned int len)
{
	static unsigned char lengthtable[] = {
		2,  4,  4,  5,  6,  3,  8,  4,  5,  4,  8,  5,  6,  4,
		5,  6,  3,  5,  6,  6,  6,  4,  4,  8,  2,  6,  5,  6,
		6,  7,  6,  7
	};
	static templ_ty wordlist[] = {
		{"do", rw_sp_nparen,},
		{"else", rw_sp_else,},
		{"goto", rw_break,},
		{"float", rw_decl,},
		{"global", rw_decl,},
		{"int", rw_decl,},
		{"volatile", rw_decl,},
		{"long", rw_decl,},
		{"const", rw_decl,},
		{"void", rw_decl,},
		{"unsigned", rw_decl,},
		{"short", rw_decl,},
		{"return", rw_return,},
		{"case", rw_case,},
		{"union", rw_struct_like,},
		{"static", rw_decl,},
		{"for", rw_sp_paren,},
		{"while", rw_sp_paren,},
		{"struct", rw_struct_like,},
		{"signed", rw_decl,},
		{"double", rw_decl,},
		{"char", rw_decl,},
		{"enum", rw_enum,},
		{"register", rw_decl,},
		{"if", rw_sp_paren,},
		{"sizeof", rw_sizeof,},
		{"break", rw_break,},
		{"extern", rw_decl,},
		{"switch", rw_switch,},
		{"typedef", rw_decl,},
		{"va_dcl", rw_decl,},
		{"default", rw_case,}
	};

	static short lookup[] = {
		-1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
		25, 26, 27, 28, 29, 30, -1, -1, -1, -1, -1, -1, -1, 31
	};

	if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
		register int key = hash(str, len);

		if (key <= MAX_HASH_VALUE && key >= 0) {
			register int index = lookup[key];

			if (index >= 0) {
				if (len == lengthtable[index]) {
					register const char *s = wordlist[index].rwd;

					if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
						return &wordlist[index];
				}
			}
		}
	}

	return 0;
}

/* Include code generated by gperf for C++ keyword set */
/* remove old defs */
#undef MIN_HASH_VALUE
#undef MAX_HASH_VALUE
#undef TOTAL_KEYWORDS
#undef MIN_WORD_LENGTH
#undef MAX_WORD_LENGTH

/* C code produced by gperf version 3.0.2 */
/* Command-line: gperf -D -c -l -p -t -T -g -j1 -o -K rwd -N is_reserved_cc -H hash_cc poem-cc.gperf  */
/* Computed positions: -k'1,3' */

#define TOTAL_KEYWORDS 48
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 9
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 57
/* maximum key range = 55, duplicates = 0 */

static __inline unsigned int hash_cc(register const char *str, register unsigned int len)
{
	static unsigned char asso_values[] = {
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 10, 58, 10, 28, 11,
		25, 15, 25,  5, 30,  1, 58, 58,  5, 37,
		22, 12, 34, 58,  2,  3,  0, 18, 34, 27,
		58, 58, 25, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
		58, 58, 58, 58, 58, 58
	};
	register int hval = len;

	switch (hval) {
	default:
		hval += asso_values[(unsigned char)str[2]];
	case 2:
	case 1:
		hval += asso_values[(unsigned char)str[0]];
		break;
	}

	return hval;
}

static __inline templ_ty *is_reserved_cc(register const char *str, register unsigned int len)
{
	static unsigned char lengthtable[] = {
		2,  3,  5,  6,  4,  6,  6,  6,  5,  6,  8,  5,  9,  4,
		6,  5,  6,  4,  6,  5,  4,  5,  2,  7,  8,  3,  4,  6,
		5,  6,  8,  6,  4,  5,  4,  6,  7,  5,  7,  4,  8,  6,
		8,  5,  6,  6,  3,  7
	};
	static templ_ty wordlist[] = {
		{"if", rw_sp_paren,},
		{"int", rw_decl,},
		{"throw", rw_return,},
		{"return", rw_return,},
		{"goto", rw_break,},
		{"switch", rw_switch,},
		{"struct", rw_struct_like,},
		{"inline", rw_decl,},
		{"sigof", rw_sizeof,},
		{"signed", rw_decl,},
		{"register", rw_decl,},
		{"catch", rw_sp_paren,},
		{"signature", rw_struct_like,},
		{"case", rw_case,},
		{"static", rw_decl,},
		{"short", rw_decl,},
		{"extern", rw_decl,},
		{"else", rw_sp_else,},
		{"global", rw_decl,},
		{"union", rw_struct_like,},
		{"char", rw_decl,},
		{"class", rw_struct_like,},
		{"do", rw_sp_nparen,},
		{"classof", rw_sizeof,},
		{"unsigned", rw_decl,},
		{"for", rw_sp_paren,},
		{"long", rw_decl,},
		{"friend", rw_decl,},
		{"while", rw_sp_paren,},
		{"sizeof", rw_sizeof,},
		{"operator", rw_operator,},
		{"delete", rw_return,},
		{"enum", rw_enum,},
		{"const", rw_decl,},
		{"void", rw_decl,},
		{"typeof", rw_sizeof,},
		{"typedef", rw_decl,},
		{"float", rw_decl,},
		{"virtual", rw_decl,},
		{"bool", rw_decl,},
		{"template", rw_decl,},
		{"headof", rw_sizeof,},
		{"volatile", rw_decl,},
		{"break", rw_break,},
		{"double", rw_decl,},
		{"va_dcl", rw_decl,},
		{"new", rw_return,},
		{"default", rw_case,}
	};

	static short lookup[] = {
		-1, -1, -1,  0,  1, -1, -1,  2,  3,  4,  5,  6,  7,  8,
		9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
		23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
		37, 38, 39, 40, 41, 42, 43, 44, 45, -1, 46, -1, -1, -1,
		-1, 47
	};

	if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
		register int key = hash_cc (str, len);

		if (key <= MAX_HASH_VALUE && key >= 0) {
			register int index = lookup[key];

			if (index >= 0) {
				if (len == lengthtable[index]) {
					register const char *s = wordlist[index].rwd;

					if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
						return &wordlist[index];
				}
			}
		}
	}

	return 0;
}
