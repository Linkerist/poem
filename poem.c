#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <libintl.h>
#include <fcntl.h>
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>

#define PRESERVE_MTIME 1
#define HAVE_UTIME_H 1

#ifdef PRESERVE_MTIME
#include <time.h>
#ifdef HAVE_UTIME_H
#include <utime.h>
#endif
#endif

#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)

#include "poem.h"

#ifdef DEBUG
int debug = 1;
#endif

static FILE *output = NULL;

static BOOLEAN inhibited = 0;

char *in_prog_pos = NULL;

char *buf_ptr = NULL;
char *buf_end = NULL;

BOOLEAN had_eof = false;

char *cur_line = NULL;

/* Explicit flags for each option. */
static int exp_T = 0;
static int exp_bacc = 0;
static int exp_badp = 0;
static int exp_bad = 0;
static int exp_bap = 0;
static int exp_bbb = 0;
static int exp_bbo = 0;
static int exp_bc = 0;
static int exp_bl = 0;
static int exp_blf = 0;
static int exp_bli = 0;
static int exp_bls = 0;
static int exp_bs = 0;

static int exp_c = 0;
static int exp_cbi = 0;
static int exp_cdb = 0;

static int exp_cd = 0;
static int exp_cdw = 0;
static int exp_ce = 0;
static int exp_ci = 0;
static int exp_cli = 0;
static int exp_cp = 0;
static int exp_cpp = 0;
static int exp_cs = 0;
static int exp_d = 0;
static int exp_bfda = 0;
static int exp_bfde = 0;
static int exp_di = 0;

static int exp_dj = 0;
static int exp_eei = 0;
static int exp_fc1 = 0;
static int exp_fca = 0;
static int exp_gnu = 0;
static int exp_hnl = 0;
static int exp_i = 0;
static int exp_il = 0;
static int exp_ip = 0;
static int exp_kr = 0;
static int exp_l = 0;
static int exp_lc = 0;
static int exp_linux = 0;
static int exp_lp = 0;
static int exp_lps = 0;
static int exp_nip = 0;
static int exp_o = 0;
static int exp_orig = 0;
static int exp_pcs = 0;
static int exp_pi = 0;
static int exp_pmt = 0;
static int exp_pro = 0;
static int exp_prs = 0;
static int exp_psl = 0;

static int exp_ppi = 0;
static int exp_sai = 0;
static int exp_saf = 0;
static int exp_saw = 0;
static int exp_sbi = 0;
static int exp_sc = 0;
static int exp_sob = 0;
static int exp_ss = 0;
static int exp_st = 0;
static int exp_ts = 0;
static int exp_ut = 0;
static int exp_v = 0;
static int exp_version = 0;

char *token;
char *token_end;

int out_lines = 0;
int com_lines = 0;

int prev_target_col_break = 0;

int buf_break_used = 0;

int preproc_indent = 0;

static unsigned int user_specials_max;
static unsigned int user_specials_idx;

int version_width = 1;

char chartype[] = {
	/* This is used to facilitate the decision of what type (alphanumeric, operator) each character is */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 3, 0, 0, 1, 3, 3, 0,
	0, 0, 3, 3, 0, 3, 0, 3,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 0, 0, 3, 3, 3, 3,
	0, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 0, 0, 0, 3, 1,
	0, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 0, 3, 0, 3, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};

static char *simple_backup_suffix = BACKUP_SUFFIX_STR;

/* The following structure is controlled by command line parameters and their meaning is explained in poem.h. */
user_options_ty settings = { 0 };

#ifdef BERKELEY_DEFAULTS

/* Settings for original defaults */
const pro_ty pro[] = {
	{"V", PRO_PRSTRING, 0, ONOFF_NA, (int *)VERSION, &exp_version},
	{"v", PRO_BOOL, false, ON, &settings.verbose, &exp_v},
	{"ut", PRO_BOOL, true, ON, &settings.use_tabs, &exp_ut},
	{"ts", PRO_INT, 8, ONOFF_NA, &settings.tabsize, &exp_ts},
	{"st", PRO_BOOL, false, ON, &settings.use_stdout, &exp_st},
	{"ss", PRO_BOOL, false, ON, &settings.space_sp_semicolon, &exp_ss},
	{"sob", PRO_BOOL, false, ON, &settings.swallow_optional_blanklines, &exp_sob},
	{"sc", PRO_BOOL, true, ON, &settings.star_comment_cont, &exp_sc},
	{"sbi", PRO_INT, 0, ONOFF_NA, &settings.struct_brace_indent, &exp_sbi},
	{"saw", PRO_BOOL, true, ON, &settings.space_after_while, &exp_saw},
	{"sai", PRO_BOOL, true, ON, &settings.space_after_if, &exp_sai},
	{"saf", PRO_BOOL, true, ON, &settings.space_after_for, &exp_saf},
	{"psl", PRO_BOOL, true, ON, &settings.procnames_start_line, &exp_psl},
	{"prs", PRO_BOOL, false, ON, &settings.parentheses_space, &exp_prs},
#ifdef PRESERVE_MTIME
	{"pmt", PRO_BOOL, false, ON, &settings.preserve_mtime, &exp_pmt},
#endif
	{"pi", PRO_INT, -1, ONOFF_NA, &settings.paren_indent, &exp_pi},
	{"pcs", PRO_BOOL, false, ON, &settings.proc_calls_space, &exp_pcs},
	{"o", PRO_BOOL, false, ON, &settings.expect_output_file, &exp_o},
	{"nv", PRO_BOOL, false, OFF, &settings.verbose, &exp_v},
	{"nut", PRO_BOOL, true, OFF, &settings.use_tabs, &exp_ut},
	{"nss", PRO_BOOL, false, OFF, &settings.space_sp_semicolon, &exp_ss},
	{"nsob", PRO_BOOL, false, OFF, &settings.swallow_optional_blanklines, &exp_sob},
	{"nsc", PRO_BOOL, true, OFF, &settings.star_comment_cont, &exp_sc},
	{"nsaw", PRO_BOOL, true, OFF, &settings.space_after_while, &exp_saw},
	{"nsai", PRO_BOOL, true, OFF, &settings.space_after_if, &exp_sai},
	{"nsaf", PRO_BOOL, true, OFF, &settings.space_after_for, &exp_saf},
	{"npsl", PRO_BOOL, true, OFF, &settings.procnames_start_line, &exp_psl},
	{"nprs", PRO_BOOL, false, OFF, &settings.parentheses_space, &exp_prs},
	{"npro", PRO_IGN, 0, ONOFF_NA, 0, &exp_pro},
#ifdef PRESERVE_MTIME
	{"npmt", PRO_BOOL, false, OFF, &settings.preserve_mtime, &exp_pmt},
#endif
	{"npcs", PRO_BOOL, false, OFF, &settings.proc_calls_space, &exp_pcs},
	{"nlps", PRO_BOOL, false, OFF, &settings.leave_preproc_space, &exp_lps},
	{"nlp", PRO_BOOL, true, OFF, &settings.lineup_to_parens, &exp_lp},
	{"nip", PRO_SETTINGS, 0, ONOFF_NA, (int *)"-ip0", &exp_nip},
	{"nhnl", PRO_BOOL, true, OFF, &settings.honour_newlines, &exp_hnl},
	{"nfca", PRO_BOOL, true, OFF, &settings.format_comments, &exp_fca},
	{"nfc1", PRO_BOOL, true, OFF, &settings.format_col1_comments, &exp_fc1},
	{"neei", PRO_BOOL, false, OFF, &settings.extra_expression_indent, &exp_eei},
	{"ndj", PRO_BOOL, false, OFF, &settings.ljust_decl, &exp_dj},
	{"ncs", PRO_BOOL, true, OFF, &settings.cast_space, &exp_cs},
	{"nce", PRO_BOOL, true, OFF, &settings.cuddle_else, &exp_ce},
	{"ncdw", PRO_BOOL, false, OFF, &settings.cuddle_do_while, &exp_cdw},
	{"ncdb", PRO_BOOL, true, OFF, &settings.comment_delimiter_on_blankline, &exp_cdb},
	{"nbs", PRO_BOOL, false, OFF, &settings.blank_after_sizeof, &exp_bs},
	{"nbfda", PRO_BOOL, false, OFF, &settings.break_function_decl_args, &exp_bfda},
	{"nbfde", PRO_BOOL, false, OFF, &settings.break_function_decl_args_end, &exp_bfde},
	{"nbc", PRO_BOOL, true, ON, &settings.leave_comma, &exp_bc},
	{"nbbo", PRO_BOOL, true, OFF, &settings.break_before_boolean_operator, &exp_bbo},
	{"nbbb", PRO_BOOL, false, OFF, &settings.blanklines_before_blockcomments, &exp_bbb},
	{"nbap", PRO_BOOL, false, OFF, &settings.blanklines_after_procs, &exp_bap},
	{"nbadp", PRO_BOOL, false, OFF, &settings.blanklines_after_declarations_at_proctop, &exp_badp},
	{"nbad", PRO_BOOL, false, OFF, &settings.blanklines_after_declarations, &exp_bad},
	{"nbacc", PRO_BOOL, false, OFF, &settings.blanklines_around_conditional_compilation, &exp_bacc},
	{"linux", PRO_SETTINGS, 0, ONOFF_NA, LINUX_SETTINGS_STRING, &exp_linux},
	{"lps", PRO_BOOL, false, ON, &settings.leave_preproc_space, &exp_lps},
	{"lp", PRO_BOOL, true, ON, &settings.lineup_to_parens, &exp_lp},
	{"lc", PRO_INT, DEFAULT_RIGHT_COMMENT_MARGIN, ONOFF_NA, &settings.comment_max_col, &exp_lc},
	{"l", PRO_INT, DEFAULT_RIGHT_MARGIN, ONOFF_NA, &settings.max_col, &exp_l},
	{"kr", PRO_SETTINGS, 0, ONOFF_NA, KR_SETTINGS_STRING, &exp_kr},
	{"ip", PRO_INT, 4, ONOFF_NA, &settings.indent_parameters, &exp_ip},
	{"i", PRO_INT, 4, ONOFF_NA, &settings.ind_size, &exp_i},
	{"il", PRO_INT, DEFAULT_LABEL_INDENT, ONOFF_NA, &settings.label_offset, &exp_il},
	{"hnl", PRO_BOOL, true, ON, &settings.honour_newlines, &exp_hnl},
	{"h", PRO_FUNCTION, 0, ONOFF_NA, (int *)usage, &exp_version},
	{"gnu", PRO_SETTINGS, 0, ONOFF_NA, GNU_SETTINGS_STRING, &exp_gnu},
	{"fca", PRO_BOOL, true, ON, &settings.format_comments, &exp_fca},
	{"fc1", PRO_BOOL, true, ON, &settings.format_col1_comments, &exp_fc1},
	{"eei", PRO_BOOL, false, ON, &settings.extra_expression_indent, &exp_eei},
	{"dj", PRO_BOOL, false, ON, &settings.ljust_decl, &exp_dj},
	{"di", PRO_INT, 16, ONOFF_NA, &settings.decl_indent, &exp_di},
	{"d", PRO_INT, 0, ONOFF_NA, &settings.unindent_displace, &exp_d},
	{"cs", PRO_BOOL, true, ON, &settings.cast_space, &exp_cs},
	{"cp", PRO_INT, 33, ONOFF_NA, &settings.else_endif_col, &exp_cp},
	{"cli", PRO_INT, 0, ONOFF_NA, &settings.case_indent, &exp_cli},
	{"ci", PRO_INT, 4, ONOFF_NA, &settings.continuation_indent, &exp_ci},
	{"ce", PRO_BOOL, true, ON, &settings.cuddle_else, &exp_ce},
	{"cdw", PRO_BOOL, false, ON, &settings.cuddle_do_while, &exp_cdw},
	{"cdb", PRO_BOOL, true, ON, &settings.comment_delimiter_on_blankline, &exp_cdb},
	{"cd", PRO_INT, 33, ONOFF_NA, &settings.decl_com_ind, &exp_cd},
	{"cbi", PRO_INT, -1, ONOFF_NA, &settings.case_brace_indent, &exp_cbi},
	{"c++", PRO_BOOL, false, ON, &settings.c_plus_plus, &exp_cpp},
	{"c", PRO_INT, 33, ONOFF_NA, &settings.com_ind, &exp_c},
	{"bs", PRO_BOOL, false, ON, &settings.blank_after_sizeof, &exp_bs},
	{"brs", PRO_BOOL, true, ON, &settings.braces_on_struct_decl_line, &exp_bls},
	{"brf", PRO_BOOL, false, ON, &settings.braces_on_func_def_line, &exp_blf},
	{"br", PRO_BOOL, true, ON, &settings.btype_2, &exp_bl},
	{"bls", PRO_BOOL, true, OFF, &settings.braces_on_struct_decl_line, &exp_bls},
	{"blf", PRO_BOOL, false, OFF, &settings.braces_on_func_def_line, &exp_blf},
	{"bli", PRO_INT, 0, ONOFF_NA, &settings.brace_indent, &exp_bli},
	{"bl", PRO_BOOL, true, OFF, &settings.btype_2, &exp_bl},
	{"bfda", PRO_BOOL, false, ON, &settings.break_function_decl_args, &exp_bfda},
	{"bfde", PRO_BOOL, false, ON, &settings.break_function_decl_args_end, &exp_bfde},
	{"bc", PRO_BOOL, true, OFF, &settings.leave_comma, &exp_bc},
	{"bbo", PRO_BOOL, true, ON, &settings.break_before_boolean_operator, &exp_bbo},
	{"bbb", PRO_BOOL, false, ON, &settings.blanklines_before_blockcomments, &exp_bbb},
	{"bap", PRO_BOOL, false, ON, &settings.blanklines_after_procs, &exp_bap},
	{"badp", PRO_BOOL, false, ON, &settings.blanklines_after_declarations_at_proctop, &exp_badp},
	{"bad", PRO_BOOL, false, ON, &settings.blanklines_after_declarations, &exp_bad},
	{"bacc", PRO_BOOL, false, ON, &settings.blanklines_around_conditional_compilation, &exp_bacc},
	{"T", PRO_KEY, 0, ONOFF_NA, 0, &exp_T},
	{"ppi", PRO_INT, 0, ONOFF_NA, &settings.force_preproc_width, &exp_ppi},
	
	{0, PRO_IGN, 0, ONOFF_NA, 0, 0}
};

#else
/* Default to GNU style */

const pro_ty pro[] = {
	{"V", PRO_PRSTRING, 0, ONOFF_NA, (int *)VERSION, &exp_version},
	{"v", PRO_BOOL, false, ON, &settings.verbose, &exp_v},
	{"ut", PRO_BOOL, true, ON, &settings.use_tabs, &exp_ut},
	{"ts", PRO_INT, 8, ONOFF_NA, &settings.tabsize, &exp_ts},
	{"st", PRO_BOOL, false, ON, &settings.use_stdout, &exp_st},
	{"ss", PRO_BOOL, false, ON, &settings.space_sp_semicolon, &exp_ss},
	{"sob", PRO_BOOL, false, ON, &settings.swallow_optional_blanklines, &exp_sob},
	{"sc", PRO_BOOL, false, ON, &settings.star_comment_cont, &exp_sc},
	{"sbi", PRO_INT, 0, ONOFF_NA, &settings.struct_brace_indent, &exp_sbi},
	{"saw", PRO_BOOL, true, ON, &settings.space_after_while, &exp_saw},
	{"sai", PRO_BOOL, true, ON, &settings.space_after_if, &exp_sai},
	{"saf", PRO_BOOL, true, ON, &settings.space_after_for, &exp_saf},
	{"psl", PRO_BOOL, true, ON, &settings.procnames_start_line, &exp_psl},
	{"prs", PRO_BOOL, false, ON, &settings.parentheses_space, &exp_prs},
#ifdef PRESERVE_MTIME
	{"pmt", PRO_BOOL, false, ON, &settings.preserve_mtime, &exp_pmt},
#endif
	{"pi", PRO_INT, -1, ONOFF_NA, &settings.paren_indent, &exp_pi},
	{"pcs", PRO_BOOL, true, ON, &settings.proc_calls_space, &exp_pcs},
	{"orig", PRO_SETTINGS, 0, ONOFF_NA, ORIG_SETTINGS_STRING, &exp_orig},
	{"o", PRO_BOOL, false, ON, &settings.expect_output_file, &exp_o},
	{"nv", PRO_BOOL, false, OFF, &settings.verbose, &exp_v},
	{"nut", PRO_BOOL, true, OFF, &settings.use_tabs, &exp_ut},
	{"nss", PRO_BOOL, false, OFF, &settings.space_sp_semicolon, &exp_ss},
	{"nsob", PRO_BOOL, false, OFF, &settings.swallow_optional_blanklines, &exp_sob},
	{"nsc", PRO_BOOL, false, OFF, &settings.star_comment_cont, &exp_sc},
	{"nsaw", PRO_BOOL, true, OFF, &settings.space_after_while, &exp_saw},
	{"nsai", PRO_BOOL, true, OFF, &settings.space_after_if, &exp_sai},
	{"nsaf", PRO_BOOL, true, OFF, &settings.space_after_for, &exp_saf},
	{"npsl", PRO_BOOL, true, OFF, &settings.procnames_start_line, &exp_psl},
	{"nprs", PRO_BOOL, false, OFF, &settings.parentheses_space, &exp_prs},
	{"npro", PRO_IGN, 0, ONOFF_NA, 0, &exp_pro},
#ifdef PRESERVE_MTIME
	{"npmt", PRO_BOOL, false, OFF, &settings.preserve_mtime, &exp_pmt},
#endif
	{"npcs", PRO_BOOL, true, OFF, &settings.proc_calls_space, &exp_pcs},
	{"nlps", PRO_BOOL, false, OFF, &settings.leave_preproc_space, &exp_lps},
	{"nlp", PRO_BOOL, true, OFF, &settings.lineup_to_parens, &exp_lp},
	{"nip", PRO_SETTINGS, 0, ONOFF_NA, (int *)"-ip0\0", &exp_nip},
	{"nhnl", PRO_BOOL, true, OFF, &settings.honour_newlines, &exp_hnl},
	{"nfca", PRO_BOOL, false, OFF, &settings.format_comments, &exp_fca},
	{"nfc1", PRO_BOOL, false, OFF, &settings.format_col1_comments, &exp_fc1},
	{"neei", PRO_BOOL, false, OFF, &settings.extra_expression_indent, &exp_eei},
	{"ndj", PRO_BOOL, false, OFF, &settings.ljust_decl, &exp_dj},
	{"ncs", PRO_BOOL, true, OFF, &settings.cast_space, &exp_cs},
	{"nce", PRO_BOOL, false, OFF, &settings.cuddle_else, &exp_ce},
	{"ncdw", PRO_BOOL, false, OFF, &settings.cuddle_do_while, &exp_cdw},
	{"ncdb", PRO_BOOL, false, OFF, &settings.comment_delimiter_on_blankline, &exp_cdb},
	{"nbs", PRO_BOOL, false, OFF, &settings.blank_after_sizeof, &exp_bs},
	{"nbfda", PRO_BOOL, false, OFF, &settings.break_function_decl_args, &exp_bfda},
	{"nbfde", PRO_BOOL, false, OFF, &settings.break_function_decl_args_end, &exp_bfde},
	{"nbc", PRO_BOOL, true, ON, &settings.leave_comma, &exp_bc},
	{"nbbo", PRO_BOOL, true, OFF, &settings.break_before_boolean_operator, &exp_bbo},
	{"nbbb", PRO_BOOL, false, OFF, &settings.blanklines_before_blockcomments, &exp_bbb},
	{"nbap", PRO_BOOL, true, OFF, &settings.blanklines_after_procs, &exp_bap},
	{"nbadp", PRO_BOOL, false, OFF, &settings.blanklines_after_declarations_at_proctop, &exp_badp},
	{"nbad", PRO_BOOL, false, OFF, &settings.blanklines_after_declarations, &exp_bad},
	{"nbacc", PRO_BOOL, false, OFF, &settings.blanklines_around_conditional_compilation, &exp_bacc},
	{"linux", PRO_SETTINGS, 0, ONOFF_NA, LINUX_SETTINGS_STRING, &exp_linux},
	{"lps", PRO_BOOL, false, ON, &settings.leave_preproc_space, &exp_lps},
	{"lp", PRO_BOOL, true, ON, &settings.lineup_to_parens, &exp_lp},
	{"lc", PRO_INT, DEFAULT_RIGHT_COMMENT_MARGIN, ONOFF_NA, &settings.comment_max_col, &exp_lc},
	{"l", PRO_INT, DEFAULT_RIGHT_MARGIN, ONOFF_NA, &settings.max_col, &exp_l},
	{"kr", PRO_SETTINGS, 0, ONOFF_NA, KR_SETTINGS_STRING, &exp_kr},
	{"il", PRO_INT, DEFAULT_LABEL_INDENT, ONOFF_NA, &settings.label_offset, &exp_il},
	{"ip", PRO_INT, 5, ONOFF_NA, &settings.indent_parameters, &exp_ip},
	{"i", PRO_INT, 2, ONOFF_NA, &settings.ind_size, &exp_i},
	{"hnl", PRO_BOOL, true, ON, &settings.honour_newlines, &exp_hnl},
	{"h", PRO_FUNCTION, 0, ONOFF_NA, (int *)usage, &exp_version},
	
	/* This is now the default. */
	{"gnu", PRO_SETTINGS, 0, ONOFF_NA, GNU_SETTINGS_STRING, &exp_gnu},
	{"fca", PRO_BOOL, false, ON, &settings.format_comments, &exp_fca},
	{"fc1", PRO_BOOL, false, ON, &settings.format_col1_comments, &exp_fc1},
	{"eei", PRO_BOOL, false, ON, &settings.extra_expression_indent, &exp_eei},
	{"dj", PRO_BOOL, false, ON, &settings.ljust_decl, &exp_dj},
	{"di", PRO_INT, 2, ONOFF_NA, &settings.decl_indent, &exp_di},
	{"d", PRO_INT, 0, ONOFF_NA, &settings.unindent_displace, &exp_d},
	{"cs", PRO_BOOL, true, ON, &settings.cast_space, &exp_cs},
	{"cp", PRO_INT, 1, ONOFF_NA, &settings.else_endif_col, &exp_cp},
	{"cli", PRO_INT, 0, ONOFF_NA, &settings.case_indent, &exp_cli},
	{"ci", PRO_INT, 0, ONOFF_NA, &settings.continuation_indent, &exp_ci},
	{"ce", PRO_BOOL, false, ON, &settings.cuddle_else, &exp_ce},
	{"cdw", PRO_BOOL, false, ON, &settings.cuddle_do_while, &exp_cdw},
	{"cdb", PRO_BOOL, false, ON, &settings.comment_delimiter_on_blankline, &exp_cdb},
	{"cd", PRO_INT, 33, ONOFF_NA, &settings.decl_com_ind, &exp_cd},
	{"cbi", PRO_INT, -1, ONOFF_NA, &settings.case_brace_indent, &exp_cbi},
	{"c++", PRO_BOOL, false, ON, &settings.c_plus_plus, &exp_cpp},
	{"c", PRO_INT, 33, ONOFF_NA, &settings.com_ind, &exp_c},
	{"bs", PRO_BOOL, false, ON, &settings.blank_after_sizeof, &exp_bs},
	{"brs", PRO_BOOL, false, ON, &settings.braces_on_struct_decl_line, &exp_bls},
	{"bls", PRO_BOOL, false, OFF, &settings.braces_on_struct_decl_line, &exp_bls},
	{"brf", PRO_BOOL, false, ON, &settings.braces_on_func_def_line, &exp_blf},
	{"blf", PRO_BOOL, false, OFF, &settings.braces_on_func_def_line, &exp_blf},
	{"bli", PRO_INT, 2, ONOFF_NA, &settings.brace_indent, &exp_bli},
	{"br", PRO_BOOL, false, ON, &settings.btype_2, &exp_bl},
	{"bl", PRO_BOOL, false, OFF, &settings.btype_2, &exp_bl},
	{"bfda", PRO_BOOL, false, ON, &settings.break_function_decl_args, &exp_bfda},
	{"bfde", PRO_BOOL, false, ON, &settings.break_function_decl_args_end, &exp_bfde},
	{"bc", PRO_BOOL, true, OFF, &settings.leave_comma, &exp_bc},
	{"bbo", PRO_BOOL, true, ON, &settings.break_before_boolean_operator, &exp_bbo},
	{"bbb", PRO_BOOL, false, ON, &settings.blanklines_before_blockcomments, &exp_bbb},
	{"bap", PRO_BOOL, true, ON, &settings.blanklines_after_procs, &exp_bap},
	{"badp", PRO_BOOL, false, ON, &settings.blanklines_after_declarations_at_proctop, &exp_badp},
	{"bad", PRO_BOOL, false, ON, &settings.blanklines_after_declarations, &exp_bad},
	{"bacc", PRO_BOOL, false, ON, &settings.blanklines_around_conditional_compilation, &exp_bacc},
	{"T", PRO_KEY, 0, ONOFF_NA, 0, &exp_T},
	{"ppi", PRO_INT, 0, ONOFF_NA, &settings.force_preproc_width, &exp_ppi},
	
	{0, PRO_IGN, 0, ONOFF_NA, 0, 0}
};

#endif

typedef struct long_option_conversion {
	char *long_name;
	char *short_name;
} long_option_conversion_ty;

const long_option_conversion_ty option_conversions[] = {
	{"version", "V"},
	{"verbose", "v"},
	{"usage", "h"},
	{"use-tabs", "ut"},
	{"tab-size", "ts"},
	{"swallow-optional-blank-lines", "sob"},
	{"struct-brace-indentation", "sbi"},
	{"start-left-side-of-comments", "sc"},
	{"standard-output", "st"},
	{"space-special-semicolon", "ss"},
	{"space-after-while", "saw"},
	{"space-after-procedure-calls", "pcs"},
	{"space-after-parentheses", "prs"},
	{"space-after-if", "sai"},
	{"space-after-for", "saf"},
	{"space-after-cast", "cs"},
	{"remove-preprocessor-space", "nlps"},
	{"procnames-start-lines", "psl"},
#ifdef PRESERVE_MTIME
	{"preserve-mtime", "pmt"},
#endif
	{"paren-indentation", "pi"},
	{"parameter-indentation", "ip"},
	{"output-file", "o"},
	{"output", "o"},
	{"original-style", "orig"},
	{"original", "orig"},
	{"no-verbosity", "nv"},
	{"no-tabs", "nut"},
	{"no-space-after-while", "nsaw"},
	{"no-space-after-parentheses", "nprs"},
	{"no-space-after-if", "nsai"},
	{"no-space-after-function-call-names", "npcs"},
	{"no-space-after-for", "nsaf"},
	{"no-space-after-casts", "ncs"},
	{"no-parameter-indentation", "nip"},
	{"no-extra-expression-indentation", "neei"},
	{"no-comment-delimiters-on-blank-lines", "ncdb"},
	{"no-blank-lines-before-block-comments", "nbbb"},
	{"no-blank-lines-after-procedures", "nbap"},
	{"no-blank-lines-after-procedure-declarations", "nbadp"},
	{"no-blank-lines-after-ifdefs", "nbacc"},
	{"no-blank-lines-after-declarations", "nbad"},
	{"no-blank-lines-after-commas", "nbc"},
	{"no-blank-before-sizeof", "nbs"},
	{"no-Bill-Shannon", "nbs"},
	{"label-offset", "il"},
	{"line-length", "l"},
	{"line-comments-indentation", "d"},
	{"linux-style", "linux"},
	{"left-justify-declarations", "dj"},
	{"leave-preprocessor-space", "lps"},
	{"leave-optional-blank-lines", "nsob"},
	{"kernighan-and-ritchie-style", "kr"},
	{"kernighan-and-ritchie", "kr"},
	{"k-and-r-style", "kr"},
	{"indent-label", "il"},
	{"indentation-level", "i"},
	{"indent-level", "i"},
	{"ignore-profile", "npro"},
	{"ignore-newlines", "nhnl"},
	{"honour-newlines", "hnl"},
	{"help", "h"},
	{"gnu-style", "gnu"},
	{"format-first-column-comments", "fc1"},
	{"format-all-comments", "fca"},
	{"extra-expression-indentation", "eei"},
	{"else-endif-column", "cp"},
	{"dont-star-comments", "nsc"},
	{"dont-space-special-semicolon", "nss"},
	{"dont-line-up-parentheses", "nlp"},
	{"dont-left-justify-declarations", "ndj"},
	{"dont-indent-parameters", "nip"},
	{"dont-format-first-column-comments", "nfc1"},
	{"dont-format-comments", "nfca"},
	{"dont-cuddle-else", "nce"},
	{"dont-cuddle-do-while", "ncdw"},
	{"dont-break-procedure-type", "npsl"},
	{"dont-break-function-decl-args", "nbfda"},
	{"dont-break-function-decl-args-end", "nbfde"},
	{"declaration-indentation", "di"},
	{"declaration-comment-column", "cd"},
	{"cuddle-else", "ce"},
	{"cuddle-do-while", "cdw"},
	{"continue-at-parentheses", "lp"},
	{"continuation-indentation", "ci"},
	{"comment-line-length", "lc"},
	{"comment-indentation", "c"},
	{"comment-delimiters-on-blank-lines", "cdb"},
	{"case-indentation", "cli"},
	{"case-brace-indentation", "cbi"},
	{"c-plus-plus", "c++"},
	{"break-function-decl-args", "bfda"},
	{"break-function-decl-args-end", "bfde"},
	{"break-before-boolean-operator", "bbo"},
	{"break-after-boolean-operator", "nbbo"},
	{"braces-on-struct-decl-line", "brs"},
	{"braces-on-func-def-line", "brf"},
	{"braces-on-if-line", "br"},
	{"braces-after-struct-decl-line", "bls"},
	{"braces-after-func-def-line", "blf"},
	{"braces-after-if-line", "bl"},
	{"brace-indent", "bli"},
	{"blank-lines-before-block-comments", "bbb"},
	{"blank-lines-after-procedures", "bap"},
	{"blank-lines-after-procedure-declarations", "badp"},
	{"blank-lines-after-ifdefs", "bacc"},
	{"blank-lines-after-declarations", "bad"},
	{"blank-lines-after-commas", "bc"},
	{"blank-before-sizeof", "bs"},
	{"berkeley-style", "orig"},
	{"berkeley", "orig"},
	{"Bill-Shannon", "bs"},
	{"preprocessor-indentation", "ppi"},

	{0, 0},
};

char *labbuf = NULL;
char *s_lab = NULL;
char *e_lab = NULL;
char *l_lab = NULL;

char *codebuf = NULL;
char *s_code = NULL;
char *e_code = NULL;
char *l_code = NULL;

char *combuf = NULL;
char *s_com = NULL;
char *e_com = NULL;
char *l_com = NULL;

/* If set, start of corresponding code in token buffer... */
char *s_code_corresponds_to = NULL;

/* Saved value of buf_ptr when taking input from save_com */
char *bp_save = NULL;

/* similarly saved value of buf_end */
char *be_save = NULL;

/* count of lines with code */
int code_lines = 0;

/* the current input line number. */
int line_no = 0;

/* when true and not in parens, break after a comma */
int break_comma = 0;

int n_real_blanklines = 0;

/* True if there is an embedded comment on this code line */
int embedded_comment_on_line = 0;

int else_or_endif = 0;

/* Structure poemation levels */
int *di_stack = NULL;

/* Currently allocated size of di_stack. */
int di_stack_alloc = 0;

int squest = 0;

/* Size of the input program, not including the NEWLINE EOS we add at the end */
unsigned long in_prog_size = 0U;

/* Pointer to the null-terminated input program */
char *in_prog = NULL;

/* The position that we will line the current line up with when it comes time to print it (if we are lining up to parentheses). */
int paren_target = 0;

int prefix_blankline_requested = 0;
codes_ty prefix_blankline_requested_code;

int postfix_blankline_requested = 0;
codes_ty postfix_blankline_requested_code;

/* points to current input file name */
char *in_name = 0;

file_buffer_ty *current_input = NULL;

static char *out_name = 0;

static int input_files = 0;

static char **in_file_names = NULL;

static int max_input_files = 128;

/*
 * Buffer in which to save a comment which occurs between an if(), while(), etc.,
 * and the statement following it. Note: the fact that we point into this buffer, and that we might realloc() it (via the need_chars macro) is a bad thing
 * (since when the buffer is realloc'd its address might change, making any pointers into it point to garbage),
 * but since the filling of the buffer (hence the need_chars) and the using of the buffer
 * (where buf_ptr points into it) occur at different times, we can get away with it (it would not be trivial to fix).
 */
buf_ty save_com;

static templ_ty *user_specials = 0;

backup_mode_ty version_control = unknown;

static version_control_values_ty values[] = {
	{none, "never"},
	
	{none, "none"},
	
	{simple, "simple"},
	
	{numbered_existing, "existing"},
	
	{numbered_existing, "nil"},
	
	{numbered, "numbered"},
	
	{numbered, "t"},
	
	{unknown, 0}
};

static buf_break_st_ty *buf_break_list = NULL;

buf_break_st_ty *buf_break = NULL;

/*
 * All manipulations of the parser state occur at the top of stack (tos). A stack is kept for conditional compilation
 * (unrelated to the p_stack, il, & cstk stacks) --it is implemented as a linked list via the next field.
 */
parser_state_ty *ppst = NULL;

char *xmalloc(unsigned size)
{
	char *val = (char *)calloc(1, size);
	if (!val) {
		fprintf(stderr, _("poem: Virtual memory exhausted.\n"));
		exit(system_error);
	}
#if defined (DEBUG)
	/* Fill it with garbage to detect code which depends on stuff being zero-filled. */
	memset(val, 'x', size);
#endif
	return val;
}

char *xrealloc(char *ptr, unsigned size)
{
	char *val = (char *)realloc(ptr, size);
	if (!val) {
		fprintf(stderr, _("poem: Virtual memory exhausted.\n"));
		exit(system_error);
	}
	return val;
}

void message(char *kind, char *string, unsigned *a0, unsigned *a1)
{
	if (kind)
		fprintf(stderr, _("poem: %s:%d: %s:"), in_name, line_no, kind);
	fprintf(stderr, string, a0, a1);
	fprintf(stderr, "\n");
}

void fatal(const char *string, const char *a0)
{
	fprintf(stderr, _("poem: Fatal Error: "));
	fprintf(stderr, string, a0);
	fprintf(stderr, "\n");
#ifdef DEBUG
	abort();
#endif
	if (errno) {
		fprintf(stderr, _("poem: System Error: "));
		perror(0);
	}
	exit(poem_fatal);
}

/*
 * returns `true' when `b1' is a better place to break the code than `b2'.
 * `b1' must be newer. When `lineup_to_parens' is true, do not break more than 1 level deeper
 * unless that doesn't cost us "too much" indentation. What is "too much" is determined in a fuzzy way as follows:
 * Consider the example,
 *   while (!(!(!(!(!(!(mask
 *                      || (((a_very_long_expression_that_cant_be_broken
 * here we prefer to break after `mask' instead of after `while'.
 * This is because the `target_col' is pretty close to the break point of the `while': "mask"->target_col - "while"->col == 15 == "mask"->level * 2 + 1.
 */
static BOOLEAN better_break(buf_break_st_ty *b1, const buf_break_st_ty *b2)
{
	static int first_level;
	BOOLEAN is_better;

	if (!b2) {
		b1->first_level = first_level = b1->level;
		is_better = true;
	} else {
		if (b2->target_col >= b2->col + 1)
			is_better = true;
		else if (settings.honour_newlines && b2->priority_newline)
			is_better = false;
		else if (settings.honour_newlines && b1->priority_newline)
			is_better = true;
		else {
			int only_parens_till_b2 = 0;

			is_better = b1->priority > b2->priority;

			if (is_better) {
				char *p;

				for (p = &s_code[b2->offset]; p >= s_code; --p) {
					if (*p == '!')
						--p;

					if (*p != '(')
						break;
				}

				if (p < s_code)
					only_parens_till_b2 = 1;
			}

			if (
				settings.lineup_to_parens
				&& b1->level > first_level + 1
				&& !(
					only_parens_till_b2
					&& b1->target_col <= b2->col + (1 + 2 * b1->level)
				)
				&& b1->level > b2->level
			)
				is_better = false;
		}
		if (is_better)
			b1->first_level = first_level;
	}
	return is_better;
}

/*
 * Calculate break priority.
 *
 * Example:
 *                              e_code          (`s_code' buffer, at the moment
 *                                               set_buf_break() is called)
 *                              ptr             (`s_code' buffer)
 *                              corresponds_to  (input buffer)
 * Left most column             col+1           (the column (31 here)).
 * |                             |
 * 1234567890123456789012345678901234567890
 *         if (!(mask[0] == '\\0'
 *               |
 *             target_col (assuming `lineup_to_parens' is true in this example)
 *            1 2    3 2         | level == 2
 *         <--------------------->
 *          priority_code_length
 */
static void set_priority(buf_break_st_ty *bb)
{
	/* use the length of priority code to judge priority of bb */
	bb->priority = bb->priority_code_length;

	switch (bb->priority_code) {
		case bb_semicolon:
			bb->priority += 6000;
			break;
		case bb_before_boolean_binary_op:
			bb->priority += 5000;
			break;
		case bb_after_boolean_binary_op:
			if (bb->priority_code_length > 2)
				bb->priority += 5000;

			if (settings.break_before_boolean_operator)
				bb->priority -= 3;
			break;
		case bb_after_equal_sign:
			bb->priority += 4000;
			break;
		case bb_attribute:
			bb->priority += 3000;
			break;
		case bb_comma:
			bb->priority += 2000;
			break;
		case bb_comparisation:
			bb->priority += 1000;
			break;
		case bb_proc_call:
			bb->priority -= 1000;
			break;
		case bb_operator6:
			bb->priority += 600;
			break;
		case bb_operator5:
			bb->priority += 500;
			break;
		case bb_operator4:
			bb->priority += 400;
			break;
		case bb_operator2:
			bb->priority += 200;
			break;
		case bb_doublecolon:
			bb->priority += 100;
			break;
		default:
			break;
	}
}

/* This function is called at every position where we possibly want to break a line (if it gets too long). */
void set_buf_break(bb_code_ty code, int paren_targ)
{
	int target_col, level;
	int code_target = compute_code_target(paren_targ);
	buf_break_st_ty *bb;

	/* First, calculate the column that code following e_code would be printed in if we'd break the line here. This is done quite simular to compute_code_target(). */
	/* Base indentation level (number of open left-braces) */
	target_col = ppst->i_l_follow + 1;

	/* Did we just parse a brace that will be put on the next line by this line break? */
	if (*token == '{')
		/* Use `ind_size' because this only happens for the first brace of initializer blocks. */
		target_col -= settings.ind_size;

	/* Number of open brackets */
	level = ppst->p_l_follow;

	/* Did we just parse a bracket that will be put on the next line by this line break? */
	if (*token == '(' || *token == '[')
		/* then don't take it into account */
		--level;
	
	/* Procedure name of function declaration? */
	if (ppst->procname[0] && token == ppst->procname)
		target_col = 1;
	/* no open brackets? */
	else if (level == 0) {
		/* breaking a statement? */
		if (ppst->in_stmt)
			target_col += settings.continuation_indent;
	} else if (!settings.lineup_to_parens)
		target_col += settings.continuation_indent + settings.paren_indent * (level - 1);
	else {
		if (ppst->paren_indents[level - 1] < 0)
			target_col = -ppst->paren_indents[level - 1];
		else
			target_col = code_target + ppst->paren_indents[level - 1];
	}

	/* Store the position of `e_code' as the place to break this line. */
	bb = (buf_break_st_ty *)xmalloc(sizeof(buf_break_st_ty));
	bb->offset = e_code - s_code;
	bb->level = level;
	bb->target_col = target_col;
	bb->corresponds_to = token;
	*e_code = 0;
	bb->col = count_columns(code_target, s_code, NULL_CHAR) - 1;

	/* calculate default priority. */
	bb->priority_code_length = e_code - s_code;
	bb->priority_newline = ppst->last_saw_nl && !ppst->broken_at_non_nl;

	if (buf_break)
		bb->first_level = buf_break->first_level;
	
	switch (ppst->last_token) {

		case binary_op:
			if (
				e_code - s_code >= 3
				&& e_code[-3] == ' '
				&& (
					(e_code[-1] == '&' && e_code[-2] == '&')
					|| (e_code[-1] == '|' && e_code[-2] == '|')
				)
			)
				bb->priority_code = bb_after_boolean_binary_op;
			else if (
				e_code - s_code >= 2
				&& e_code[-1] == '='
				&& (
					e_code[-2] == ' '
					|| (
						e_code - s_code >= 3
						&& e_code[-3] == ' '
						&& (
							e_code[-2] == '%'
							|| e_code[-2] == '^'
							|| e_code[-2] == '&'
							|| e_code[-2] == '*'
							|| e_code[-2] == '-'
							|| e_code[-2] == '+'
							|| e_code[-2] == '|'
						)
					)
				)
			)
				bb->priority_code = bb_after_equal_sign;
			else if (
				(
					e_code - s_code >= 2
					&& e_code[-2] == ' '
					&& (e_code[-1] == '<' || e_code[-1] == '>')
				)
				|| (
					e_code - s_code >= 3
					&& e_code[-3] == ' '
					&& e_code[-1] == '='
					&& (
						e_code[-2] == '='
						|| e_code[-2] == '!'
						|| e_code[-2] == '<'
						|| e_code[-2] == '>'
					)
				)
			)
				bb->priority_code = bb_comparisation;
			else if (e_code[-1] == '+' || e_code[-1] == '-')
				bb->priority_code = bb_operator6;
			else if (e_code[-1] == '*' || e_code[-1] == '/' || e_code[-1] == '%')
				bb->priority_code = bb_operator5;
			else
				bb->priority_code = bb_binary_op;

			break;
		case comma:
			bb->priority_code = bb_comma;
			break;
		default:
			if (
				code == bb_binary_op
				&& (*token == '&' || *token == '|')
				&& *token == token[1]
			)
				bb->priority_code = bb_before_boolean_binary_op;
			else if (e_code[-1] == ';')
				bb->priority_code = bb_semicolon;
			else {
				bb->priority_code = code;
				
				/* . -> .* or ->* */
				if (code == bb_struct_delim)
					bb->priority_code = e_code[-1] == '*' ? bb_operator4 : bb_operator2;
			}
	}
	
	set_priority(bb);

	/* add buf_break to the list */
	if (buf_break_list)
		buf_break_list->next = bb;

	bb->prev = buf_break_list;
	bb->next = NULL;
	buf_break_list = bb;

	if (!buf_break || bb->col <= settings.max_col) {
		if (better_break(bb, buf_break)) {
			/* found better buf_break.  Get rid of all previous possible breaks. */
			buf_break = bb;

			for (bb = bb->prev; bb;) {
				buf_break_st_ty *obb = bb;
				bb = bb->prev;
				free(obb);
			}
			buf_break->prev = NULL;
		}
	}
}

void clear_buf_break_list(BOOLEAN *pbreak_line)
{
	buf_break_st_ty *bb;

	for (bb = buf_break_list; bb;) {
		buf_break_st_ty *obb = bb;
		bb = bb->prev;
		free(obb);
	}

	buf_break = buf_break_list = NULL;
	*pbreak_line = false;
}

/**
 * @verbatim
 *        prev_code_target
 *        |                prev_code_target + offset
 *        |                |
 * <----->if ((aaa == bbb) && xxx
 *            && xxx
 *            |
 *            new_code_target
 * @endverbatim
 */
static void set_next_buf_break(int prev_code_target, int new_code_target, int offset, BOOLEAN *pbreak_line)
{
	buf_break_st_ty *bb;

	/* reset first_level */
	better_break(buf_break, NULL);

	if (buf_break_list == buf_break)
		clear_buf_break_list(pbreak_line);
	else {
		/* correct all elements of the remaining buf breaks: */
		for (bb = buf_break_list; bb; bb = bb->prev) {
			
			if (bb->target_col > buf_break->target_col && settings.lineup_to_parens)
				bb->target_col -= ((prev_code_target + offset) - new_code_target);

			bb->col -= ((prev_code_target + offset) - new_code_target);
			bb->offset -= offset;
			bb->priority_code_length -= offset;
			bb->first_level = buf_break->first_level;

			if (!buf_break->priority_newline)
				bb->priority_newline = false;

			set_priority(bb);

			if (bb->prev == buf_break)
				break;
		}

		free(buf_break);

		/* set buf_break to first break in the list */
		buf_break = bb;

		buf_break->prev = NULL;

		/* Find a better break of the existing breaks */
		for (bb = buf_break; bb; bb = bb->next) {
			
			if (bb->col > settings.max_col)
				continue;

			if (better_break(bb, buf_break)) {
				/* found better buf_break.  Get rid of all previous possible breaks. */
				buf_break = bb;

				for (bb = bb->prev; bb;) {
					buf_break_st_ty *obb = bb;

					bb = bb->prev;
					free(obb);
				}
				
				bb = buf_break;
				buf_break->prev = NULL;
			}
		}
	}
}

static int pad_output(int current_column, int target_column)
{
	if (current_column < target_column) {
		if (settings.use_tabs && settings.tabsize > 1) {
			int offset = settings.tabsize - (current_column - 1) % settings.tabsize;

			while (current_column + offset <= target_column) {
				putc(TAB, output);
				current_column += offset;
				offset = settings.tabsize;
			}
		}

		while (current_column < target_column) {
			putc(' ', output);
			current_column++;
		}
	}
	return current_column;
}

static void output_substring(FILE *file, const char *begin, const char *end)
{
	const char *p;

	for (p = begin; p < end; p++)
		putc(*p, file);
}

static BOOLEAN is_comment_start(const char *p)
{
	return *p == '/' && (*(p + 1) == '*' || *(p + 1) == '/');
}

static int dump_line_label(void)
{
	int cur_col;

	while (e_lab > s_lab && (e_lab[-1] == ' ' || e_lab[-1] == TAB))
		e_lab--;

	cur_col = pad_output(1, compute_label_target());

	if (settings.force_preproc_width > 0 && s_lab[0] == '#') {
		int preproc_postcrement;
		char *p = s_lab + 1;

		while (*p == ' ')
			p++;

		preproc_postcrement = settings.force_preproc_width;

		if (!strncmp(p, "else", 4))
			preproc_indent -= settings.force_preproc_width;
		else if (!strncmp(p, "if", 2) || !strncmp(p, "ifdef", 5)) {
			
		} else if (!strncmp(p, "elif", 4))
			preproc_indent -= settings.force_preproc_width;
		else if (!strncmp(p, "endif", 5)) {
			preproc_indent -= settings.force_preproc_width;
			preproc_postcrement = 0;
		} else
			preproc_postcrement = 0;

		if (preproc_indent == 0)
			fprintf(output, "#");
		else
			fprintf(output, "#%*s", preproc_indent, " ");

		fprintf(output, "%.*s", (int)(e_lab - p), p);

		cur_col = count_columns(cur_col + preproc_indent + 1, p, NULL_CHAR);
		preproc_indent += preproc_postcrement;
	} else if (
		*s_lab == '#'
		&& (
			!strncmp(s_lab + 1, "else", 4)
			|| !strncmp(s_lab + 1, "endif", 5)
		)
	) {
		char *s = s_lab;

		/* don't include EOL in the comment */
		if (e_lab[-1] == EOL)
			e_lab--;
		
		do {
			putc(*s++, output);
			++cur_col;
		} while (s < e_lab && 'a' <= *s && *s <= 'z');

		while ((*s == ' ' || *s == TAB) && s < e_lab)
			s++;

		if (s < e_lab) {
			if (settings.tabsize > 1)
				cur_col = pad_output(cur_col, cur_col + settings.tabsize - (cur_col - 1) % settings.tabsize);
			else
				cur_col = pad_output(cur_col, cur_col + 2);
			
			if (is_comment_start(s))
				fprintf(output, "%.*s", (int)(e_lab - s), s);
			else
				fprintf(output, "/* %.*s */", (int)(e_lab - s), s);
		}
	} else {
		fprintf(output, "%.*s", (int)(e_lab - s_lab), s_lab);
		cur_col = count_columns(cur_col, s_lab, NULL_CHAR);
	}

	return cur_col;
}

static int count_parens(const char *str)
{
	int paren_level = 0;

	while (*str) {
		switch (*str) {
			case '(':
			case '[':
				paren_level++;
				break;
			case ')':
			case ']':
				paren_level--;
				break;
			default:
				break;
		}
		str++;
	}
	return paren_level;
}

static void dump_line_code(int *pcur_col, int *pnot_truncated, int paren_targ, BOOLEAN *pbreak_line, int target_col_break)
{
	int paren_level = 0;
	int target_col = 0;
	int i;

	if (s_code == e_code)
		return;

	/* if a comment begins this line, then poem it to the correct column for comments, otherwise the line starts with code, so poem it for code */
	if (embedded_comment_on_line == 1)
		target_col = ppst->com_col;
	else if (target_col_break != -1)
		target_col = target_col_break;
	else
		target_col = compute_code_target(paren_targ);
	
	if (paren_level > 0)
		target_col += ppst->paren_indents[ppst->p_l_follow + paren_level - 1];
	
	/* If a line ends in an lparen character, the following line should not line up with the parenthesis, but should be indented by the usual amount. */
	if (ppst->last_token == lparen)
		ppst->paren_indents[ppst->p_l_follow - 1] += settings.ind_size - 1;

	*pcur_col = pad_output(*pcur_col, target_col);

	if (*pbreak_line && s_com == e_com && buf_break->target_col <= buf_break->col) {
		int offset;
		int len;
		char c;
		char *ptr = s_code + buf_break->offset;

		if (*ptr != ' ')
			--ptr;

		/* Add target_col (and negate) the brackets that are actually printed.  The remaining brackets must be given an offset of */
		offset = ptr - s_code + 1;

		for (i = 0; i < ppst->p_l_follow; i++)
			if (ppst->paren_indents[i] >= 0) {
				if (ppst->paren_indents[i] < ptr - s_code)
					ppst->paren_indents[i] = -(ppst->paren_indents[i] + target_col);
				else
					ppst->paren_indents[i] -= offset;
			}

		for (i = ppst->p_l_follow; i < ppst->paren_indents_size; ++i)
			if (ppst->paren_indents[i] >= ptr - s_code)
				ppst->paren_indents[i] -= offset;

		output_substring(output, s_code, s_code + buf_break->offset);

		/* Backup the offset character of s_code temporarily */
		c = s_code[buf_break->offset];
		s_code[buf_break->offset] = '\0';
		*pcur_col = count_columns(*pcur_col, s_code, NULL_CHAR);

		paren_level += count_parens(s_code);

		/* restore the offset character of s_code */
		s_code[buf_break->offset] = c;

		*pnot_truncated = 0;

		len = e_code - ptr - 1;
		memmove(s_code, ptr + 1, len);

		e_code = s_code + len;
		*e_code = '\0';

		s_code_corresponds_to = buf_break->corresponds_to;
		prev_target_col_break = buf_break->target_col;

		if (!buf_break->priority_newline)
			ppst->broken_at_non_nl = true;
		
		set_next_buf_break(target_col, buf_break->target_col, offset, pbreak_line);

		buf_break_used = 1;

		*pbreak_line = buf_break != NULL && output_line_length() > settings.max_col;
	} else {
		for (i = 0; i < ppst->p_l_follow; i++)
			if (ppst->paren_indents[i] >= 0)
				ppst->paren_indents[i] = -(ppst->paren_indents[i] + target_col);

		output_substring(output, s_code, e_code);

		*pcur_col = count_columns(*pcur_col, s_code, NULL_CHAR);
		clear_buf_break_list(pbreak_line);
	}
}

static void dump_line_comment(int *pcur_col)
{
	if (s_com != e_com) {
		int target = ppst->com_col;
		char *com_st = s_com;

		if (*pcur_col > target) {
			putc(EOL, output);
			*pcur_col = 1;
			++out_lines;
		}

		*pcur_col = pad_output(*pcur_col, target);
		fwrite(com_st, e_com - com_st, 1, output);
		*pcur_col += e_com - com_st;
		com_lines++;
	} else if (embedded_comment_on_line)
		com_lines++;
}

void dump_line(int force_nl, int *paren_targ, BOOLEAN *pbreak_line)
{
	int cur_col;
	int not_truncated = 1;
	int target_col_break = -1;

	if (buf_break_used) {
		buf_break_used = 0;
		target_col_break = prev_target_col_break;
	} else if (force_nl)
		ppst->broken_at_non_nl = false;

	if (
		ppst->procname[0]
		&& !ppst->classname[0]
		&& s_code_corresponds_to == ppst->procname
	)
		ppst->procname = "\0";
	else if (
		ppst->procname[0]
		&& ppst->classname[0]
		&& s_code_corresponds_to == ppst->classname
	)
		ppst->procname = ppst->classname = "\0";

	if (s_code == e_code && s_lab == e_lab && s_com == e_com) {
		if (ppst->use_ff) {
			putc('\014', output);
			ppst->use_ff = false;
		} else
			n_real_blanklines++;
	} else {
		if ((prefix_blankline_requested && !n_real_blanklines) || (settings.swallow_optional_blanklines && n_real_blanklines > 1))
			n_real_blanklines = 1;

		while (--n_real_blanklines >= 0)
			putc(EOL, output);

		n_real_blanklines = 0;
		
		if (e_lab != s_lab || e_code != s_code)
			++code_lines;

		cur_col = e_lab != s_lab ? dump_line_label() : 1;
		
		ppst->pcase = false;

		while (*(e_code - 1) == ' ' && e_code > s_code)
			*(--e_code) = NULL_CHAR;

		dump_line_code(&cur_col, &not_truncated, *paren_targ, pbreak_line, target_col_break);

		dump_line_comment(&cur_col);

#if 0
		if (s_com != e_com) {
			int target = ppst->com_col;
			char *com_st = s_com;

			if (cur_col > target) {
				putc(EOL, output);
				cur_col = 1;
				++out_lines;
			}

			cur_col = pad_output(cur_col, target);
			fwrite(com_st, e_com - com_st, 1, output);
			cur_col += e_com - com_st;
			com_lines++;
		} else if (embedded_comment_on_line)
			com_lines++;
#endif

		embedded_comment_on_line = 0;

		if (ppst->use_ff) {
			putc('\014', output);
			ppst->use_ff = false;
		} else
			putc(EOL, output);

		++out_lines;

		if (ppst->just_saw_decl == 1 && settings.blanklines_after_declarations) {
			prefix_blankline_requested = 1;
			prefix_blankline_requested_code = decl;
			ppst->just_saw_decl = 0;
		} else {
			prefix_blankline_requested = postfix_blankline_requested;
			prefix_blankline_requested_code = postfix_blankline_requested_code;
		}

		postfix_blankline_requested = 0;
	}

	/* if we are in the middle of a declaration, remember that fact for proper comment indentation */
	ppst->decl_on_line = ppst->in_decl;

	/* next line should be indented if we have not completed this stmt */
	ppst->ind_stmt = ppst->in_stmt;

	e_lab = s_lab;
	*s_lab = '\0';

	if (not_truncated) {
		e_code = s_code;
		*s_code = '\0';
		s_code_corresponds_to = NULL;
	}

	e_com = s_com;
	*s_com = '\0';

	ppst->ind_level = ppst->i_l_follow;
	ppst->paren_level = ppst->p_l_follow;

	if (ppst->paren_level > 0) {
		/*
		 * if we broke the line and the following line will begin with a rparen, the indentation is set for
		 * the column of the rparen *before* the break - reset the column to the position after the break.
		 */
		if (!not_truncated && (*s_code == '(' || *s_code == '[') && ppst->paren_level >= 2)
			*paren_targ = -ppst->paren_indents[ppst->paren_level - 2];
		else
			*paren_targ = -ppst->paren_indents[ppst->paren_level - 1];
	} else
		*paren_targ = 0;

	if (inhibited) {
		char *p = cur_line;

		while (--n_real_blanklines >= 0)
			putc(EOL, output);

		n_real_blanklines = 0;

		do {
			while (*p != '\0' && *p != EOL)
				putc(*p++, output);

			if (*p == '\0' && (unsigned long)(p - current_input->data) == current_input->size) {
				buf_ptr = buf_end = in_prog_pos = p;
				had_eof = true;
				return;
			}

			if (*p == EOL) {
				cur_line = p + 1;
				line_no++;
			}

			putc(*p++, output);

			while (*p == ' ' || *p == TAB)
				putc(*p++, output);

			if (is_comment_start(p)) {
				putc(*p++, output);
				putc(*p++, output);

				while (*p == ' ' || *p == TAB)
					putc(*p++, output);

				if (!strncmp(p, "*POEM-ON*", 9)) {
					do {
						while (*p != '\0' && *p != EOL)
							putc(*p++, output);

						if (*p == '\0' && ((unsigned long)(p - current_input->data) == current_input->size)) {
							buf_ptr = buf_end = in_prog_pos = p;
							had_eof = true;
							return;
						} else {
							if (*p == EOL) {
								inhibited = false;
								cur_line = p + 1;
								line_no++;
							}
							putc(*p++, output);
						}
					} while (inhibited);
				}
			}
		} while (inhibited);

		buf_ptr = buf_end = in_prog_pos = cur_line;

		fill_buffer();
	}

	/* Output the rest already if we really wanted a new-line after this code. */
	if (buf_break_used && s_code != e_code && force_nl) {
		prefix_blankline_requested = 0;
		dump_line(true, paren_targ, pbreak_line);
	}

	return;
}

void flush_output(void)
{
	fflush(output);
}

void open_output(const char *filename, const char *mode)
{
	if (filename == NULL)
		output = stdout;
	else {
		output = fopen(filename, mode);

		if (output == NULL) {
			fprintf(stderr, _("poem: can't create %s\n"), filename);
			exit(poem_fatal);
		}
	}
}

void reopen_output_trunc(const char *filename)
{
	output = freopen(filename, "w", output);
}

void close_output(struct stat *file_stats, const char *filename)
{
	if (output != stdout) {
		if (fclose(output))
			fatal(_("Can't close output file %s"), filename);
		else {
#ifdef PRESERVE_MTIME
			if (file_stats) {
				struct utimbuf buf;

				buf.actime = time(NULL);
				buf.modtime = file_stats->st_mtime;
				if (utime(filename, &buf))
					WARNING(_("Can't preserve modification time on output file %s"), filename, 0);
			}
#endif
		}
	}
}

void inhibit_poeming(BOOLEAN flag)
{
	inhibited = flag;
}

/* Return the column in which we should place the code in s_code.  */
int compute_code_target(int paren_targ)
{
	int target_col;

	if (buf_break_used)
		return prev_target_col_break;
	
	if (ppst->procname[0] && s_code_corresponds_to == ppst->procname) {
		target_col = 1;

		if (!ppst->paren_level)
			return target_col;
	} else
		target_col = ppst->ind_level + 1;

	if (!ppst->paren_level) {
		if (ppst->ind_stmt)
			target_col += settings.continuation_indent;

		return target_col;
	}

	/* If true, continued code within parens will be lined up to the open paren */
	if (!settings.lineup_to_parens)
		return target_col + settings.continuation_indent + settings.paren_indent * (ppst->paren_level - 1);

	return paren_targ;
}

int count_columns(int column, char *bp, int stop_char)
{
	while (*bp != stop_char && *bp != NULL_CHAR) {
		switch (*bp++) {
			case EOL:
			case '\f':
				column = 1;
				break;
			case TAB:
				column += settings.tabsize - (column - 1) % settings.tabsize;
				break;
			case 010:
				--column;
				break;
			default:
				++column;
				break;
		}
	}
	return column;
}

int compute_label_target(void)
{
	if (*s_lab == '#')
		return 1;

	if (ppst->pcase)
		return ppst->cstk[ppst->tos] + 1;

	if (settings.c_plus_plus && ppst->in_decl)
		return 1;

	if (settings.label_offset < 0)
		return ppst->ind_level + settings.label_offset + 1;

	return settings.label_offset + 1;
}

int output_line_length(void)
{
	int code_length = 0;
	int com_length = 0;
	int length;

	length = s_lab == e_lab ? 0 : count_columns(compute_label_target(), s_lab, EOL) - 1;

	if (s_code != e_code) {
		int code_col = compute_code_target(paren_target);
		code_length = count_columns(code_col, s_code, EOL) - code_col;
	}

	if (s_com != e_com) {
		int com_col = ppst->com_col;
		com_length = count_columns(com_col, s_com, EOL) - com_col;
	}

	if (code_length != 0) {
		length += compute_code_target(paren_target) - 1 + code_length;
		if (embedded_comment_on_line)
			length += com_length;
	}

	return length;
}

char *skip_horiz_space(const char *p)
{
	while (*p == ' ' || *p == TAB)
		p++;
	return (char *)p;
}

void skip_buffered_space(void)
{
	while (*buf_ptr == ' ' || *buf_ptr == TAB) {
		if (++buf_ptr >= buf_end)
			fill_buffer();
	}
}

int current_column(void)
{
	char *p;
	int column;

	/* Use save_com.size here instead of save_com.end, because save_com is already emptied at this point. */
	if (save_com.ptr <= buf_ptr && buf_ptr <= save_com.ptr + save_com.len) {
		p = save_com.ptr;
		column = save_com.start_column;
	} else {
		p = cur_line;
		column = 1;
	}

	while (p < buf_ptr) {
		switch (*p) {
			case EOL:
			case 014:
				column = 1;
				break;
			case TAB:
				column += settings.tabsize - (column - 1) % settings.tabsize;
				break;
			case '\b':
				column--;
				break;
			default:
				column++;
				break;
		}
		p++;
	}

	return column;
}

file_buffer_ty *read_file(char *filename, struct stat *file_stats)
{
	static file_buffer_ty fileptr;

	unsigned int size;
	int namelen = strlen(filename);

	int fd = open(filename, O_RDONLY, 0777);

	if (fd < 0)
		fatal(_("Can't open input file %s"), filename);

	if (fstat(fd, file_stats) < 0)
		fatal(_("Can't stat input file %s"), filename);

	if (file_stats->st_size == 0)
		ERROR(_("Warning: Zero-length file %s"), filename, 0);

	if (file_stats->st_size < 0)
		fatal(_("System problem reading file %s"), filename);

	fileptr.size = file_stats->st_size;

	fileptr.data = fileptr.data ? xrealloc(fileptr.data, fileptr.size + 2) : xmalloc(fileptr.size + 2);

	size = read(fd, fileptr.data, fileptr.size);

	if (size < 0)
		fatal(_("Error reading input file %s"), filename);

	if (close(fd) < 0)
		fatal(_("Error closeing input file %s"), filename);

	/*
	 * Apparently, the DOS stores files using CR-LF for newlines, but then the DOS `read' changes them into '\n'.
	 * Thus, the size of the file on disc is larger than what is read into memory.
	 */
	if (size < fileptr.size)
		fileptr.size = size;

	fileptr.name = fileptr.name ? xrealloc(fileptr.name, namelen + 1) : xmalloc(namelen + 1);

	strncpy(fileptr.name, filename, namelen);
	fileptr.name[namelen] = EOS;

	if (fileptr.data[fileptr.size - 1] != EOL) {
		fileptr.data[fileptr.size] = EOL;
		fileptr.size++;
	}

	fileptr.data[fileptr.size] = EOS;

	return &fileptr;
}

file_buffer_ty *read_stdin(void)
{
	static file_buffer_ty stdinptr;

	unsigned int size = 15 * BUFSIZ;
	int ch = EOF;
	char *p = NULL;

	if (stdinptr.data)
		free(stdinptr.data);

	stdinptr.data = xmalloc(size + 1);
	stdinptr.size = 0;

	p = stdinptr.data;

	do {
		while (stdinptr.size < size) {
			ch = getc(stdin);

			if (ch == EOF)
				break;

			*p++ = ch;
			stdinptr.size++;
		}

		if (ch != EOF) {
			size += 2 * BUFSIZ;
			stdinptr.data = xrealloc(stdinptr.data, (unsigned)size);
			p = stdinptr.data + stdinptr.size;
		}
	} while (ch != EOF);

	stdinptr.name = "Standard Input";

	stdinptr.data[stdinptr.size] = EOS;

	return &stdinptr;
}

/*
 * Advance `buf_ptr' so that it points to the next line of input.
 *
 * If the next input line contains an poem control comment turning off formatting(a comment, C or C++, beginning with *POEM-OFF*),
 * disable poeming by calling inhibit_poeming() which will cause `dump_line ()' to simply print out all input lines without formatting
 * until it finds a corresponding comment containing *POEM-0N* which re-enables formatting.
 *
 * Note that if this is a C comment we do not look for the closing delimiter.
 * Note also that older versions of this program also skipped lines containing *POEM**
 * which represented errors generated by poem in some previous formatting. This version does not recognize such lines. 
 */
void fill_buffer(void)
{
	char *p = NULL;
	BOOLEAN finished_a_line = false;

	/*
	 * poem() may be saving the text between "if (...)" and the following statement.
	 * To do so, it uses another buffer (`save_com'). Switch back to the previous buffer here.
	 */
	if (bp_save) {
		buf_ptr = bp_save;
		buf_end = be_save;
		bp_save = be_save = NULL;

		/* Only return if there is really something in this buffer */
		if (buf_ptr < buf_end)
			return;
	}

	if (*in_prog_pos == EOS) {
		cur_line = buf_ptr = in_prog_pos;
		had_eof = true;
	} else {
		/* Here if we know there are chars to read. The file is NULL-terminated, so we can always look one character ahead safely. */
		p = cur_line = in_prog_pos;
		finished_a_line = false;

		do {
			p = skip_horiz_space(p);

			if (is_comment_start(p)) {
				p += 2;
				p = skip_horiz_space(p);

				if (!strncmp(p, "*POEM-OFF*", 10))
					inhibit_poeming(true);
			}

			while (*p != EOS && *p != EOL)
				p++;

			if (*p == EOL) {
				finished_a_line = true;
				in_prog_pos = p + 1;
			} else if ((unsigned int)(p - current_input->data) < current_input->size) {
				WARNING(_("Warning: File %s contains embedded NULLs\n"), current_input->name, 0);
				p++;
			} else {
				in_prog_pos = p;
				finished_a_line = true;
			}
		} while (!finished_a_line);

		buf_ptr = cur_line;
		buf_end = in_prog_pos;
	}
}

codes_ty lexi(void)
{
	int unary_delim;
	
	static codes_ty last_code;
	
	static int l_struct;
	
	static int l_enum;
	
	codes_ty code;
	
	char qchar;

	static int count;

	count++;

	unary_delim = false;

	/* Tell the world that this token started in column 1 if the last thing scanned was nl */
	ppst->last_saw_nl = ppst->col_1 = ppst->last_nl;
	ppst->last_nl = false;

	if (buf_ptr >= buf_end)
		fill_buffer();

	if (*buf_ptr == ' ' || *buf_ptr == TAB) {
		ppst->col_1 = false;
		skip_buffered_space();
	}

	if (buf_ptr >= buf_end)
		fill_buffer();

	/*
	 * WARNING
	 * Note that subsequent calls to `fill_buffer ()' may switch `buf_ptr'
	 * to a different buffer. Thus when `token_end' gets set later, it may be pointing into a different buffer than `token'.
	 */
	token = buf_ptr;

	/* Scan an alphanumeric token */
	if (
		(
			!(
				buf_ptr[0] == 'L'
				&& (buf_ptr[1] == '"' || buf_ptr[1] == '\'')
			)
			&& chartype[0xff & (int)*buf_ptr] == alphanum
		)
		|| (buf_ptr[0] == '.' && isdigit(buf_ptr[1]))
	) {
		templ_ty *p;

		/* We have a character or number */
		if (isdigit(*buf_ptr) || (buf_ptr[0] == '.' && isdigit(buf_ptr[1]))) {

			int seendot = 0, seenexp = 0;

			if (*buf_ptr == '0' && (buf_ptr[1] == 'x' || buf_ptr[1] == 'X')) {
				buf_ptr += 2;

				while (isxdigit(*buf_ptr))
					buf_ptr++;
			} else {
				while (1) {
					if (*buf_ptr == '.') {
						if (seendot)
							break;
						else
							seendot++;
					}

					buf_ptr++;

					if (!isdigit(*buf_ptr) && *buf_ptr != '.') {
						if ((*buf_ptr != 'E' && *buf_ptr != 'e') || seenexp)
							break;
						else {
							seenexp++;
							seendot++;
							buf_ptr++;

							if (*buf_ptr == '+' || *buf_ptr == '-')
								buf_ptr++;
						}
					}
				}
			}

			if (*buf_ptr == 'F' || *buf_ptr == 'f' || *buf_ptr == 'i' || *buf_ptr == 'j')
				buf_ptr++;
			else {
				while (*buf_ptr == 'U' || *buf_ptr == 'u' || *buf_ptr == 'L' || *buf_ptr == 'l')
					buf_ptr++;
			}
		} else {
			while (chartype[0xff & (int)*buf_ptr] == alphanum) {
				if (++buf_ptr >= buf_end)
					fill_buffer();
			}
		}

		token_end = buf_ptr;

		if (token_end - token == 13 && !strncmp(token, "__attribute__", 13)) {
			last_code = decl;
			ppst->last_u_d = true;
			return attribute;
		}
		
		skip_buffered_space();

		/* Handle operator declarations. */
		if (token_end - token == 8 && !strncmp(token, "operator", 8)) {
			while (chartype[0xff & (int)*buf_ptr] == opchar) {
				if (++buf_ptr >= buf_end)
					fill_buffer();
			}
			token_end = buf_ptr;
			
			skip_buffered_space();
		}

		ppst->sizeof_keyword = ppst->its_a_keyword = false;

		/* If last token was 'struct', then this token should be treated as a declaration */
		if (l_struct) {
			l_struct = false;
			last_code = ident;
			ppst->last_u_d = true;

			return ppst->last_token == cpp_operator ? overloaded : decl;
		}

		/* Operator after indentifier is binary */
		ppst->last_u_d = false;
		last_code = ident;

		p = settings.c_plus_plus ? is_reserved_cc(token, token_end - token) : is_reserved(token, token_end - token);

		if (p == NULL && user_specials != 0) {
			for (
				p = &user_specials[0];
				p < &user_specials[0] + user_specials_idx;
				p++
			) {
				char *q = token;
				char *r = p->rwd;

				/* This string compare is a little nonstandard because token ends at the character before token_end and p->rwd is null-terminated. */
				while (1) {
					/* If we have come to the end of both the keyword in user_specials and the keyword in token they are equal. */
					if (q >= token_end && !*r)
						goto found_keyword;

					/* If we have come to the end of just one, they are not equal. */
					if (q >= token_end || !*r)
						break;

					/* If the characters in corresponding characters are not equal, the strings are not equal. */
					if (*q++ != *r++)
						break;
				}
			}
			/* Didn't find anything in user_specials. */
			p = NULL;
		}
found_keyword:
		if (p) {
			codes_ty value;

			value = ident;
			
			ppst->its_a_keyword = true;
			ppst->last_u_d = true;
			ppst->last_rw = p->rwcode;
			ppst->last_rw_depth = ppst->paren_depth;

			switch (p->rwcode) {
				case rw_operator:
					value = cpp_operator;
					ppst->in_parameter_declaration = 1;
					break;
				case rw_switch:
					value = swstmt;
					break;
				case rw_case:
					value = casestmt;
					break;
				case rw_enum:
					l_enum = true;
					/* Reset on '(' ')' '{' '}' or ';' since enum can appear after them */
				case rw_struct_like:
					/* inside parens: cast */
					if (
						ppst->p_l_follow
						&& !(ppst->noncast_mask & 1 << ppst->p_l_follow)
					) {
						ppst->cast_mask |= 1 << ppst->p_l_follow;
						break;
					}
					l_struct = true;
				case rw_decl:
					/* One of the declaration keywords */
					/* Inside parens: cast */
					if (
						ppst->p_l_follow
						&& !(ppst->noncast_mask & 1 << ppst->p_l_follow)
					) {
						ppst->cast_mask |= 1 << ppst->p_l_follow;
						break;
					}

					last_code = decl;
					value = decl;
					break;
				case rw_sp_paren:
					value = sp_paren;
					if (*token == 'i' && ppst->last_token == sp_else)
						ppst->i_l_follow -= settings.ind_size;
					break;
				case rw_sp_nparen:
					value = sp_nparen;
					break;
				case rw_sp_else:
					value = sp_else;
					break;
				case rw_sizeof:
					ppst->sizeof_keyword = true;
					value = ident;
					break;
				case rw_return:
				case rw_break:
				default:
					value = ident;
			}

			return ppst->last_token == cpp_operator ? overloaded : value;
		} else if (
			*buf_ptr == '('
			&& ppst->tos <= 1
			&& ppst->ind_level == 0
			&& ppst->paren_depth == 0
		) {
			char *tp;
			
			/* We have found something which might be the name in a function definition.  */
			int paren_count = 1;

			/*
			 * If the return type of this function definition was not defined with a -T commandline option, then the output of poem would
			 * alternate on subsequent calls. In order to avoid that we try to detect that case here and make a minimal change to cause the correct behaviour.
			 */
			if (ppst->last_token == ident && ppst->last_saw_nl)
				ppst->in_decl = true;

			/* Skip to the matching ')'. */
			for (
				tp = buf_ptr + 1;
				paren_count > 0 && tp < in_prog + in_prog_size;
				tp++
			) {
				if (*tp == '(')
					paren_count++;

				if (*tp == ')')
					paren_count--;

				/* Can't occur in parameter list; this way we don't search the whole file in the case of unbalanced parens. */
				if (*tp == ';')
					goto not_proc;
			}

			if (paren_count == 0) {
				ppst->procname = token;
				ppst->procname_end = token_end;

				while (isspace(*tp))
					tp++;

				if (
					*tp == '_'
					&& in_prog + in_prog_size - tp >= 13
					&& !strncmp(tp, "__attribute__", 13)
				) {
					/* found an __attribute__ after a function declaration */
					/* must be a declaration */
				} else {
					/*
					 * If the next char is ';' or ',' or '(' we have a function declaration, not a definition.
					 * I've added '=' to this list to keep from breaking a non-valid C macro from libc.  -jla
					 */
					if (*tp != ';' && *tp != ',' && *tp != '(' && *tp != '=')
						ppst->in_parameter_declaration = 1;
				}
			}
not_proc:;
		} else if (
			*buf_ptr == ':'
			&& *(buf_ptr + 1) == ':'
			&& ppst->tos <= 1
			&& ppst->ind_level == 0
			&& ppst->paren_depth == 0
		) {
			ppst->classname = token;
			ppst->classname_end = token_end;
			
		/* The following hack attempts to guess whether or not the current token is in fact a declaration keyword -- one that has been typedef'd */
		} else if (
			(
				(*buf_ptr == '*' && buf_ptr[1] != '=')
				|| isalpha(*buf_ptr)
				|| *buf_ptr == '_'
			)
			&& !ppst->p_l_follow
			&& !ppst->block_init
			&& (
				ppst->last_token == rparen
				|| ppst->last_token == semicolon
				|| ppst->last_token == rbrace
				|| ppst->last_token == decl
				|| ppst->last_token == lbrace
				|| ppst->last_token == start_token
			)
		) {
			ppst->its_a_keyword = true;
			ppst->last_u_d = true;
			last_code = decl;

			return ppst->last_token == cpp_operator ? overloaded : decl;
		}

		/* If this is a declared variable, then following sign is unary will make "int a -1" work */
		if (last_code == decl)
			ppst->last_u_d = true;

		last_code = ident;

		/* the ident is not in the list */
		return ppst->last_token == cpp_operator ? overloaded : ident;
	}
	
	/* Scan a non-alphanumeric token */
	/* If it is not a one character token, token_end will get changed later.  */
	token_end = buf_ptr + 1;

	/*
	 * Note that it may be possible for this to kill us -- if `fill_buffer' at any time switches `buf_ptr' to the
	 * other input buffer, `token' and `token_end' will point to different storage areas!!!
	 */
	if (++buf_ptr >= buf_end)
		fill_buffer();

	/* If it is a backslash new-line, just eat the backslash */
	if (*token == '\\' && buf_ptr[0] == EOL) {
		token = buf_ptr;

		if (++buf_ptr >= buf_end)
			fill_buffer();
	}

	switch (*token) {
		case '\0':
			code = code_eof;
			break;
		case EOL:
			ppst->matching_brace_on_same_line = -1;
			unary_delim = ppst->last_u_d;
			ppst->last_nl = true;
			code = newline;
			break;
		/* Handle wide strings and chars. */
		case 'L':
			if (buf_ptr[0] != '"' && buf_ptr[0] != '\'') {
				token_end = buf_ptr;
				code = ident;
				break;
			}

			qchar = buf_ptr[0];
			buf_ptr++;
			goto handle_string;
		case '\'':
		case '"':
			qchar = *token;
handle_string:
			/* Find out how big the literal is so we can set token_end. Invariant: before loop test buf_ptr points to the next character that we have not yet checked. */
			/* && *buf_ptr != EOL) */
			while (*buf_ptr != qchar && *buf_ptr != 0) {
				
				if (*buf_ptr == EOL)
					++line_no;
				
				if (*buf_ptr == '\\') {
					if (++buf_ptr >= buf_end)
						fill_buffer();

					if (*buf_ptr == EOL)
						++line_no;
					if (*buf_ptr == 0)
						break;
				}

				if (++buf_ptr >= buf_end)
					fill_buffer();
			}

			if (*buf_ptr == EOL || *buf_ptr == 0)
				WARNING((qchar == '\'' ? _("Unterminated character constant") : _("Unterminated string constant")), 0, 0);
			else {
				/* Advance over end quote char. */
				if (++buf_ptr >= buf_end)
					fill_buffer();
			}

			token_end = buf_ptr;
			code = ident;
			break;
		case '(':
			l_enum = false;
			unary_delim = true;
			code = lparen;
			break;
		case '[':
			if (ppst->in_or_st)
				ppst->in_or_st++;
			unary_delim = true;
			code = lparen;
			break;
		case ')':
			l_enum = false;
			code = rparen;
			break;
		case ']':
			if (ppst->in_or_st > 1)
				ppst->in_or_st--;
			code = rparen;
			break;
		case '#':
			unary_delim = ppst->last_u_d;
			code = preesc;

			/* Make spaces between '#' and the directive be part of the token if user specified "-lps" */
			if (settings.leave_preproc_space) {
				while (*buf_ptr == ' ' && buf_ptr < buf_end)
					buf_ptr++;
				token_end = buf_ptr;
			}
			break;
		case '?':
			unary_delim = true;
			code = question;
			break;
		case ':':
			if (*buf_ptr == ':') {
				code = doublecolon;
				token_end = ++buf_ptr;
				break;
			}

			code = colon;
			unary_delim = true;
			if (squest && *e_com != ' ')
				ppst->want_blank = e_code != s_code;
			break;
		case ';':
			l_enum = false;
			unary_delim = true;
			code = semicolon;
			break;
		case '{':
			if (ppst->matching_brace_on_same_line < 0)
				ppst->matching_brace_on_same_line = 1;
			else
				ppst->matching_brace_on_same_line++;
			
			if (l_enum) {
				/*
				 * Keep all variables in the same column:
				 *   ONE,
				 *   TWO, etc
				 * instead of
				 *   ONE,
				 *     TWO,
				 * Use a special code for `block_init' however, because we still want to do the line breaks when `settings.braces_on_struct_decl_line' is not set.
				 */
				ppst->block_init = 2;
				ppst->block_init_level = 0;
				l_enum = false;
			}
			unary_delim = true;
			code = lbrace;
			break;
		case '}':
			ppst->matching_brace_on_same_line--;
			l_enum = false;
			unary_delim = true;
			code = rbrace;
			break;
		case 014:
			unary_delim = ppst->last_u_d;
			/* Remember this so we can set 'ppst->col_1' right */
			ppst->last_nl = true;
			code = form_feed;
			break;
		case ',':
			unary_delim = true;
			code = comma;
			break;
		case '.':
			if (ppst->in_decl && buf_ptr[0] == '.' && buf_ptr[1] == '.') {
				if ((buf_ptr += 2) >= buf_end)
					fill_buffer();

				unary_delim = true;
				code = decl;
				token_end = buf_ptr;
				break;
			}
			unary_delim = false;
			code = struct_delim;

			/* Object .* pointer-to-member */
			if (*buf_ptr == '*')
				token_end = ++buf_ptr;
			break;
		/* check for -, +, --, ++ */
		case '-':
		case '+':
			code = ppst->last_u_d ? unary_op : binary_op;
			unary_delim = true;

			if (*buf_ptr == token[0]) {
				buf_ptr++;

				/* Buffer overflow will be checked at end of loop */
				if (last_code == ident || last_code == rparen) {
					code = ppst->last_u_d ? unary_op : postop;
					/* Check for following ++ or -- */
					unary_delim = false;
				}
			} else if (*buf_ptr == '=')
				/* Check for operator += */
				buf_ptr++;
			else if (*buf_ptr == '>') {
				/* Check for operator -> */
				buf_ptr++;
				code = struct_delim;
				/* Check for operator ->* */
				if (*buf_ptr == '*')
					buf_ptr++;
			}

			token_end = buf_ptr;
			
			/* Buffer overflow will be checked at end of switch */
			break;
		case '=':
			if (ppst->in_or_st && ppst->last_token != cpp_operator) {
				ppst->block_init = 1;
				ppst->block_init_level = 0;
			}

			if (*buf_ptr == '=')
				buf_ptr++;
			else if (*buf_ptr == '-' || *buf_ptr == '+' || *buf_ptr == '*' || *buf_ptr == '&') {
				/*
				 * Something like x=-1, which can mean x -= 1 ("old style" in K&R1) or x = -1 (ANSI).  Note that this is only an ambiguity if the
				 * character can also be a unary operator.  If not, just produce output code that produces a syntax error (the theory being that
				 * people want to detect and eliminate old style assignments but they don't want poem to silently change the meaning of their code).
				 */
				WARNING(
					_("old style assignment ambiguity in \"=%c\".  Assuming \"= %c\"\n"),
					(unsigned long)*((unsigned char *)buf_ptr),
					(unsigned long)*((unsigned char *)buf_ptr)
				);
			}

			code = binary_op;
			unary_delim = true;
			token_end = buf_ptr;
			break;
		case '>':
		case '<':
		case '!':
			/*
			 * ops like <, <<, <=, !=, <<=, etc
			 * This will of course scan sequences like "<=>", "!=>", "<<>", etc. as one token, but I don't think that will cause any harm.
			 * in C++ mode also scan <?[=], >?[=] GNU C++ operators maybe some flag to them ?
			 */
			while (*buf_ptr == '>' || *buf_ptr == '<' || *buf_ptr == '=' || (settings.c_plus_plus && *buf_ptr == '?')) {
				if (++buf_ptr >= buf_end)
					fill_buffer();

				if (*buf_ptr == '=')
					if (++buf_ptr >= buf_end)
						fill_buffer();
			}

			code = ppst->last_u_d ? unary_op : binary_op;
			unary_delim = true;
			token_end = buf_ptr;
			break;
		default:
			if (token[0] == '/' && (*buf_ptr == '*' || *buf_ptr == '/')) {

				code = *buf_ptr == '*' ? comment : cplus_comment;

				if (++buf_ptr >= buf_end)
					fill_buffer();
				
				if (code == comment) {
					/* Threat comments of type / *UPPERCASE* / not as comments */
					char *p = buf_ptr;

					/* There is always at least one newline in the buffer; so no need to check for buf_end. */
					while (isupper(*p++));

					if (p < buf_end && p[-1] == '*' && *p == '/') {
						buf_ptr = p + 1;
						code = ident;
						ppst->want_blank = true;
					}
				}
				unary_delim = ppst->last_u_d;
			} else if (ppst->last_token == cpp_operator)
				last_code = code = overloaded;
			else {
				while (*(buf_ptr - 1) == *buf_ptr || *buf_ptr == '=')
					/* handle ||, &&, etc, and also things as in int *****i */
					if (++buf_ptr >= buf_end)
						fill_buffer();
				code = ppst->last_u_d ? unary_op : binary_op;
				unary_delim = true;
			}

			token_end = buf_ptr;
	}

	if (code != newline) {
		l_struct = false;
		last_code = code;
	}

	if (buf_ptr >= buf_end)
		fill_buffer();

	ppst->last_u_d = unary_delim;

	return ppst->last_token == cpp_operator ? overloaded : code;
}

/* Add the given keyword to the keyword table, using val as the keyword type */
void addkey(char *key, rwcodes_ty val)
{
	templ_ty *p;

	if (
		(settings.c_plus_plus && is_reserved_cc(key, strlen(key)))
		|| (!settings.c_plus_plus && is_reserved(key, strlen(key)))
	) {

	} else {
		if (user_specials == 0) {
			user_specials = (templ_ty *)xmalloc(5 * sizeof(templ_ty));
			user_specials_max = 5;
			user_specials_idx = 0;
		} else if (user_specials_idx == user_specials_max) {
			user_specials_max += 5;
			user_specials = (templ_ty *)xrealloc((char *)user_specials, user_specials_max * sizeof(templ_ty));
		}

		p = &user_specials[user_specials_idx++];
		p->rwd = key;
		p->rwcode = val;
	}
}

/*
 * GNU/Emacs style backups --
 * This behaviour is controlled by two environment variables,
 * VERSION_CONTROL and SIMPLE_BACKUP_SUFFIX.
 *
 * VERSION_CONTROL determines what kinds of backups are made.  If it's
 * value is "numbered", then the first modification of some file
 * "eraserhead.c" will yield a backup file "eraserhead.c.~1~", the
 * second modification will yield "eraserhead.c.~2~", and so on.  It
 * does not matter if the version numbers are not a sequence;  the next
 * version will be one greater than the highest in that directory.
 *
 * If the value of VERSION_CONTROL is "numbered_existing", then such
 * numbered backups will be made if there are already numbered backup
 * versions of the file.  Otherwise, the backup name will be that of
 * the original file with "~" (tilde) appended.  E.g., "eraserhead.c~".
 *
 * If the value of VERSION_CONTROL is "simple", then the backup name
 * will be that of the original file with "~" appended, regardless of
 * whether or not there exist numbered versions in the directory.
 *
 * For simple backups, the value of SIMPLE_BACKUP_SUFFIX will be used
 * rather than "~" if it is set.
 *
 * If VERSION_CONTROL is unset, "numbered_existing" is assumed.  For
 * Emacs lovers, "nil" is equivalent to "numbered_existing" and "t" is
 * equivalent to "numbered".
 *
 * Finally, if VERSION_CONTROL is "none" or "never", backups are not
 * made.  I suggest you avoid this behaviour.
 *
 * If VERSION_WIDTH is set, then it controls zero padding of a numbered
 * suffix.
 */

static char *simple_backup_name(char *pathname)
{
	char *backup_name;

	/* '\0' and newline */
	backup_name = xmalloc(strlen(pathname) + strlen(simple_backup_suffix) + 2);
	sprintf(backup_name, "%s%s", pathname, simple_backup_suffix);
	return backup_name;
}

static int version_number(char *base, char *direntry, int base_length)
{
	int version = 0;
	char *p = NULL;
	if (!strncmp(base, direntry, base_length) && ISDIGIT(direntry[base_length + 2])) {
		for (p = &direntry[base_length + 2]; ISDIGIT(*p); ++p)
			version = version * 10 + *p - '0';

		if (p[0] != BACKUP_SUFFIX_CHAR || p[1])
			version = 0;
	}
	return version;
}

static int highest_version(char *filename, char *dirname)
{
	DIR *dirp = opendir(dirname);
	struct dirent *dp = NULL;
	int highest_version;

	if (!dirp)
		highest_version = 0;
	else {
		int this_version;
		int file_name_length = strlen(filename);

		highest_version = 0;

		while ((dp = readdir(dirp)) != 0) {
			if (NAMLEN(dp) <= file_name_length + 2)
				continue;

			this_version = version_number(filename, dp->d_name, file_name_length);

			if (this_version > highest_version)
				highest_version = this_version;
		}
		closedir(dirp);
	}
	return highest_version;
}

static int max_version(char *pathname)
{
	char *p;
	char *filename;
	int pathlen = strlen(pathname);
	int version;

	p = pathname + pathlen - 1;

	while (p > pathname && *p != '/')
		p--;

	if (*p == '/') {
		int dirlen = p - pathname;
		char *dirname;

		filename = p + 1;
		dirname = xmalloc(dirlen + 1);
		strncpy(dirname, pathname, dirlen);
		dirname[dirlen] = '\0';
		version = highest_version(filename, dirname);
		free(dirname);
	} else {
		filename = pathname;
		version = highest_version(filename, ".");
	}

	return version;
}

static char *generate_backup_filename(backup_mode_ty version_control, char *pathname)
{
	int last_numbered_version;
	char *backup_name;

	if (version_control == none)
		backup_name = NULL;
	else {
		if (version_control == simple)
			backup_name = simple_backup_name(pathname);
		else {
			last_numbered_version = max_version(pathname);

			if (version_control == numbered_existing && last_numbered_version == 0)
				backup_name = simple_backup_name(pathname);
			else {
				last_numbered_version++;
				backup_name = xmalloc(strlen(pathname) + 16);

				if (backup_name)
					sprintf(backup_name, BACKUP_SUFFIX_FORMAT, pathname, version_width, last_numbered_version);
			}
		}
	}
	return backup_name;
}

backup_mode_ty version_control_value(void)
{
	char *version = getenv("VERSION_CONTROL");
	version_control_values_ty *v;
	backup_mode_ty ret = unknown;

	if (version == NULL || *version == 0)
		ret = numbered_existing;
	else {
		v = &values[0];

		while (v->name) {
			if (!strcmp(version, v->name)) {
				ret = v->value;
				break;
			} else
				v++;
		}
	}

	return ret;
}

static void set_version_width(void)
{
	char *v = getenv("VERSION_WIDTH");

	if (v && ISDIGIT(*v))
		version_width = atoi(v);

	if (version_width > 16)
		version_width = 16;
}

void initialize_backups(void)
{
	char *v = getenv("SIMPLE_BACKUP_SUFFIX");

	if (v && *v)
		simple_backup_suffix = v;
	
	version_control = version_control_value();

	if (version_control == unknown) {
		fprintf(stderr, _("poem:  Strange version-control value\n"));
		fprintf(stderr, _("poem:  Using numbered-existing\n"));
		version_control = numbered_existing;
	}

	set_version_width();
}

void make_backup(file_buffer_ty *file, const struct stat *file_stats)
{
	FILE *bf;
	char *backup_filename;
	unsigned int size;

	if (version_control == none)
		;
	else {
		backup_filename = generate_backup_filename(version_control, file->name);

		if (!backup_filename) {
			fprintf(stderr, _("poem: Can't make backup filename of %s\n"), file->name);
			exit(system_error);
		}

		bf = fopen(backup_filename, "w");

		if (bf == NULL)
			fatal(_("Can't open backup file %s"), backup_filename);

		size = fwrite(file->data, file->size, 1, bf);

		if (size != 1)
			fatal(_("Can't write to backup file %s"), backup_filename);

		fclose(bf);
#ifdef PRESERVE_MTIME
		{
			struct utimbuf buf;

			buf.actime = time(NULL);
			buf.modtime = file_stats->st_mtime;

			if (utime(backup_filename, &buf))
				WARNING(_("Can't preserve modification time on backup file %s"), backup_filename, 0);
		}
#endif
		free(backup_filename);
	}
}

void init_parser(void)
{	
	ppst = (parser_state_ty *)xmalloc(sizeof(parser_state_ty));
	
	ppst->p_stack_size = INITIAL_STACK_SIZE;
	ppst->p_stack = (codes_ty *)xmalloc(INITIAL_STACK_SIZE * sizeof(codes_ty));
	ppst->il = (int *)xmalloc(INITIAL_STACK_SIZE * sizeof(int));
	ppst->cstk = (int *)xmalloc(INITIAL_STACK_SIZE * sizeof(int));
	ppst->paren_indents_size = 8;
	ppst->paren_indents = (short *)xmalloc(ppst->paren_indents_size * sizeof(short));

	combuf = (char *)xmalloc(INITIAL_BUFFER_SIZE);
	labbuf = (char *)xmalloc(INITIAL_BUFFER_SIZE);
	codebuf = (char *)xmalloc(INITIAL_BUFFER_SIZE);

	save_com.size = INITIAL_BUFFER_SIZE;
	save_com.end = save_com.ptr = xmalloc(save_com.size);
	save_com.len = save_com.column = 0;

	di_stack_alloc = 2;
	di_stack = (int *)xmalloc(di_stack_alloc * sizeof(*di_stack));
}

void reset_parser(void)
{
	ppst->next = NULL;
	ppst->last_token = start_token;
	ppst->p_stack[0] = stmt;
	ppst->il[0] = 0;
	ppst->cstk[0] = 0;
	ppst->tos = 0;
	ppst->box_com = false;
	ppst->cast_mask = 0;
	ppst->noncast_mask = 0;
	ppst->sizeof_mask = 0;
	ppst->block_init = 0;
	ppst->block_init_level = 0;
	ppst->last_nl = true;
	ppst->last_saw_nl = false;
	ppst->saw_double_colon = false;
	ppst->broken_at_non_nl = false;
	ppst->in_or_st = 0;
	ppst->col_1 = false;
	ppst->com_col = 0;
	ppst->dec_nest = 0;
	ppst->decl_on_line = false;
	ppst->i_l_follow = 0;
	ppst->in_decl = false;
	ppst->in_stmt = false;
	ppst->in_parameter_declaration = false;
	ppst->ind_level = 0;
	ppst->ind_stmt = false;
	ppst->last_u_d = false;
	ppst->p_l_follow = 0;
	ppst->paren_level = 0;
	ppst->paren_depth = 0;
	ppst->pcase = false;
	ppst->search_brace = false;
	ppst->use_ff = false;
	ppst->want_blank = false;
	ppst->can_break = bb_none;
	ppst->its_a_keyword = false;
	ppst->sizeof_keyword = false;
	ppst->procname = "\0";
	ppst->procname_end = "\0";
	ppst->classname = "\0";
	ppst->classname_end = "\0";
	ppst->just_saw_decl = 0;

	save_com.len = save_com.column = 0;

	di_stack[ppst->dec_nest] = 0;

	l_com = combuf + INITIAL_BUFFER_SIZE - 5;
	l_lab = labbuf + INITIAL_BUFFER_SIZE - 5;
	l_code = codebuf + INITIAL_BUFFER_SIZE - 5;

	combuf[0] = codebuf[0] = labbuf[0] = ' ';
	combuf[1] = codebuf[1] = labbuf[1] = '\0';

	else_or_endif = false;
	s_lab = e_lab = labbuf + 1;
	s_code = e_code = codebuf + 1;
	s_com = e_com = combuf + 1;

	line_no = 1;
	had_eof = false;
	break_comma = false;
	bp_save = 0;
	be_save = 0;

	if (settings.tabsize <= 0)
		settings.tabsize = 1;

	prefix_blankline_requested = 0;
}

int inc_pstack(void)
{
	if (++ppst->tos >= ppst->p_stack_size) {
		ppst->p_stack_size *= 2;
		ppst->p_stack = (codes_ty *)xrealloc((char *)ppst->p_stack, ppst->p_stack_size * sizeof(codes_ty));
		ppst->il = (int *)xrealloc((char *)ppst->il, ppst->p_stack_size * sizeof(int));
		ppst->cstk = (int *)xrealloc((char *)ppst->cstk, ppst->p_stack_size * sizeof(int));
	}
	ppst->cstk[ppst->tos] = ppst->cstk[ppst->tos - 1];
	return ppst->tos;
}

#ifdef DEBUG
static char **debug_symbol_strings;

void debug_init(void)
{
	debug_symbol_strings = (char **)xmalloc(number_of_codes * sizeof(char *));

	debug_symbol_strings[code_eof] = "code_eof";
	debug_symbol_strings[newline] = "newline";
	debug_symbol_strings[lparen] = "lparen";
	debug_symbol_strings[rparen] = "rparen";
	debug_symbol_strings[start_token] = "start_token";
	debug_symbol_strings[unary_op] = "unary_op";
	debug_symbol_strings[binary_op] = "binary_op";
	debug_symbol_strings[postop] = "postop";
	debug_symbol_strings[question] = "question";
	debug_symbol_strings[casestmt] = "casestmt";
	debug_symbol_strings[colon] = "colon";
	debug_symbol_strings[doublecolon] = "doublecolon";
	debug_symbol_strings[semicolon] = "semicolon";
	debug_symbol_strings[lbrace] = "lbrace";
	debug_symbol_strings[rbrace] = "rbrace";
	debug_symbol_strings[ident] = "ident";
	debug_symbol_strings[overloaded] = "overloaded";
	debug_symbol_strings[cpp_operator] = "cpp_operator";
	debug_symbol_strings[comma] = "comma";
	debug_symbol_strings[comment] = "comment";
	debug_symbol_strings[cplus_comment] = "cplus_comment";
	debug_symbol_strings[swstmt] = "swstmt";
	debug_symbol_strings[preesc] = "preesc";
	debug_symbol_strings[form_feed]	= "form_feed";
	debug_symbol_strings[decl] = "decl";
	debug_symbol_strings[sp_paren] = "sp_paren";
	debug_symbol_strings[sp_nparen] = "sp_nparen";
	debug_symbol_strings[sp_else] = "sp_else";
	debug_symbol_strings[ifstmt] = "ifstmt";
	debug_symbol_strings[elseifstmt] = "elseifstmt";
	debug_symbol_strings[whilestmt] = "whilestmt";
	debug_symbol_strings[forstmt] = "forstmt";
	debug_symbol_strings[stmt] = "stmt";
	debug_symbol_strings[stmtl] = "stmtl";
	debug_symbol_strings[elselit] = "elselit";
	debug_symbol_strings[dolit] = "dolit";
	debug_symbol_strings[dohead] = "dohead";
	debug_symbol_strings[dostmt] = "dostmt";
	debug_symbol_strings[ifhead] = "ifhead";
	debug_symbol_strings[elsehead] = "elsehead";
	debug_symbol_strings[struct_delim] = "struct_delim";
	debug_symbol_strings[attribute] = "attribute";
}
#endif

exit_values_ty parse(codes_ty tk)
{
	/* The code for the construct scanned */
	int i;
#ifdef DEBUG
	if (debug) {
		if (code_eof <= tk && tk < number_of_codes)
			printf("Parse: %s\n", debug_symbol_strings[tk]);
		else
			printf("Parse: Unknown code: %d for %s\n", (int)tk, token ? token : "NULL");
	}
#endif
	while (ppst->p_stack[ppst->tos] == ifhead && tk != elselit) {
		/* true if we have an if without an else */
		/* apply the if(..) stmt ::= stmt reduction */
		ppst->p_stack[ppst->tos] = stmt;
		reduce();
	}

	switch (tk) {
		/* Go on and figure out what to do with the input */
		case decl:
			/* Scanned a declaration word */
			ppst->search_brace = settings.braces_on_struct_decl_line;

			/* Indicate that following brace should be on same line */
			if (ppst->p_stack[ppst->tos] != decl && ppst->block_init == 0) {
				/* Only put one declaration onto stack, while in declaration, newline should be forced after comma */
				break_comma = true;

				inc_pstack();
				ppst->p_stack[ppst->tos] = decl;
				ppst->il[ppst->tos] = ppst->i_l_follow;

				/* Only do if we want left justified declarations */
				if (settings.ljust_decl) {
					ppst->ind_level = 0;
					for (i = ppst->tos - 1; i > 0; --i) {
						if (ppst->p_stack[i] == decl)
							/* indentation is number of declaration levels deep we are* times spaces per level */
							ppst->ind_level += settings.ind_size;
					}
					ppst->i_l_follow = ppst->ind_level;
				}
			}
			break;
		case ifstmt:
			if (ppst->p_stack[ppst->tos] == elsehead)
				ppst->i_l_follow = ppst->il[ppst->tos];
		case dolit:
		case forstmt:
		case casestmt:
			inc_pstack();
			ppst->p_stack[ppst->tos] = tk;
			ppst->ind_level = ppst->i_l_follow;
			ppst->il[ppst->tos] = ppst->ind_level;

			if (tk != casestmt)
				/* Subsequent statements should be indented */
				ppst->i_l_follow += settings.ind_size;
			
			ppst->search_brace = settings.btype_2;
			break;
		case lbrace:
			/* scanned { */
			/* don't break comma in an initial list */
			break_comma = false;
			if (
				ppst->p_stack[ppst->tos] == stmt
				|| ppst->p_stack[ppst->tos] == stmtl
			)
				/* It is a random, isolated stmt group or a declaration */
				ppst->i_l_follow += settings.ind_size;
			else if (ppst->p_stack[ppst->tos] == decl) {
				ppst->i_l_follow += settings.ind_size;

				if (
					(
						ppst->last_rw == rw_struct_like
						|| ppst->last_rw == rw_enum
					)
					&& (
						ppst->block_init != 1
						|| ppst->block_init_level == 0
					)
					&& ppst->last_token != rparen
					&& !settings.braces_on_struct_decl_line
				) {
					ppst->ind_level += settings.struct_brace_indent;
					ppst->i_l_follow += settings.struct_brace_indent;
				}
			} else if (ppst->p_stack[ppst->tos] == casestmt) {
				ppst->ind_level += settings.case_brace_indent - settings.ind_size;
				ppst->i_l_follow += settings.case_brace_indent;
			} else {
				
				/* It is a group as part of a while, for, etc. Only do this if there is nothing on the line */
				if (s_code == e_code)
					ppst->ind_level -= settings.ind_size;
				
				/* For -bl formatting, poem by settings.brace_indent additional spaces e.g. if (foo == bar) { <--> settings.brace_indent spaces (in this example, 4) */
				if (!settings.btype_2) {
					ppst->ind_level += settings.brace_indent;
					ppst->i_l_follow += settings.brace_indent;
				}

				if (ppst->p_stack[ppst->tos] == swstmt)
					ppst->i_l_follow += settings.case_indent;
			}

			inc_pstack();
			ppst->p_stack[ppst->tos] = lbrace;
			ppst->il[ppst->tos] = ppst->ind_level;

			inc_pstack();
			ppst->p_stack[ppst->tos] = stmt;
			/* allow null stmt between braces */
			ppst->il[ppst->tos] = ppst->i_l_follow;

			break;
		case whilestmt:
			if (ppst->p_stack[ppst->tos] == dohead) {
				ppst->i_l_follow = ppst->ind_level = ppst->il[ppst->tos];

				inc_pstack();
				ppst->p_stack[ppst->tos] = whilestmt;

				ppst->ind_level = ppst->il[ppst->tos] = ppst->i_l_follow;

			} else {
				inc_pstack();
				ppst->p_stack[ppst->tos] = whilestmt;
				ppst->il[ppst->tos] = ppst->i_l_follow;

				ppst->i_l_follow += settings.ind_size;
				ppst->search_brace = settings.btype_2;
			}
			break;
		case elselit:
			if (ppst->p_stack[ppst->tos] != ifhead)
				ERROR(_("Unmatched 'else'"), 0, 0);
			else {
				/* Indentation for else should be same as for if */
				ppst->ind_level = ppst->il[ppst->tos];

				/* Everything following should be in 1 level */
				ppst->i_l_follow = (ppst->ind_level + settings.ind_size);

				ppst->p_stack[ppst->tos] = elsehead;
				/* remember if with else */
				ppst->search_brace = true;
			}
			break;
		case rbrace:
			/* Scanned a }, stack should have <lbrace> <stmt> or <lbrace> <stmtl> */
			if (ppst->p_stack[ppst->tos - 1] == lbrace) {
				ppst->i_l_follow = ppst->il[--ppst->tos];
				ppst->ind_level = ppst->i_l_follow;
				ppst->p_stack[ppst->tos] = stmt;
			} else
				ERROR(_("Stmt nesting error."), 0, 0);
			
			break;
		case swstmt:
			inc_pstack();
			ppst->p_stack[ppst->tos] = swstmt;
			ppst->cstk[ppst->tos] = settings.case_indent + ppst->i_l_follow;

			if (!settings.btype_2)
				ppst->cstk[ppst->tos] += settings.brace_indent;
			
			/* save current case poem level */
			ppst->il[ppst->tos] = ppst->i_l_follow;

			/* Case labels should be one level down from switch, plus `settings.case_indent' if any. Then, statements should be the `settings.ind_size' further */
			ppst->i_l_follow += settings.ind_size;
			ppst->search_brace = settings.btype_2;
			break;
		case semicolon:
			/* this indicates a simple stmt */
			/* turn off flag to break after commas in a declaration */
			break_comma = false;

			if (ppst->p_stack[ppst->tos] == dostmt)
				ppst->p_stack[ppst->tos] = stmt;
			else {
				inc_pstack();
				ppst->p_stack[ppst->tos] = stmt;
				ppst->il[ppst->tos] = ppst->ind_level;
			}

			break;
		default:
			fatal(_("Unknown code to parser"), 0);
	}

	reduce();

#ifdef DEBUG
	if (debug) {
		printf("\n");

		printf(_("ParseStack [%d]:\n"), (int)ppst->p_stack_size);

		for (i = 1; i <= ppst->tos; ++i)
			printf(_("  stack[%d] =>   stack: %d   ind_level: %d\n"), (int)i, (int)ppst->p_stack[i], (int)ppst->il[i]);

		printf("\n");
	}
#endif
	return total_success;
}

/*
 * FUNCTION: Implements the reduce part of the parsing algorithm
 * ALGORITHM: The following reductions are done. Reductions are repeated until no more are possible.
 *
 *  Old TOS              New TOS [stmt] [stmt]           [stmtl] [stmtl] [stmt]
 *     [stmtl] do [stmt]                 dohead [dohead] [whilestmt]
 *     [dostmt] if [stmt]                "ifstmt" switch [stmt]          [stmt]
 *     decl [stmt]               [stmt] "ifelse" [stmt]          [stmt] for
 *     [stmt]                    [stmt] while [stmt]                     [stmt]
 *     "dostmt" while            [stmt]
 *
 *  On each reduction, ppst->i_l_follow (the indentation for the following line) is set to the indentation level associated with the old TOS.
 *
 *  GLOBALS: ppst->cstk ppst->i_l_follow =
 *     ppst->il ppst->p_stack = ppst->tos =
 */
void reduce(void)
{
	int i;
	for (;;) {
		/* keep looping until there is nothing left to reduce */
		switch (ppst->p_stack[ppst->tos]) {
			case stmt:
				switch (ppst->p_stack[ppst->tos - 1]) {
					case stmt:
					case stmtl:
						/* stmtl stmt or stmt stmt */
						ppst->p_stack[--ppst->tos] = stmtl;
						break;
					case dolit:
						/* [do] [stmt] */
						ppst->p_stack[--ppst->tos] = dohead;
						ppst->i_l_follow = ppst->il[ppst->tos];
						break;
					case ifstmt:
						/* [if] [stmt] */
						ppst->p_stack[--ppst->tos] = ifhead;
						for (
							i = ppst->tos - 1;
							(
								ppst->p_stack[i] != stmt
								&& ppst->p_stack[i] != stmtl
								&& ppst->p_stack[i] != lbrace
							);
							--i
						);

						ppst->i_l_follow = ppst->il[i];

						/* For the time being, we will assume that there is no else on this if, and set the indentation level accordingly. If an else is scanned, it will be fixed up later */
						break;
					case swstmt:
						/* [switch] [stmt] */
					case decl:
						/* finish of a declaration */
					case elsehead:
						/* [[if] [stmt] else] [stmt] */
					case forstmt:
						/* [for] [stmt] */
					case casestmt:
						/* [case n:] [stmt] */
					case whilestmt:
						/* [while] [stmt] */
						ppst->p_stack[--ppst->tos] = stmt;
						ppst->i_l_follow = ppst->il[ppst->tos];
						break;
					default:
						/* [anything else] [stmt] */
						return;
				}
				break;
			case whilestmt:
				/* while (...) on top */
				if (ppst->p_stack[ppst->tos - 1] == dohead) {
					/* it is termination of a do while */
					ppst->p_stack[--ppst->tos] = dostmt;
					break;
				} else
					return;
			default:
				return;
		}
	}
}

/*
 * This kludge is called from main. It is just like parse(semicolon) except that it does not clear break_comma.
 * Leaving break_comma alone is necessary to make sure that "int foo(), bar()" gets formatted correctly under -bc.
 */
void parse_lparen_in_decl(void)
{
	inc_pstack();
	ppst->p_stack[ppst->tos] = stmt;
	
	ppst->il[ppst->tos] = ppst->ind_level;

	reduce();
}

/*
 * Output a comment.
 * `buf_ptr' is pointing to the character after the beginning comment delimiter when this is called. This handles both C and C++ comments.
 *
 * As far as poem is concerned, there are basically two types of comments -- those on lines by themselves and those which are on lines with other code.
 * Variables (and the options specifying them) affecting the printing of comments are:
 *
 * `format_comments'                ("fca"):  Ignore newlines in the
 *     comment and perform filling up to `comment_max_col'. Double
 *     newlines indicate paragraph breaks.
 *
 * `format_col1_comments'           ("fc1"):  Format comments which
 *     begin in column 1.
 *
 * `unindent_displace'              ("d"):  The hanging indentation for
 *     comments which do not appear to the right of code.
 *
 * `comment_delimiter_on_blankline' ("cdb"):  If set, place the comment
 *     delimiters on lines by themselves.  This only affects comments
 *     which are not to the right of code.
 *
 * `com_ind'                        ("c"):  The column in which to begin
 *     comments that are to the right of code.
 *
 * `decl_com_ind'                   ("cd"):  The column in which to begin
 *     comments that are to the right of declarations.
 *
 * `else_endif_col'                 ("cp"):  The column in which to begin
 *     comments to the right of preprocessor directives.
 *
 * `star_comment_cont'              ("sc"):  Place a star ('*') to the
 *     left of the comment body.
 *
 * `comment_max_col'                ("lc"): The length of a comment line.
 *     Formatted comments which extend past this column will be continued on
 *     the following line.  If this option is not specified, `max_col' is
 *     used.
 */
void print_comment(int *paren_targ, BOOLEAN *pbreak_line)
{
	int column;
	int format;
	codes_ty comment_type;

	int start_column;
	int found_column;
	int first_comment_line;
	
	/* right margin column of comments */
	int right_margin;
	
	int boxed_comment;
	
	/* Place a star ('*') to the left of the comment body. */
	int stars;
	
	/* put comments delimiters on blank lines */
	int blankline_delims;
	
	int paragraph_break;
	
	/* set to true if user wants blank lines merged */
	int merge_blank_comment_lines;

	/* judgement helper for two contiguous comments */
	int two_contiguous_comments = 0;
	
	int save_length = 0;
	char *save_ptr = NULL;
	char *text_on_line = NULL;
	char *line_break_ptr = NULL;
	
	/* start delimiter, c or cplus style */
	char *start_delim = NULL;

	/* a preamble at the beginning of line */
	char *line_preamble = NULL;

	/* the length of the preamble at the beginning of line */
	int line_preamble_length;
	
	/* set to true of it is a visible preamble */
	int visible_preamble;
	
	/* shall we suppress the -cdb option? */
	int suppress_cdb = 0;

	/* Increment the parser stack, as we will store some things there for dump_line to use. */
	inc_pstack();

	/* Have to do it this way because this piece of shit program doesn't always place the last token code on the stack. */
	comment_type = *(token + 1) == '/' ? cplus_comment : comment;

	/*
	 * First, decide what kind of comment this is: C++, C, or boxed C. Even if this appears to be a normal C comment,
	 * we may change our minds if we find a star in the right column of the second line, in which case that's a boxed comment too.
	 */
	if (comment_type == cplus_comment) {
		start_delim = "//";
		line_preamble = "// ";
		line_preamble_length = strlen(line_preamble);
		visible_preamble = 1;
		boxed_comment = 0;
		stars = 0;
		blankline_delims = 0;
	} else if (
		*buf_ptr == '*'
		|| *buf_ptr == '-'
		|| *buf_ptr == '='
		|| *buf_ptr == '_'
		|| (ppst->col_1 && !settings.format_col1_comments)
	) {
		/* Boxed comment.  This block of code will return. */
		
		/* how many lines of current comments */
		int comment_lines_count = 1;
		stars = 0;
		boxed_comment = 0;
		blankline_delims = 0;
		line_preamble_length = 0;
		visible_preamble = 0;

		start_column = current_column() - 2;
		found_column = start_column;
		
		ppst->box_com = 1;
		ppst->com_col = found_column;

		if (settings.blanklines_before_blockcomments)
			prefix_blankline_requested = 1;

		*e_com++ = '/';
		*e_com++ = '*';

		while (1) {
			do {
				/* Count line numbers within comment blocks */
				if (*buf_ptr == EOL)
					++line_no;

				*e_com++ = *buf_ptr++;
				CHECK_SIZE(com)
			} while ((*buf_ptr != '*') && (buf_ptr < buf_end));

			/* We have reached the end of the comment, and it's all on this line. */
			if (*buf_ptr == '*' && *(buf_ptr + 1) == '/') {
				
				if (buf_ptr == buf_end)
					fill_buffer();
				
				buf_ptr += 2;

				if (buf_ptr == buf_end)
					fill_buffer();
				
				*e_com++ = '*';
				*e_com++ = '/';
				*e_com = '\0';
				ppst->tos--;

				/*
				 * If this is the only line of a boxed comment, it may be after some other text (e.g., #if foo <comment>),
				 * in which case we want to specify the correct column.
				 * In the other cases, the leading spaces account for the columns and we start it in column 1.
				 */
				ppst->com_col = comment_lines_count > 1 ? 1 : found_column;
				return;
			}

			/* End of the line, or end of file. */
			if (buf_ptr == buf_end) {
				if (*(buf_ptr - 1) == EOL) {
					*(--e_com) = EOS;
					dump_line(true, paren_targ, pbreak_line);
					comment_lines_count++;
					ppst->com_col = 1;
				}

				fill_buffer();

				if (had_eof) {
					*e_com++ = '\0';
					ppst->tos--;
					ppst->com_col = start_column;
					return;
				}
			}
		}
	} else {
		start_delim = "/*";
		line_preamble = 0;
		line_preamble_length = 0;
		visible_preamble = 0;
		boxed_comment = 0;
		stars = settings.star_comment_cont;
		blankline_delims = settings.comment_delimiter_on_blankline;
	}

	paragraph_break = 0;
	merge_blank_comment_lines = 0;
	first_comment_line = com_lines;
	right_margin = settings.comment_max_col;

	/* Now, compute the correct indentation for this comment and whether or not it should be formatted. */
	found_column = current_column() - 2;

	if (s_lab == e_lab && s_code == e_code) {
		/* first handle comments which begin the line. */
		if (ppst->col_1 && !settings.format_col1_comments) {
			format = settings.format_col1_comments;
			start_column = 1;
		} else {
			format = settings.format_comments;

			if (
				ppst->ind_level <= 0
				&& (
					!ppst->in_stmt
					|| (
						ppst->in_decl
						&& ppst->paren_level == 0
					)
				)
			)
				start_column = found_column;
			else {
				/* This comment is within a procedure or other code. */
				start_column = compute_code_target(*paren_targ) - settings.unindent_displace;
				
				if (start_column < 0)
					start_column = 1;
			}
		}
	} else {
		/* This comment follows code of some sort. */
		int target;

		suppress_cdb = 1;

		/* First, compute where the comment SHOULD go. */
		if (ppst->decl_on_line)
			target = settings.decl_com_ind;
		else if (else_or_endif)
			target = settings.else_endif_col;
		else
			target = settings.com_ind;

		/* Now determine if the code on the line is short enough to allow the comment to begin where it should. */
		if (s_code != e_code)
			start_column = count_columns(compute_code_target(*paren_targ), s_code, NULL_CHAR);
		else {
			/* s_lab != e_lab : there is a label here. */
			start_column = count_columns(compute_label_target(), s_lab, NULL_CHAR);
		}

		if (start_column < target)
			start_column = target;
		else {
			/* If the too-long code is a pre-processor command, start the comment 1 space afterwards, otherwise start at the next tab mark. */
			if (else_or_endif) {
				start_column++;
				else_or_endif = false;
			} else
				start_column += settings.tabsize - (start_column - 1) % settings.tabsize;
		}

		format = settings.format_comments;
	}

	/* set the preamble and its' corresponding length, is it a visible preamble as well */
	if (!line_preamble) {
		line_preamble_length = 3;

		if (stars) {
			line_preamble = " * ";
			visible_preamble = 1;
		} else {
			line_preamble = "   ";
			visible_preamble = 0;
		}
	}

	/* These are the parser stack variables used to communicate formatting information to dump_line (). */
	ppst->com_col = two_contiguous_comments ? 1 : start_column;
	ppst->box_com = boxed_comment;

	*e_com++ = *start_delim;
	*e_com++ = *(start_delim + 1);
	column = start_column + 2;

	/* If the user specified -cdb, put the delimiter on one line. */
	if (blankline_delims && !suppress_cdb) {
		char *p = buf_ptr;

		*e_com = '\0';
		dump_line(true, paren_targ, pbreak_line);

		/* Check if the delimiter was already on a line by itself, and skip whitespace if formating. */
		p = skip_horiz_space(p);

		if (*p == EOL)
			buf_ptr = p + 1;
		else if (format)
			buf_ptr = p;

		if (buf_ptr >= buf_end)
			fill_buffer();

		column = start_column;
		goto begin_line;
	} else if (format) {
		*e_com++ = ' ';
		column = start_column + 3;

		skip_buffered_space();
	}

	/* Iterate through the lines of the comment */
	while (!had_eof) {
		
		/* Iterate through the characters on one line */
		while (!had_eof) {
			CHECK_SIZE(com)

			switch (*buf_ptr) {
				case ' ':
				case TAB:
					/* If formatting, and previous break marker is nonexistant, or before text on line, reset it to here. */
					if (format && line_break_ptr < text_on_line)
						line_break_ptr = e_com;

					if (format) {
						/* Don't write two spaces after another, unless the first space is preceeded by a dot. */
						if (
							e_com == s_com
							|| e_com[-1] != ' '
							|| e_com - 1 == s_com
							|| e_com[-2] == '.'
						) {
							*e_com++ = ' ';
							column++;
						}
					} else if (*buf_ptr == ' ') {
						*e_com++ = ' ';
						column++;
					} else {
						/* Convert the tab to the appropriate number of spaces, based on the column we found the comment in, not the one we're printing in. */
						int tab_width = settings.tabsize - ((column + found_column - start_column - 1) % settings.tabsize);
						column += tab_width;
						while (tab_width--)
							*e_com++ = ' ';
					}
					break;

				case EOL:
					/* We may be at the end of a C++ comment */
					if (comment_type == cplus_comment) {
	cplus_exit:
						ppst->tos--;
						ppst->com_col = two_contiguous_comments ? 1 : start_column;
						ppst->box_com = boxed_comment;
						*e_com = 0;
						return;
					}

					if (format) {
						/* Newline and null are the two characters which end an input line, so check here if we need to get the next line. */
						if (*buf_ptr == EOL)
							++line_no;

						if (++buf_ptr >= buf_end)
							fill_buffer();

						/* If there are any spaces between the text and this newline character, remove them. */
						if (e_com > line_break_ptr && text_on_line < line_break_ptr)
							e_com = line_break_ptr;

						/* If this is "\n\n", or "\n<whitespace>\n", it's a paragraph break. */
						skip_buffered_space();

						if (*buf_ptr == EOL || !text_on_line) {
							paragraph_break = 1;
							goto end_line;
						}

						/* Also need to eat the preamble. */
						if (
							!boxed_comment
							&& current_column() == found_column + 1
							&& buf_ptr[0] == '*'
							&& buf_ptr[1] != '/'
						) {
							if (++buf_ptr >= buf_end)
								fill_buffer();

							if (*buf_ptr == ' ' && ++buf_ptr >= buf_end)
								fill_buffer();
						}

						/* This is a single newline. Transform it (and any following whitespace) into a single blank. */
						if (e_com[-1] != ' ') {
							line_break_ptr = e_com;
							*e_com++ = ' ';
							column++;
						}
						continue;
					}
					/* We are printing this line "as is", so output it and continue on to the next line. */
					goto end_line;
				case '*':
					/* Check if we've reached the end of the comment. */
					if (comment_type == comment) {
						if (*(buf_ptr + 1) == '/') {
							/* If it's not a boxed comment, put some whitespace before the ending delimiter. Otherwise, simply insert the delimiter. */
							if (!boxed_comment) {
								if (text_on_line) {
									if (blankline_delims && !suppress_cdb) {
										*e_com = '\0';
										dump_line(true, paren_targ, pbreak_line);
										*e_com++ = ' ';
									} else
										/* Insert space before closing delim */
										if (*(e_com - 1) != ' ' && *(e_com - 1) != TAB)
											*e_com++ = ' ';
								} else if (s_com == e_com || *s_com != '/') {
									/*
									 * If no text on line, then line is completely empty or starts with preamble,
									 * or is beginning of comment and starts with beginning delimiter.
									 */
									e_com = s_com;
									
									*e_com++ = ' ';
								} else {
									/* This is case of first comment line. Test with: if (first_comment_line != com_lines) abort (); */
									if (*(e_com - 1) != ' ' && *(e_com - 1) != TAB)
										*e_com++ = ' ';
								}
							}

							*e_com++ = '*';
							*e_com++ = '/';
							*e_com = '\0';

							/*
							 * Skip any whitespace following the comment. If there is only whitespace after it, print the line.
							 * NOTE:  We're not printing the line: TRY IT!
							 */
							buf_ptr += 2;

							buf_ptr = skip_horiz_space(buf_ptr);

							if (buf_ptr >= buf_end)
								fill_buffer();

							ppst->tos--;
							ppst->com_col = two_contiguous_comments ? 1 : start_column;
							ppst->box_com = boxed_comment;
							return;
						}

						/* If this star is on the second line of the comment in the same column as the star of the beginning delimiter, then consider it a boxed comment. */
						if (first_comment_line == com_lines - 1 && e_com == s_com + line_preamble_length) {
							
							/* Account for change in line_preamble_length: */
							column -= line_preamble_length - 1;
							line_preamble = " ";
							line_preamble_length = 1;
							boxed_comment = 1;
							format = 0;
							blankline_delims = 0;
							
							*s_com = ' ';
							*(s_com + 1) = '*';
							text_on_line = e_com = s_com + 2;
							column++;
							break;
						}
					}

				/* If it was not the end of the comment, drop through and insert the star on the line. */
				default:
					/* Some textual character. */
					text_on_line = e_com;
					*e_com++ = *buf_ptr;
					column++;
					break;
			}

			/* If we are formatting, check that we haven't exceeded the line length. If we haven't set line_break_ptr, keep going. */
			if (format && column > right_margin && line_break_ptr) {
				
				if (line_break_ptr < e_com - 1) {
					/* Here if we are really "breaking" the line: the line break is before some text we've seen. */
					*line_break_ptr = '\0';
					save_ptr = line_break_ptr + 1;
					save_length = e_com - save_ptr;
					e_com = line_break_ptr;

					/* If we had to go past `right_margin' to print stuff out, extend `right_margin' out to this point. */
					if (column - save_length > right_margin)
						right_margin = column - save_length;
					
				} else {
					/* The line break is after the last text;  we're really truncating the line. */
					if (comment_type == cplus_comment) {
						buf_ptr = skip_horiz_space(buf_ptr);

						buf_ptr--;
						if (*buf_ptr == EOL)
							goto cplus_exit;
					} else {
						while (*buf_ptr == TAB || *buf_ptr == ' ' || *buf_ptr == EOL) {
							if (*buf_ptr == EOL)
								++line_no;

							if (++buf_ptr >= buf_end)
								fill_buffer();
						}
						buf_ptr--;
					}
					*e_com = EOS;
				}
				goto end_line;
			}

			if (*buf_ptr == EOL)
				++line_no;

			if (++buf_ptr == buf_end)
				fill_buffer();
		}
end_line:
		/* Compress pure whitespace lines into newlines. */
		if (!text_on_line && !visible_preamble && !(first_comment_line == com_lines))
			e_com = s_com;

		*e_com = EOS;
		dump_line(true, paren_targ, pbreak_line);

		/* We're in the middle of a C-comment, don't add blank lines! */
		prefix_blankline_requested = 0;

		/* If formatting (paragraph_break is only used for formatted comments) and user wants blank lines merged, kill all white space after the "\n\n" indicating a paragraph break. */
		if (paragraph_break) {
			if (merge_blank_comment_lines) {
				while (*buf_ptr == EOL || *buf_ptr == ' ' || *buf_ptr == TAB) {
					if (*buf_ptr == EOL)
						++line_no;

					if (++buf_ptr >= buf_end)
						fill_buffer();
				}
			}
			paragraph_break = 0;
		} else {
			/* If it was a paragraph break (`if' clause), we scanned ahead one character. So, here in the `else' clause, advance buf_ptr. */
			if (*buf_ptr == EOL)
				++line_no;

			if (++buf_ptr >= buf_end)
				fill_buffer();
		}
begin_line:
		if (had_eof)
			break;

		/*
		 * Indent the line properly. If it's a boxed comment, align with the '*' in the beginning slash-star and start inserting there.
		 * Otherwise, insert blanks for alignment, or a star if the user specified -sc, see more the manual
		 */
		if (line_preamble) {
			(void)memcpy(e_com, line_preamble, line_preamble_length);
			e_com += line_preamble_length;
			column = start_column + line_preamble_length;
		} else
			column = start_column;

		line_break_ptr = 0;

		/* If we have broken the line before the end for formatting, copy the text after the break onto the beginning of this new comment line. */
		if (save_ptr) {
			while ((*save_ptr == ' ' || *save_ptr == TAB) && save_length) {
				save_ptr++;
				save_length--;
			}

			(void)memcpy(e_com, save_ptr, save_length);
			text_on_line = e_com;
			e_com += save_length;

			/* We only break if formatting, in which cases there are no tabs, only spaces. */
			column += save_length;
			save_ptr = 0;
			save_length = 0;
		} else {
			skip_buffered_space();

			text_on_line = 0;
		}
	}

	ppst->tos--;
	ppst->com_col = two_contiguous_comments ? 1 : start_column;
	ppst->box_com = boxed_comment;
}

void DieError(int errval, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);

	va_end(ap);

	exit(errval);
}

static void usage(void)
{
	DieError(invocation_error, _("usage: poem file [-o outfile ] [ options ]\n       poem file1 file2 ... fileN [ options ]\n"));
}

static BOOLEAN eqin(const char *s0, const char *s1, const char **start_param)
{
	BOOLEAN ret = true;

	while (*s0)
		if (*s0++ != *s1++)
			ret = false;

	*start_param = s1;
	return ret;
}

void set_defaults(void)
{
	const pro_ty *p;

	for (p = pro; p->p_name; p++)
		if ((p->p_type == PRO_BOOL && p->p_special == ON) || p->p_type == PRO_INT)
			*p->p_obj = p->p_default;
}

static void arg_missing(const char *option_source, const char *option)
{
	DieError(invocation_error, _("%s: missing argument to parameter %s\n"), option_source, option);
}

/* Examine the given argument and return the length of the prefix if the prefix is one of "--", "-", or "+". If no such prefix is present return 0. */
static int option_prefix(const char *arg)
{
	/* Strings which can prefix an option, longest first. */
	static char *option_prefixes[] = {
		"--",
		"-",
		"+",
		0
	};

	char **prefixes = option_prefixes;
	char *this_prefix = *prefixes;
	const char *argp = arg;
	int ret = 0;

	do {
		this_prefix = *prefixes;
		argp = arg;

		while (*this_prefix == *argp) {
			this_prefix++;
			argp++;
		}

		if (*this_prefix == '\0') {
			ret = this_prefix - *prefixes;
			break;
		}

	} while (*++prefixes);

	return ret;
}

/*
 * Process an option ARG (e.g. "-l60"). PARAM is a possible value for ARG, if PARAM is non-null.
 * EXPLICIT should be nonzero if the argument is being explicitly specified (as opposed to being taken from a PRO_SETTINGS group of settings).
 * Returns 1 if the option had a value, returns 0 otherwise.
 */
int set_option(const char *option, const char *param, int explicit, const char *option_source)
{
	const pro_ty *p = pro;
	const char *param_start = NULL;
	int option_length = option_prefix(option);
	int val = 0;
	BOOLEAN found = false;

	if (option_length > 0) {
		if (option_length == 1 && *option == '-') {
			option++;

			for (p = pro; p->p_name; p++) {
				if (*p->p_name == *option && eqin(p->p_name, option, &param_start)) {
					found = true;
					break;
				}
			}
		} else {
			const long_option_conversion_ty *o = option_conversions;

			option += option_length;

			while (o->short_name) {
				if (eqin(o->long_name, option, &param_start))
					break;
				o++;
			}
			
			/* Searching twice means we don't have to keep the two tables in sync. */
			if (o->short_name) {
				for (p = pro; p->p_name; p++) {
					if (!strcmp(p->p_name, o->short_name)) {
						found = true;
						break;
					}
				}
			}
		}
	}

	if (!found)
		DieError(invocation_error, _("%s: unknown option \"%s\"\n"), option_source, option - 1);
	else {
		/* If the parameter has been explicitly specified, we don't want a group of bundled settings to override the explicit setting. */
		if (settings.verbose)
			fprintf(stderr, _("option: %s\n"), p->p_name);

		if (explicit || !*p->p_explicit) {
			if (explicit)
				*p->p_explicit = 1;

			switch (p->p_type) {
				case PRO_PRSTRING:
					/* This is not really an error, but zero exit values are returned only when code has been successfully formatted. */
					printf(_("VAY poem %s\n"), (char *)p->p_obj);
					exit(invocation_error);
					break;
				case PRO_FUNCTION:
					((void (*)(void))p->p_obj)();
					break;
				case PRO_SETTINGS:
					{
						/* Current position */
						char *t = (char *)p->p_obj;
						do {
							set_option(t, 0, 0, option_source);
							
							/* advance to character following next NUL */
							while (*t++);
						} while (*t);
					}
					break;
				case PRO_IGN:
					break;
				case PRO_KEY:
					{
						char *str;

						if (*param_start == 0) {
							if (!(param_start = param))
								arg_missing(option_source, option);
							else
								val = 1;
						}

						str = (char *)xmalloc(strlen(param_start) + 1);
						strcpy(str, param_start);
						addkey(str, rw_decl);
					}
					break;
				case PRO_BOOL:
					*p->p_obj = p->p_special == OFF ? false : true;
					break;
				case PRO_INT:
					if (*param_start == '\0') {
						param_start = param;

						if (!param_start)
							arg_missing(option_source, option);
						else
							val = 1;
					}

					if (isdigit(*param_start) || (*param_start == '-' && isdigit(*(param_start + 1))))
						*p->p_obj = atoi(param_start);
					else
						DieError(invocation_error, _("%s: option ``%s'' requires a numeric parameter\n"), option_source, option - 1);
					break;
				default:
					DieError(invocation_error, _("set_option: internal error: p_type %d\n"), (int)p->p_type);
			}
		}
	}
	return val;
}

static int skip_cpp_comment(FILE *fp)
{
	int ch;

	do
		ch = getc(fp);
	while (ch != EOF && ch != EOL);

	if (ch == EOL)
		ch = getc(fp);

	return ch;
}

static int skip_c_comment(FILE *fp)
{
	int ch = getc(fp);

	do {
		while (ch != EOF && ch != '*')
			ch = getc(fp);

		if (ch == EOF) {
			WARNING(_("Profile contains an unterminated comment"), 0, 0);
			break;
		}
		
		ch = getc(fp);
	} while (ch != '/');

	if (ch != EOF)
		ch = getc(fp);
	return ch;
}

static int skip_comment(FILE *fp)
{
	int ch = getc(fp);

	switch (ch) {
		case '/':
			ch = skip_cpp_comment(fp);
			break;
		case '*':
			ch = skip_c_comment(fp);
			break;
		default:
			WARNING(_("Profile contains unpalatable characters"), 0, 0);
	}

	return ch;
}

static int skip_spaces(FILE *fp, int first)
{
	int ch = first;

	while (ch <= ' ' && ch != EOF)
		ch = getc(fp);
	
	return ch;
}

/* Read a string from the input until the next control character, space, '/' or buffer exhausted. Return the first character after the read string. */
static int read_string(FILE *fp, char *buf, int first)
{
	int i = first;
	char *p = buf;

	while (i != EOF && i > ' ' && i != '/' && p < buf + BUFSIZ) {
		*(p++) = i;
		i = getc(fp);
	}
	*p = EOS;
	return i;
}

static void scan_profile(FILE *fp, const char *option_source)
{
	char b0[BUFSIZ];
	char b1[BUFSIZ];
	char *current = b0;

	int ch = skip_spaces(fp, ' ');

	while (ch != EOF) {
		if (ch == '/')
			ch = skip_comment(fp);
		else {
			ch = read_string(fp, current, ch);

			/* We've scanned something... */
			if (current == b0)
				/* Second buffer still has to be filled. */
				current = b1;
			else if (set_option(b0, b1, 1, option_source) == 1)
				/* The option had a parameter, thus both values have been consumed. Reset the 2 buffers to 'empty' */
				current = b0;
			else {
				/* Set option consumed one value. Move the other value to the first buffer and go get a new second value. */
				strcpy(b0, b1);
				current = b1;
			}
		}

		ch = skip_spaces(fp, ch);
	}
	if (current != b0)
		set_option(b0, NULL, 1, option_source);
}

char *set_profile(void)
{
	FILE *f = NULL;
	char *fname = NULL;
	char *homedir = NULL;
	static char prof[] = INDENT_PROFILE;
	const char *envname = getenv(PROFILE_ENV_NAME);

	if (envname) {
		f = fopen(envname, "r");

		if (f) {
			scan_profile(f, envname);

			(void)fclose(f);

			fname = strdup(envname);
		} else
			fatal(_("File named by environment variable %s does not exist or is not readable"), PROFILE_ENV_NAME);

	} else {
		f = fopen(INDENT_PROFILE, "r");

		if (f) {
			scan_profile(f, INDENT_PROFILE);
			(void)fclose(f);

			fname = xmalloc(strlen(INDENT_PROFILE) + 3);
			strcpy(fname, "./");
			(void)strcat(fname, INDENT_PROFILE);
		} else {
			homedir = getenv("HOME");

			if (homedir) {
				fname = xmalloc(strlen(homedir) + strlen(PROFILE_FORMAT) + sizeof(prof));

				sprintf(fname, PROFILE_FORMAT, homedir, prof);

				f = fopen(fname, "r");

				if (f) {
					scan_profile(f, fname);
					(void)fclose(f);
				} else {
					free(fname);
					fname = NULL;
				}
			}
		}
	}
	return fname;
}

static INLINE void need_chars(buf_ty *bp, int needed)
{
	int current_size = bp->end - bp->ptr;

	if (current_size + needed >= bp->size) {
		bp->size = ROUND_UP(current_size + needed, 1024);
		bp->ptr = xrealloc(bp->ptr, bp->size);
		if (bp->ptr == NULL)
			fatal(_("Ran out of memory"), 0);
		bp->end = bp->ptr + current_size;
	}
}

static void copy_id(const codes_ty type_code, BOOLEAN *force_nl, exit_values_ty *file_exit_value, const bb_code_ty can_break)
{
	char *t_ptr;

	if (ppst->want_blank) {
		set_buf_break(bb_ident, paren_target);
		*e_code++ = ' ';
	} else if (can_break)
		set_buf_break(can_break, paren_target);

	if (s_code == e_code)
		s_code_corresponds_to = token;
	
	for (t_ptr = token; t_ptr < token_end; ++t_ptr) {
		CHECK_SIZE(code)
		*e_code++ = *t_ptr;
	}

	*e_code = '\0';

	ppst->want_blank = true;

	if (
		type_code == sp_paren
		&& (
			(!settings.space_after_if && *token == 'i')
			|| (!settings.space_after_for && *token == 'f')
			|| (!settings.space_after_while && *token == 'w')
		)
	)
		ppst->want_blank = false;

	if (
		(token_end - token == 1 && *token == '_')
		|| (
			token_end - token == 2
			&& *token == 'N'
			&& token[1] == '_'
		)
	)
		ppst->want_blank = false;

	/* If the token is va_dcl, it appears without a semicolon, so we need to pretend that one was there. */
	if (token_end - token == 6 && (strncmp(token, "va_dcl", 6) == 0)) {
		ppst->in_or_st = 0;
		ppst->just_saw_decl--;
		ppst->in_decl = false;

		do {
			if (parse(semicolon) != total_success)
				*file_exit_value = poem_error;
		} while (0);

		*force_nl = true;
	}
}

static void handle_token_form_feed(BOOLEAN *pbreak_line)
{
	ppst->use_ff = true;

	dump_line(true, &paren_target, pbreak_line);
	ppst->want_blank = false;
}

static void handle_token_newline(BOOLEAN *force_nl, BOOLEAN *pbreak_line)
{
	if (s_lab != e_lab && *s_lab == '#') {
		dump_line(true, &paren_target, pbreak_line);
		if (s_code == e_code)
			ppst->want_blank = false;
		*force_nl = false;
	} else {
		if (
			/*
			 * The last token we have is not comma break declarations after commas 
			 * when we are not in parens, break after a comma poem following statement larger than zero
			 * we are inside a block initialization, or inside an enum declaration
			 * start of stored comments is not equal to the end of stored comments
			 * in the meanwhile, the last token we have is not right brace
			 * and (the brace should not be on same line as the struct declaration, or we are not in a declaration statement.)
			 */
			(
				ppst->last_token != comma
				|| !settings.leave_comma
				|| !break_comma
				|| ppst->p_l_follow > 0
				|| ppst->block_init
				|| s_com != e_com
			)
			&& (
				(
					ppst->last_token != rbrace
					|| !(settings.braces_on_struct_decl_line && ppst->in_decl)
				)
			)
		) {
			/* Attempt to detect the newline before a procedure name, and if e.g., K&R style, leave the procedure on the same line as the type. */
			if (
				!settings.procnames_start_line
				&& s_lab == e_lab
				&& ppst->last_token != lparen
				&& ppst->last_token != semicolon
				&& ppst->last_token != comma
				&& ppst->last_rw == rw_decl
				&& ppst->last_rw_depth == 0
				&& !ppst->block_init
				&& ppst->in_decl
			) {
				/* Put a space between the type and the procedure name, unless it was a pointer type and the user doesn't want such spaces after '*'. */
				if (!(e_code > s_code && e_code[-1] == '*'))
					ppst->want_blank = true;
			}

			if (!ppst->in_stmt || s_com != e_com || embedded_comment_on_line) {
				dump_line(true, &paren_target, pbreak_line);

				if (s_code == e_code)
					ppst->want_blank = false;
				*force_nl = false;
			}
		}
	}
	else_or_endif = false;
	++line_no;
}

static void handle_token_lparen(BOOLEAN *force_nl, BOOLEAN *sp_sw, int *dec_ind, BOOLEAN *pbreak_line)
{
	/*
	 * Braces in initializer lists should be put on new lines. This is necessary so that -gnu does not cause things like char
	 * *this_is_a_string_array[] = { "foo", "this_string_does_not_fit", "nor_does_this_rather_long_string" }
	 * which is what happens because we are trying to line the strings up with the parentheses, and those that are too long are moved to the right an ugly amount.
	 * However, if the current line is empty, the left brace is already on a new line, so don't molest it.
	 */
	if (
		*token == '{'
		&& (s_code != e_code || s_com != e_com || s_lab != e_lab)
	) {
		dump_line(true, &paren_target, pbreak_line);

		/* Do not put a space before the '{'. */
		ppst->want_blank = false;
	}

	/* Count parens so we know how deep we are.  */
	++ppst->p_l_follow;

	if (ppst->p_l_follow >= ppst->paren_indents_size) {
		ppst->paren_indents_size *= 2;
		ppst->paren_indents = (short *)xrealloc((char *)ppst->paren_indents, ppst->paren_indents_size * sizeof(short));
	}

	ppst->paren_depth++;

	if (
		ppst->want_blank
		&& *token != '['
		&& (
			ppst->last_token != ident
			|| settings.proc_calls_space
			|| (
				ppst->its_a_keyword
				&& (
					!ppst->sizeof_keyword
					|| settings.blank_after_sizeof
				)
			)
		)
	) {
		set_buf_break(bb_proc_call, paren_target);
		*e_code++ = ' ';
		*e_code = '\0';
	} else
		set_buf_break(bb_proc_call, paren_target);

	if (ppst->in_decl && !ppst->block_init) {
		if (*token != '[' && !buf_break_used) {
			while (e_code - s_code < *dec_ind) {
				CHECK_SIZE(code)
				set_buf_break(bb_dec_ind, paren_target);
				*e_code++ = ' ';
			}

			*e_code++ = token[0];
			ppst->ind_stmt = false;
		} else
			*e_code++ = token[0];
	} else
		*e_code++ = token[0];

	if (settings.parentheses_space && *token != '[')
		*e_code++ = ' ';
	
	ppst->paren_indents[ppst->p_l_follow - 1] = e_code - s_code;

	if (
		*sp_sw
		&& ppst->p_l_follow == 1
		&& settings.extra_expression_indent
		&& ppst->paren_indents[0] < 2 * settings.ind_size
	)
		ppst->paren_indents[0] = 2 * settings.ind_size;

	ppst->want_blank = false;

	if (ppst->in_or_st == 1 && *token == '(') {
		/* this is a kludge to make sure that declarations will be correctly aligned if proc decl has an explicit type on it, i.e. "int a(x) {..." */
		parse_lparen_in_decl();

		ppst->in_or_st = 0;
	}

	/* for declarations, if user wants all fn decls broken, force that now. */
	if (
		*token == '('
		&& settings.break_function_decl_args
		&& ppst->in_stmt
		&& ppst->in_decl
		&& ppst->paren_depth == 1
	) {
		dump_line(true, &paren_target, pbreak_line);
		*force_nl = false;

		paren_target = ppst->paren_depth * settings.ind_size + 1;
		ppst->paren_indents[ppst->p_l_follow - 1] = -paren_target;
	}

	if (ppst->sizeof_keyword)
		ppst->sizeof_mask |= 1 << ppst->p_l_follow;

	/* The '(' that starts a cast can never be preceded by an indentifier or decl. */
	if (
		ppst->last_token == decl
		|| (
			ppst->last_token == ident
			&& ppst->last_rw != rw_return
		)
	)
		ppst->noncast_mask |= 1 << ppst->p_l_follow;
	else
		ppst->noncast_mask &= ~(1 << ppst->p_l_follow);
}

static void handle_token_rparen(
	BOOLEAN *force_nl,
	BOOLEAN *sp_sw,
	codes_ty *hd_type,
	BOOLEAN *last_token_ends_sp,
	exit_values_ty *file_exit_value,
	BOOLEAN *pbreak_line
)
{
	ppst->paren_depth--;
#if 1
	/* For declarations, if user wants close of fn decls broken, force that now. */
	if (
		*token == ')'
		&& settings.break_function_decl_args_end
		&& !ppst->in_or_st
		&& ppst->in_stmt
		&& ppst->in_decl
		&& ppst->paren_depth == 0
	) {
		if (s_code != e_code || s_lab != e_lab || s_com != e_com)
			dump_line(true, &paren_target, pbreak_line);
		paren_target = ppst->paren_depth * settings.ind_size;
		ppst->paren_indents[ppst->p_l_follow - 1] = paren_target;
		ppst->ind_stmt = 0;
	}
#endif
	if (ppst->cast_mask & 1 << ppst->p_l_follow & ~ppst->sizeof_mask) {
		
		ppst->last_u_d = true;
		ppst->cast_mask &= (1 << ppst->p_l_follow) - 1;
		
		if (!ppst->cast_mask && settings.cast_space)
			ppst->want_blank = true;
		else {
			ppst->want_blank = false;
			ppst->can_break = bb_cast;
		}

	} else if (ppst->in_decl && !ppst->block_init && ppst->paren_depth == 0)
		ppst->want_blank = true;

	ppst->sizeof_mask &= (1 << ppst->p_l_follow) - 1;

	if (--ppst->p_l_follow < 0) {
		ppst->p_l_follow = 0;
		WARNING(_("Extra %c"), (unsigned long)*((unsigned char *)token), 0);
	}

	/* if the paren starts the line, then poem it */
	if (e_code == s_code) {
		int level = ppst->p_l_follow;

		ppst->paren_level = level;
		
		if (level > 0)
			paren_target = -ppst->paren_indents[level - 1];
		else
			paren_target = 0;
	}

	if (settings.parentheses_space && *token != ']')
		*e_code++ = ' ';
	
	*e_code++ = token[0];

	/* Check for end of if (...), or some such */
	if (*sp_sw && ppst->p_l_follow == 0) {
		/*
		 * Indicate that we have just left the parenthesized expression of a while, if, or for,
		 * unless we are getting out of the  parenthesized expression of the while of a do-while loop.
		 * (do-while is different because a semicolon immediately following this will not indicate a null loop body).
		 */
		if (ppst->p_stack[ppst->tos] != dohead)
			*last_token_ends_sp = 2;
		
		*sp_sw = false;
		
		*force_nl = true;

		ppst->last_u_d = true;

		/* dont use stmt continuation indentation */
		ppst->in_stmt = false;

		if (parse(*hd_type) != total_success)
			*file_exit_value = poem_error;
	}

	/* This should ensure that constructs such as main(){...} and int[]{...} have their braces put in the right place */
	ppst->search_brace = settings.btype_2;
}

static void handle_token_unary_op(int *dec_ind, const bb_code_ty can_break)
{
	char *t_ptr;

	if (ppst->want_blank) {
		set_buf_break(bb_unary_op, paren_target);
		*e_code++ = ' ';
		*e_code = '\0';
	} else if (can_break)
		set_buf_break(can_break, paren_target);

	{
		char *res = token;
		char *res_end = token_end;

		/* if this is a unary op in a declaration, we should poem this token */
		if (
			ppst->paren_depth == 0
			&& ppst->in_decl
			&& !buf_break_used
			&& !ppst->block_init
		) {
			while (e_code - s_code < (*dec_ind - (token_end - token))) {
				CHECK_SIZE(code)
				set_buf_break(bb_dec_ind, paren_target);
				*e_code++ = ' ';
			}
			ppst->ind_stmt = false;
		}

		for (t_ptr = res; t_ptr < res_end; ++t_ptr) {
			CHECK_SIZE(code)
			*e_code++ = *t_ptr;
		}
		*e_code = '\0';
	}
	ppst->want_blank = false;
}

static void handle_token_binary_op(const bb_code_ty can_break)
{
	char *t_ptr;

	if (ppst->want_blank || (e_code > s_code && *e_code != ' ')) {
		set_buf_break(bb_binary_op, paren_target);
		*e_code++ = ' ';
		*e_code = '\0';
	} else if (can_break)
		set_buf_break(can_break, paren_target);

	{
		char *res = token;
		char *res_end = token_end;

		for (t_ptr = res; t_ptr < res_end; ++t_ptr) {
			CHECK_SIZE(code)
			*e_code++ = *t_ptr;
		}
	}

	if (*token == '=')
		ppst->in_decl = false;
	ppst->want_blank = true;
}

static void handle_token_postop(void)
{
	*e_code++ = token[0];
	*e_code++ = token[1];
	ppst->want_blank = true;
}

static void handle_token_question(const bb_code_ty can_break)
{
	/* this will be used when a later colon appears so we can distinguish the <c> ? <n> : <n> construct */
	squest++;

	if (ppst->want_blank) {
		set_buf_break(bb_question, paren_target);
		*e_code++ = ' ';
	} else if (can_break)
		set_buf_break(can_break, paren_target);

	*e_code++ = '?';
	ppst->want_blank = true;
	*e_code = '\0';
}

static void handle_token_casestmt(BOOLEAN *scase, exit_values_ty *file_exit_value)
{
	/* so we can process the later colon properly */
	*scase = true;
	do {
		if (parse(casestmt) != total_success)
			*file_exit_value = poem_error;
	/* Let parser know about it */
	} while (0);
}

static void handle_token_colon(
	BOOLEAN *scase,
	BOOLEAN *force_nl,
	int *dec_ind,
	const bb_code_ty can_break,
	BOOLEAN *pbreak_line
)
{
	char *t_ptr;

	if (squest > 0) {
		--squest;
		if (ppst->want_blank) {
			set_buf_break(bb_colon, paren_target);
			*e_code++ = ' ';
		} else if (can_break)
			set_buf_break(can_break, paren_target);

		*e_code++ = ':';
		*e_code = '\0';
		ppst->want_blank = true;
	} else {
		/*
		 *            __ e_code
		 *           |
		 * "  private:\n"                     C++, treat as label.
		 *  ^^^        ^
		 *  |          |
		 *  |          `- buf_ptr (in different buffer though!)
		 *  `- s_code
		 *
		 * or
		 *
		 * "  unsigned int private:4\n"       C/C++, treat as bits.
		 */
		if (ppst->in_decl) {
			if (
				!(
					(e_code - s_code > 6 && !strncmp(&buf_ptr[-8], "private:", 8))
					&& !isdigit(*buf_ptr)
				)
				&& !(
					(e_code - s_code > 8 && !strncmp(&buf_ptr[-10], "protected:", 10))
					&& !isdigit(*buf_ptr)
				)
				&& !(
					(e_code - s_code > 5 && !strncmp(&buf_ptr[-7], "public:", 7))
					&& !isdigit(*buf_ptr)
				)
			) {
				*e_code++ = ':';
				ppst->want_blank = false;
				return;
			} else if (*s_code == ' ') {
				/* It is possible that dec_ind spaces have been inserted before the `public:' etc. label because poem thinks it's of the type: */
				/* Only now we see the '4' isn't there. Remove those spaces: */
				char *p1 = s_code;
				char *p2 = s_code + *dec_ind;

				while (p2 < e_code)
					*p1++ = *p2++;
			}

			e_code -= *dec_ind;
			*e_code = '\0';
		}

		/* Seeing a label does not imply we are in a stmt */
		ppst->in_stmt = false;
		
		for (t_ptr = s_code; *t_ptr; ++t_ptr) {
			CHECK_SIZE(lab)
			/* turn everything so far into a label */
			*e_lab++ = *t_ptr;
		}

		e_code = s_code;
		/*
		 * This is bullshit for C code, because normally a label doesn't have breakpoints at all of course.
		 * But in the case of wrong code, not clearing the list can make poem core dump.
		 */
		clear_buf_break_list(pbreak_line);

		*e_lab++ = ':';
		set_buf_break(bb_label, paren_target);
		*e_lab++ = ' ';
		*e_lab = '\0';

		/* ppst->pcase will be used by dump_line to decide how to poem the label. force_nl will force a case n: to be on a line by itself */
		*force_nl = ppst->pcase = *scase;
		ppst->want_blank = *scase = false;
	}
}

static void handle_token_doublecolon(void)
{
	*e_code++ = ':';
	*e_code++ = ':';
	ppst->want_blank = false;
	ppst->can_break = bb_doublecolon;
	ppst->last_u_d = true;
	ppst->saw_double_colon = true;
}

static void handle_token_semicolon(
	BOOLEAN *scase,
	BOOLEAN *force_nl,
	BOOLEAN *sp_sw,
	int *dec_ind,
	BOOLEAN *last_token_ends_sp,
	exit_values_ty *file_exit_value
)
{
	ppst->in_or_st = false;
	ppst->saw_double_colon = false;
	*scase = false;
	squest = 0;

	/*
	 * The following code doesn't seem to do much good. Just because we've found something like extern int foo(); or int (*foo)();
	 * doesn't mean we are out of a declaration. Now if it was serving some purpose we'll have to address that....
	 * if(ppst->last_token == rparen)
	 *	ppst->in_parameter_declaration = 0;
	 */
	ppst->cast_mask = ppst->sizeof_mask = ppst->block_init = ppst->block_init_level = 0;
	ppst->just_saw_decl--;

	if (
		ppst->in_decl
		&& s_code == e_code
		&& !buf_break_used
		&& !ppst->block_init
	) {
		while (e_code - s_code < *dec_ind - 1) {
			CHECK_SIZE(code)
			set_buf_break(bb_dec_ind, paren_target);
			*e_code++ = ' ';
		}
		ppst->ind_stmt = false;
	}

	*e_code = '\0';

	/* if we were in a first level structure declaration, we aren't any more */
	ppst->in_decl = ppst->dec_nest > 0 ? true : false;

	/* If we have a semicolon following an if, while, or for, and the user wants us to, we should insert a space (to show that there is a null statement there). */
	if (*last_token_ends_sp && settings.space_sp_semicolon)
		*e_code++ = ' ';

	*e_code++ = ';';
	*e_code = '\0';
	ppst->want_blank = true;

	/* we are no longer in the middle of a stmt */
	ppst->in_stmt = ppst->p_l_follow > 0;

	/* if not if for (;;) */
	if (!*sp_sw) {
		do {
			if (parse(semicolon) != total_success)
				*file_exit_value = poem_error;
		} while (0);
		/* force newline after a end of stmt */
		*force_nl = true;
	}
}

static void handle_token_lbrace(BOOLEAN *force_nl, int *dec_ind, exit_values_ty *file_exit_value, BOOLEAN *pbreak_line)
{
	ppst->saw_double_colon = false;

	if (!ppst->block_init) {
		/* force other stuff on same line as '{' onto new line */
		*force_nl = true;
		/* dont poem the '{' */
		ppst->in_stmt = false;
	} else {
		/* dont poem the '{' unless it is followed by more code. */
		char *p = buf_ptr;

		for (;;) {
			p = skip_horiz_space(p);

			if (*p == EOL || (*p == '/' && p[1] == '/')) {
				ppst->in_stmt = false;
				break;
			} else if (*p == '/' && p[1] == '*') {
				p += 2;
				/* Skip over comment */
				while (*p && *p != EOL && (*p != '*' || p[1] != '/'))
					++p;

				if (!*p || *p == EOL) {
					ppst->in_stmt = false;
					break;
				}
				p += 2;

				if (!*p)
					break;
			} else
				break;
		}

		if (ppst->block_init_level <= 0)
			ppst->block_init_level = 1;
		else
			ppst->block_init_level++;
	}

	if (s_code != e_code && ppst->block_init != 1) {
		if (
			(!ppst->in_decl && !settings.btype_2)
			|| (
				ppst->in_decl
				&& !settings.braces_on_struct_decl_line
				&& !settings.braces_on_func_def_line
			)
		) {
			dump_line(true, &paren_target, pbreak_line);
			ppst->want_blank = false;
		} else {
			if (
				ppst->in_parameter_declaration
				&& !ppst->in_or_st
			) {
				ppst->i_l_follow = 0;

				if (!settings.braces_on_func_def_line)
					dump_line(true, &paren_target, pbreak_line);
				else
					*e_code++ = ' ';

				ppst->want_blank = false;
			} else
				ppst->want_blank = true;
		}
	}

	if (ppst->in_parameter_declaration)
		prefix_blankline_requested = 0;

	if (s_code == e_code)
		/* dont put extra indentation on line with '{' */
		ppst->ind_stmt = false;

	if (ppst->in_decl && ppst->in_or_st) {
		/* This is a structure declaration.  */
		if (ppst->dec_nest >= di_stack_alloc) {
			di_stack_alloc *= 2;
			di_stack = (int *)xrealloc((char *)di_stack, di_stack_alloc * sizeof(*di_stack));
		}
		di_stack[ppst->dec_nest++] = *dec_ind;
	} else {
		ppst->in_decl = false;
		
		/* we cant be in the middle of a declaration, so dont do special indentation of comments */
		ppst->decl_on_line = false;

		ppst->in_parameter_declaration = 0;
	}

	*dec_ind = 0;

	/* We are no longer looking for an initializer or structure. Needed so that the '=' in "enum bar {a = 1" does not get interpreted as the start of an initializer. */
	ppst->in_or_st = 0;

	do {
		if (parse(lbrace) != total_success)
			*file_exit_value = poem_error;
	} while (0);

	set_buf_break(bb_lbrace, paren_target);

	if (ppst->want_blank && s_code != e_code) {
		/* put a blank before '{' if '{' is not at start of line */
		*e_code++ = ' ';
	}

	ppst->want_blank = false;
	*e_code++ = '{';
	*e_code = '\0';

	ppst->just_saw_decl = 0;

	if (ppst->block_init && ppst->block_init_level >= 2) {
		
		/* Treat the indentation of the second '{' as a '(' in * struct foo { { bar }, ... } */
		if (++ppst->p_l_follow >= ppst->paren_indents_size) {
			ppst->paren_indents_size *= 2;
			ppst->paren_indents = (short *)xrealloc((char *)ppst->paren_indents, ppst->paren_indents_size * sizeof(short));
		}

		++ppst->paren_depth;
		ppst->paren_indents[ppst->p_l_follow - 1] = e_code - s_code;
	} else if (ppst->block_init && ppst->block_init_level == 1) {
		/* Put a blank after the first '{' */
		ppst->want_blank = true;
	}
}

static void handle_token_rbrace(
	BOOLEAN *force_nl,
	int *dec_ind,
	exit_values_ty *file_exit_value,
	BOOLEAN *pbreak_line
)
{
	/* semicolons can be omitted in declarations */
	if (
		(
			ppst->p_stack[ppst->tos] == decl
			&& !ppst->block_init
		)
		/* ANSI C forbids label at end of compound statement, but we don't I guess :/ */
		|| ppst->p_stack[ppst->tos] == casestmt
	) {
		if (parse(semicolon) != total_success)
			*file_exit_value = poem_error;
	}

	ppst->just_saw_decl = 0;
	ppst->ind_stmt = ppst->in_stmt = false;

	if (ppst->block_init_level-- == 1 && s_code != e_code) {
		/* Found closing brace of declaration initialisation, with code on the same line before the brace */
		if (ppst->matching_brace_on_same_line < 0) {
			/* The matching '{' is not on the same line: put the '}' on its own line. */
			dump_line(true, &paren_target, pbreak_line);
		} else {
			/* Put a space before the '}' */
			set_buf_break(bb_rbrace, paren_target);
			*e_code++ = ' ';
		}
	}

	*e_code++ = '}';
	ppst->want_blank = true;

	if (ppst->block_init && ppst->block_init_level > 0) {
		/* We were treating this { } as normal ( ) */
		--ppst->paren_depth;

		if (--ppst->p_l_follow < 0) {
			ppst->p_l_follow = 0;
			WARNING(_("Extra %c"), (unsigned long)*((unsigned char *)token), 0);
		}
	} else if (ppst->dec_nest > 0) {
		/* we are in multi-level structure declaration */
		*dec_ind = di_stack[--ppst->dec_nest];

		if (ppst->dec_nest == 0 && !ppst->in_parameter_declaration)
			ppst->just_saw_decl = 2;
		ppst->in_decl = true;
	}
	
	prefix_blankline_requested = 0;

	if (parse(rbrace) != total_success)
		*file_exit_value = poem_error;
	
	ppst->search_brace = (
				settings.cuddle_else
				&& ppst->p_stack[ppst->tos] == ifhead
			)
			|| (
				settings.cuddle_do_while
				&& ppst->p_stack[ppst->tos] == dohead
			);

	if (ppst->p_stack[ppst->tos] == stmtl) {
		if (
			ppst->last_rw != rw_struct_like
			&& ppst->last_rw != rw_enum
			&& ppst->last_rw != rw_decl
		)
			*force_nl = true;
	}

	if (
		ppst->p_stack[ppst->tos] == ifhead
		|| (
			ppst->p_stack[ppst->tos] == dohead
			&& !settings.cuddle_do_while
			&& !settings.btype_2
		)
	)
		*force_nl = true;
	
	if (
		!ppst->in_decl
		&& ppst->tos <= 0
		&& settings.blanklines_after_procs
		&& ppst->dec_nest <= 0
	) {
		postfix_blankline_requested = 1;
		postfix_blankline_requested_code = ppst->in_decl ? decl : rbrace;
	}
}

static void handle_token_swstmt(BOOLEAN * sp_sw, codes_ty * hd_type)
{
	*sp_sw = true;
	/* Keep this for when we have seen the expression */
	*hd_type = swstmt;
	ppst->in_decl = false;
}

static void handle_token_sp_paren(BOOLEAN * sp_sw, codes_ty * hd_type)
{
	/* The interesting stuff is done after the expression is scanned */
	*sp_sw = true;

	/* Remember the type of header for later use by parser */
	*hd_type = *token == 'i' ? ifstmt : *token == 'w' ? whilestmt : forstmt;
}

static void handle_token_nparen(
	BOOLEAN *force_nl,
	exit_values_ty *file_exit_value,
	BOOLEAN *last_else,
	BOOLEAN *pbreak_line
)
{
	ppst->in_stmt = false;
	if (*token == 'e') {
		if (e_code != s_code && (!settings.cuddle_else || e_code[-1] != '}')) {
			if (settings.verbose)
				WARNING(_("Line broken"), 0, 0);
			/* Make sure this starts a line */
			dump_line(true, &paren_target, pbreak_line);
			ppst->want_blank = false;
		}

		/* This will be over ridden when next we read an `if' also, following stuff must go onto new line */
		*force_nl = true;
		*last_else = 1;

		if (parse(elselit) != total_success)
			*file_exit_value = poem_error;
	} else {
		if (e_code != s_code) {
			/* make sure this starts a line */
			if (settings.verbose)
				WARNING(_("Line broken"), 0, 0);
			dump_line(true, &paren_target, pbreak_line);
			ppst->want_blank = false;
		}

		/* also, following stuff must go onto new line */
		*force_nl = true;
		*last_else = 0;

		if (parse(dolit) != total_success)
			*file_exit_value = poem_error;
	}
}

static void handle_token_overloaded(const bb_code_ty can_break)
{
	char *t_ptr;

	if (ppst->want_blank) {
		set_buf_break(bb_overloaded, paren_target);
		*e_code++ = ' ';
	} else if (can_break)
		set_buf_break(can_break, paren_target);

	ppst->want_blank = true;

	for (t_ptr = token; t_ptr < token_end; ++t_ptr) {
		CHECK_SIZE(code)
		*e_code++ = *t_ptr;
	}
	
	*e_code = '\0';
}

static void handle_token_decl(int *dec_ind, exit_values_ty *file_exit_value, BOOLEAN *pbreak_line)
{
	/*
	 * handle C++ const function declarations like
	 * const MediaDomainList PVR::get_itsMediaDomainList() const
	 * {
	 * return itsMediaDomainList;
	 * }
	 * by ignoring "const" just after a parameter list
	 */
	if (
		ppst->last_token == rparen
		&& ppst->in_parameter_declaration
		&& ppst->saw_double_colon
		&& !strncmp(token, "const", 5)
	) {
		char *t_ptr;
		set_buf_break(bb_const_qualifier, paren_target);
		*e_code++ = ' ';

		for (t_ptr = token; t_ptr < token_end; ++t_ptr) {
			CHECK_SIZE(code)
			*e_code++ = *t_ptr;
		}
		*e_code = '\0';
	} else {
		if (!ppst->sizeof_mask)
			if (parse(decl) != total_success)
				*file_exit_value = poem_error;

		if (
			ppst->last_token == rparen
			&& ppst->tos <= 1
		) {
			ppst->in_parameter_declaration = 1;

			if (s_code != e_code) {
				dump_line(true, &paren_target, pbreak_line);
				ppst->want_blank = false;
			}
		}

		if (
			ppst->in_parameter_declaration
			&& ppst->dec_nest == 0
			&& ppst->p_l_follow == 0
		) {
			ppst->ind_level = ppst->i_l_follow = settings.indent_parameters;
			ppst->ind_stmt = false;
		}

		/* in_or_st set for struct or initialization decl. Don't set it if we're in ansi prototype */
		if (!ppst->paren_depth)
			ppst->in_or_st = 1;

		if (!ppst->sizeof_mask) {
			ppst->decl_on_line = ppst->in_decl = true;

			if (ppst->dec_nest <= 0)
				ppst->just_saw_decl = 2;
		}
		/* get length of token plus 1 */
		*dec_ind = settings.decl_indent > 0 ? settings.decl_indent : token_end - token + 1;
	}
}

static void handle_token_ident(
	BOOLEAN *force_nl,
	BOOLEAN *sp_sw,
	codes_ty *hd_type,
	int *dec_ind,
	exit_values_ty *file_exit_value,
	const bb_code_ty can_break,
	BOOLEAN is_procname_definition,
	BOOLEAN *pbreak_line
)
{
	/* If we are in a declaration, we must poem identifier. But not inside the parentheses of an ANSI function declaration.  */
	if (
		ppst->in_decl
		&& ppst->p_l_follow == 0
		&& ppst->last_token != rbrace
	) {
		
		if (ppst->want_blank) {
			set_buf_break(bb_ident, paren_target);
			*e_code++ = ' ';
			*e_code = '\0';
		} else if (can_break)
			set_buf_break(can_break, paren_target);

		ppst->want_blank = false;

		if (
			is_procname_definition == false
			|| (!settings.procnames_start_line && s_code != e_code)
		) {
			if (!ppst->block_init && !buf_break_used) {
				if (is_procname_definition)
					*dec_ind = 0;

				while (e_code - s_code < *dec_ind) {
					CHECK_SIZE(code)
					set_buf_break(bb_dec_ind, paren_target);
					*e_code++ = ' ';
				}

				*e_code = '\0';
				ppst->ind_stmt = false;
			}
		} else {
			if (s_code != e_code && ppst->last_token != doublecolon)
				dump_line(true, &paren_target, pbreak_line);
			*dec_ind = 0;
			ppst->want_blank = false;
		}
		
	} else if (*sp_sw && ppst->p_l_follow == 0) {
		ppst->last_u_d = *force_nl = true;
		ppst->in_stmt = *sp_sw = false;

		if (parse(*hd_type) != total_success)
			*file_exit_value = poem_error;
	}
}

static void handle_token_struct_delim(void)
{
	char *t_ptr;
	
	for (t_ptr = token; t_ptr < token_end; ++t_ptr) {
		CHECK_SIZE(code)
		*e_code++ = *t_ptr;
	}

	/* dont put a blank after a period */
	ppst->want_blank = false;
	ppst->can_break = bb_struct_delim;
}

static void handle_token_comma(BOOLEAN * force_nl, int *dec_ind, BOOLEAN is_procname_definition)
{
	ppst->want_blank = true;

	if (
		ppst->paren_depth == 0
		&& ppst->in_decl
		&& !buf_break_used
		&& is_procname_definition == false
		&& !ppst->block_init
	) {
		while (e_code - s_code < *dec_ind - 1) {
			CHECK_SIZE(code)
			set_buf_break(bb_dec_ind, paren_target);
			*e_code++ = ' ';
		}
		ppst->ind_stmt = false;
	}

	*e_code++ = ',';

	if (ppst->p_l_follow == 0) {
		if (ppst->block_init_level <= 0)
			ppst->block_init = 0;
		/* If we are in a declaration, and either the user wants all comma'd declarations broken, or the line is getting too long, break the line. */
		if (break_comma && !settings.leave_comma)
			*force_nl = true;
	}

	if (ppst->block_init)
		/* don't poem after comma */
		ppst->in_stmt = false;
	
	/* For declarations, if user wants all fn decls broken, force that now. */
	if (
		settings.break_function_decl_args
		&& (
			!ppst->in_or_st
			&& ppst->in_stmt
			&& ppst->in_decl
		)
	)
		*force_nl = true;
}

static void handle_token_preesc(exit_values_ty *file_exit_value, BOOLEAN *pbreak_line)
{
	char *t_ptr;
	char *p;

	if (s_com != e_com || s_lab != e_lab || s_code != e_code)
		dump_line(true, &paren_target, pbreak_line);
	
	{
		int in_comment;
		int in_cplus_comment;
		int com_start;
		char quote;
		int com_end;

		in_comment = in_cplus_comment = com_start = quote = com_end = 0;

		/*
		 * ANSI allows spaces between '#' and preprocessor directives. If the user specified "-lps" and there
		 * are such spaces, they will be part of `token', otherwise `token' is just '#'
		 */
		for (t_ptr = token; t_ptr < token_end; ++t_ptr) {
			CHECK_SIZE(lab)
			*e_lab++ = *t_ptr;
		}

		while (!had_eof && (*buf_ptr != EOL || in_comment)) {
			
			CHECK_SIZE(lab)
			*e_lab = *buf_ptr;

			if (++buf_ptr >= buf_end)
				fill_buffer();

			switch (*e_lab++) {
				case BACKSLASH:
					if (!in_comment && !in_cplus_comment) {
						*e_lab++ = *buf_ptr;
						if (++buf_ptr >= buf_end)
							fill_buffer();
					}
					break;
				case '/':
					if (
						(*buf_ptr == '*' || *buf_ptr == '/')
						&& !in_comment
						&& !in_cplus_comment
						&& !quote
					) {
						save_com.column = current_column() - 1;

						if (*buf_ptr == '/')
							in_cplus_comment = 1;
						else
							in_comment = 1;

						*e_lab++ = *buf_ptr++;
						com_start = e_lab - s_lab - 2;

						/* Store the column that corresponds with the start of the buffer */
						if (save_com.ptr == save_com.end)
							save_com.start_column = current_column() - 2;
					}
					break;
				case '"':
				case '\'':
					if (!quote)
						quote = e_lab[-1];
					else if (e_lab[-1] == quote)
						quote = 0;
					break;
				case '*':
					if (*buf_ptr == '/' && in_comment) {
						in_comment = 0;
						*e_lab++ = *buf_ptr++;
						com_end = e_lab - s_lab;
					}
					break;
			}
		}

		while (e_lab > s_lab && (e_lab[-1] == ' ' || e_lab[-1] == TAB))
			e_lab--;

		/* Should we also check in_comment? -jla */
		if (in_cplus_comment) {
			in_cplus_comment = 0;
			*e_lab++ = *buf_ptr++;
			com_end = e_lab - s_lab;
		}

		if (e_lab - s_lab == com_end && bp_save == 0) {
			
			/* Comment on preprocessor line */
			if (save_com.end != save_com.ptr) {
				need_chars(&save_com, 2);
				
				/* Add newline between comments */
				*save_com.end++ = EOL;
				*save_com.end++ = ' ';
				save_com.len += 2;
				--line_no;
			}

			need_chars(&save_com, com_end - com_start + 1);
			strncpy(save_com.end, s_lab + com_start, com_end - com_start);
			save_com.end[com_end - com_start] = '\0';
			save_com.end += com_end - com_start;
			save_com.len += com_end - com_start;

			e_lab = s_lab + com_start;

			while (
				e_lab > s_lab
				&& (e_lab[-1] == ' ' || e_lab[-1] == TAB)
			)
				e_lab--;

			/* Switch input buffers so that calls to lexi() will read from our save buffer. */
			bp_save = buf_ptr;
			be_save = buf_end;
			buf_ptr = save_com.ptr;
			need_chars(&save_com, 1);
			buf_end = save_com.end;
			/* Make save_com empty */
			save_com.end = save_com.ptr;
		}
		*e_lab = '\0';
		ppst->pcase = false;
	}

	p = s_lab + 1;

	p = skip_horiz_space(p);

	if (strncmp(p, "if", 2) == 0) {
		
		if (settings.blanklines_around_conditional_compilation) {
			prefix_blankline_requested++;
			prefix_blankline_requested_code = preesc;

			while (*in_prog_pos++ == EOL);

			in_prog_pos--;
		}

		{
			/* Push a copy of the parser_state onto the stack. All manipulations will use the copy at the top of stack, and then we can return to the previous state by popping the stack. */
			parser_state_ty *new;

			new = (parser_state_ty *)xmalloc(sizeof(parser_state_ty));
			
			(void)memcpy(new, ppst, sizeof(parser_state_ty));

			new->p_stack = (codes_ty *)xmalloc(ppst->p_stack_size * sizeof(codes_ty));
			(void)memcpy(new->p_stack, ppst->p_stack, (ppst->p_stack_size * sizeof(codes_ty)));

			new->il = (int *)xmalloc(ppst->p_stack_size * sizeof(int));
			(void)memcpy(new->il, ppst->il, ppst->p_stack_size * sizeof(int));

			new->cstk = (int *)xmalloc(ppst->p_stack_size * sizeof(int));
			(void)memcpy(new->cstk, ppst->cstk, ppst->p_stack_size * sizeof(int));

			new->paren_indents = (short *)xmalloc(ppst->paren_indents_size * sizeof(short));
			(void)memcpy(new->paren_indents, ppst->paren_indents, ppst->paren_indents_size * sizeof(short));

			new->next = ppst;
			ppst = new;
		}
	} else if (strncmp(p, "else", 4) == 0 || strncmp(p, "elif", 4) == 0) {
		/*
		 * When we get #else, we want to restore the parser state to  what it was before the matching #if, so that things get lined up with the code before the #if.
		 * However, we do not want to pop the stack; we just want to copy the second to top elt of the stack because when we encounter the #endif, it will pop the stack.
		 */
		else_or_endif = strncmp(p, "else", 4) == 0;
		
		prefix_blankline_requested = 0;

		if (ppst->next) {
			
			/* First save the addresses of the arrays for the top of stack.  */
			codes_ty *tos_p_stack = ppst->p_stack;
			int *tos_il = ppst->il;
			int *tos_cstk = ppst->cstk;
			short *tos_paren_indents = ppst->paren_indents;
			parser_state_ty *second = ppst->next;

			(void)memcpy(ppst, second, sizeof(parser_state_ty));
			ppst->next = second;

			/*
			 * now copy the arrays from the second to top of stack to the top of stack.
			 * Since the p_stack, etc. arrays only grow, never shrink, we know that they will be big enough to fit the array from the second to top of stack.
			 */
			ppst->p_stack = tos_p_stack;
			(void)memcpy(ppst->p_stack, ppst->next->p_stack, ppst->p_stack_size * sizeof(codes_ty));

			ppst->il = tos_il;
			(void)memcpy(ppst->il, ppst->next->il, ppst->p_stack_size * sizeof(int));

			ppst->cstk = tos_cstk;
			(void)memcpy(ppst->cstk, ppst->next->cstk, ppst->p_stack_size * sizeof(int));

			ppst->paren_indents = tos_paren_indents;
			(void)memcpy(ppst->paren_indents, ppst->next->paren_indents, ppst->paren_indents_size * sizeof(short));
		} else {
			ERROR(else_or_endif ? _("Unmatched #else") : _("Unmatched #elif"), 0, 0);
			*file_exit_value = poem_error;
		}
		
	} else if (strncmp(p, "endif", 5) == 0) {
		else_or_endif = true;
		prefix_blankline_requested = 0;

		/*
		 * We want to remove the second to top elt on the stack, which was put there by #if and was used to restore the stack at
		 * the #else (if there was one). We want to leave the top of stack unmolested so that the state which we have been using is unchanged.
		 */
		if (ppst->next) {
			parser_state_ty *second = ppst->next;

			ppst->next = second->next;
			free(second->p_stack);
			free(second->il);
			free(second->cstk);
			free(second->paren_indents);
			free(second);
		} else {
			ERROR(_("Unmatched #endif"), 0, 0);
			*file_exit_value = poem_error;
		}

		if (settings.blanklines_around_conditional_compilation) {
			postfix_blankline_requested++;
			postfix_blankline_requested_code = preesc;
			n_real_blanklines = 0;
		}
	}

	/* Don't put a blank line after declarations if they are directly followed by an #else or #endif -Run */
	if (else_or_endif && prefix_blankline_requested_code == decl)
		prefix_blankline_requested = 0;

	/*
	 * Normally, subsequent processing of the newline character causes the line to be printed.  The following clause handles
	 * a special case (comma-separated declarations separated by the preprocessor lines) where this doesn't happen
	 */
	if (
		ppst->last_token == comma
		&& ppst->p_l_follow <= 0
		&& settings.leave_comma
		&& !ppst->block_init
		&& break_comma
		&& s_com == e_com
	) {
		dump_line(true, &paren_target, pbreak_line);
		ppst->want_blank = false;
	}
}

static void handle_token_comment(BOOLEAN *force_nl, BOOLEAN *flushed_nl, BOOLEAN *pbreak_line)
{
	if (ppst->last_saw_nl && s_code != e_code) {
		*flushed_nl = false;
		dump_line(true, &paren_target, pbreak_line);
		ppst->want_blank = false;
		*force_nl = false;
	}
	print_comment(&paren_target, pbreak_line);
}

static void handle_token_attribute(void)
{
	char *t_ptr;
	if (s_code != e_code) {
		set_buf_break(bb_attribute, paren_target);
		*e_code++ = ' ';
	}

	for (t_ptr = token; t_ptr < token_end; ++t_ptr) {
		CHECK_SIZE(code)
		*e_code++ = *t_ptr;
	}

	ppst->in_decl = false;
	ppst->want_blank = true;
}

static void handle_the_token(
	const codes_ty type_code,
	BOOLEAN *scase,
	BOOLEAN *force_nl,
	BOOLEAN *sp_sw,
	BOOLEAN *flushed_nl,
	codes_ty *hd_type,
	int *dec_ind,
	BOOLEAN *last_token_ends_sp,
	exit_values_ty *file_exit_value,
	const bb_code_ty can_break,
	BOOLEAN *last_else,
	BOOLEAN is_procname_definition,
	BOOLEAN *pbreak_line
)
{
	switch (type_code) {
		case form_feed:
			handle_token_form_feed(pbreak_line);
			break;
		case newline:
			handle_token_newline(force_nl, pbreak_line);
			break;
		case lparen:
			handle_token_lparen(force_nl, sp_sw, dec_ind, pbreak_line);
			break;
		case rparen:
			handle_token_rparen(force_nl, sp_sw, hd_type, last_token_ends_sp, file_exit_value, pbreak_line);
			break;
		case unary_op:
			handle_token_unary_op(dec_ind, can_break);
			break;
		case binary_op:
			handle_token_binary_op(can_break);
			break;
		case postop:
			handle_token_postop();
			break;
		case question:
			handle_token_question(can_break);
			break;
		case casestmt:
			handle_token_casestmt(scase, file_exit_value);
			copy_id(type_code, force_nl, file_exit_value, can_break);
			break;
		case colon:
			handle_token_colon(scase, force_nl, dec_ind, can_break, pbreak_line);
			break;
		case doublecolon:
			handle_token_doublecolon();
			break;
		case semicolon:
			/* we are not in an initialization or structure declaration */
			handle_token_semicolon(scase, force_nl, sp_sw, dec_ind, last_token_ends_sp, file_exit_value);
			break;
		case lbrace:
			handle_token_lbrace(force_nl, dec_ind, file_exit_value, pbreak_line);
			break;
		case rbrace:
			handle_token_rbrace(force_nl, dec_ind, file_exit_value, pbreak_line);
			break;
		case swstmt:
			handle_token_swstmt(sp_sw, hd_type);
			copy_id(type_code, force_nl, file_exit_value, can_break);
			break;
		case sp_paren:
			handle_token_sp_paren(sp_sw, hd_type);
			copy_id(type_code, force_nl, file_exit_value, can_break);
			break;
		case sp_else:
		case sp_nparen:
			handle_token_nparen(force_nl, file_exit_value, last_else, pbreak_line);
			copy_id(type_code, force_nl, file_exit_value, can_break);
			break;
		case overloaded:
			/* handle C++ operator overloading like: Class foo::operator = ()" This is just like a decl, but we need to remember this token type. */
			handle_token_overloaded(can_break);
			break;
		case decl:
			handle_token_decl(dec_ind, file_exit_value, pbreak_line);
			copy_id(type_code, force_nl, file_exit_value, can_break);
			break;
		case cpp_operator:
			/* Handle C++ operator overloading.  See case overloaded above. */
		case ident:
			handle_token_ident(force_nl, sp_sw, hd_type, dec_ind, file_exit_value, can_break, is_procname_definition, pbreak_line);
			copy_id(type_code, force_nl, file_exit_value, can_break);
			break;
		case struct_delim:
			handle_token_struct_delim();
			break;
		case comma:
			handle_token_comma(force_nl, dec_ind, is_procname_definition);
			break;
		case preesc:
			handle_token_preesc(file_exit_value, pbreak_line);
			break;
		case comment:
		case cplus_comment:
			handle_token_comment(force_nl, flushed_nl, pbreak_line);
			break;
		case attribute:
			handle_token_attribute();
			break;
		default:
			abort();
	}
}

static void sw_buffer(void)
{
	ppst->search_brace = false;

	bp_save = buf_ptr;
	be_save = buf_end;

	buf_ptr = save_com.ptr;
	need_chars(&save_com, 1);
	buf_end = save_com.end;
	
	/* Make save_com empty */
	save_com.end = save_com.ptr;
}

static BOOLEAN search_brace(
	codes_ty *type_code,
	BOOLEAN *force_nl,
	BOOLEAN *flushed_nl,
	BOOLEAN *last_else,
	BOOLEAN *is_procname_definition,
	BOOLEAN *pbreak_line
)
{
	while (ppst->search_brace) {
		/* After scanning an if(), while (), etc., it might be necessary to keep track of the text between the if() and the start of the statement which follows. Use save_com to do so. */
		switch (*type_code) {
			case newline:
				++line_no;
				*flushed_nl = true;
				break;
			case form_feed:
				break;
			case lbrace:
				if (save_com.end == save_com.ptr) {
					ppst->search_brace = false;
					return true;
				}

				if (settings.btype_2 && ppst->last_token != rbrace) {
					if (
						ppst->last_token == sp_else
						&& save_com.end > &save_com.ptr[4]
						&& save_com.end[-2] == '*'
						&& save_com.end[-1] == '/'
						&& save_com.ptr[2] == '/'
						&& save_com.ptr[3] == '*'
					) {
						char *p;

						for (p = &save_com.ptr[4]; *p != '\n' && p < &save_com.end[-2]; ++p);

						if (*p != '\n')
							*save_com.end++ = EOL;
					}

					save_com.ptr[0] = '{';
					save_com.len = 1;
					save_com.column = current_column();
				} else {
					/*
					 * Put the brace at the end of the saved buffer, after a newline character.
					 * The newline char will cause a `dump_line' call, thus ensuring that the brace will go into the right column.
					 */
					*save_com.end++ = EOL;
					*save_com.end++ = '{';
					save_com.len += 2;
				}

				sw_buffer();
				break;
			case comment:
				/* save this comment in the `save_com' buffer, for possible re-insertion in the output stream later */
				if (!*flushed_nl || save_com.end != save_com.ptr) {
					need_chars(&save_com, 10);

					if (save_com.end == save_com.ptr) {
						/* if this is the first comment, we must set up the buffer */
						save_com.start_column = current_column();
						save_com.ptr[0] = save_com.ptr[1] = ' ';
						save_com.end = save_com.ptr + 2;
						save_com.len = 2;
						save_com.column = current_column();
					} else {
						/* add newline between comments */
						*save_com.end++ = EOL;
						*save_com.end++ = ' ';
						save_com.len += 2;
						--line_no;
					}

					*save_com.end++ = '/';
					*save_com.end++ = '*';

					for (;;) {
						/* Make sure there is room for this character and (while we're at it) the '/' we might add at the end of the loop. */
						need_chars(&save_com, 2);
						*save_com.end = *buf_ptr++;
						save_com.len++;

						if (buf_ptr >= buf_end) {
							fill_buffer();

							if (had_eof) {
								ERROR(_("EOF encountered in comment"), 0, 0);
								return poem_punt;
							}
						}

						if (*save_com.end++ == '*' && *buf_ptr == '/')
							break;
					}

					*save_com.end++ = '/';
					save_com.len++;

					/* get past / in buffer */
					if (++buf_ptr >= buf_end)
						fill_buffer();

					break;
				}
			/* Just some statement. */
			default:
				/* Some statement.  Unless it's special, arrange to break the line. */
				if (
					/* "if" statement */
					(
						*type_code == sp_paren
						&& *token == 'i'
						&& *last_else
					)
					/* "else" statement */
					|| (
						*type_code == sp_else
						&& e_code != s_code
						/* The "else" follows '}' */
						&& e_code[-1] == '}'
					)
				)
					*force_nl = false;
				else if (*flushed_nl)
					*force_nl = true;

				if (save_com.end == save_com.ptr) {
					ppst->search_brace = false;
					return true;
				}

				if (*force_nl) {
					*force_nl = false;
					/* this will be re-increased when the nl is read from the buffer */
					--line_no;
					need_chars(&save_com, 2);

					*save_com.end++ = EOL;
					save_com.len++;
					
					if (settings.verbose && !*flushed_nl)
						WARNING(_("Line broken"), 0, 0);
					
					*flushed_nl = false;
				}

				/* now copy this token we just found into the saved buffer. */
				*save_com.end++ = ' ';
				save_com.len++;
				buf_ptr = token;

				/*
				 * a total nightmare is created by trying to get the next token into this save buffer.
				 * Rather than that, I've just backed up the buffer pointer to point at `token'.
				 */
				ppst->procname = ppst->procname_end = ppst->classname = ppst->classname_end = "\0";

				/* Switch input buffers so that calls to lexi() will read from our save buffer. */
				sw_buffer();
				break;
		}

		if (*type_code != code_eof) {
			int just_saw_nl = *type_code == newline;

			*type_code = lexi();

			if (
				(*type_code == newline && just_saw_nl == true)
				|| (
					*type_code == comment
					&& ppst->last_saw_nl
					&& ppst->last_token != sp_else
				)
			) {
				dump_line(true, &paren_target, pbreak_line);
				*flushed_nl = true;
			}

			*is_procname_definition = ppst->procname[0] != '\0' && ppst->in_parameter_declaration;
		}

		if (
			*type_code == ident
			&& *flushed_nl
			&& !settings.procnames_start_line
			&& ppst->in_decl
			&& ppst->procname[0] != '\0'
		)
			*flushed_nl = false;
	}

	*last_else = 0;

	return true;
}

static exit_values_ty __poem(BOOLEAN *pbreak_line)
{
	codes_ty hd_type = code_eof;
	
	char *t_ptr = NULL;
	
	codes_ty type_code = start_token;
	
	exit_values_ty file_exit_value = total_success;
	
	int dec_ind = 0;

	BOOLEAN scase = false;

	/* Used when buffering up comments to remember that a newline was passed over */
	BOOLEAN flushed_nl;
	
	/* True when in the expression part of if(...), * while(...), etc. */
	BOOLEAN sp_sw = false;
	
	BOOLEAN force_nl = false;

	/*
	 * last_token_ends_sp: True if we have just encountered the end of an if (...), etc. (i.e. the ')' of the if (...) was the last token). The variable is
	 * set to 2 in the middle of the main token reading loop and is decremented at the beginning of the loop, so it will reach zero when the second token after the ')' is read.
	 */
	BOOLEAN last_token_ends_sp = false;

	BOOLEAN last_else = false;

	for (;;) {
		BOOLEAN is_procname_definition;
		bb_code_ty can_break = bb_none;

		if (type_code != newline)
			can_break = ppst->can_break;

		ppst->last_saw_nl = false;
		ppst->can_break = bb_none;

		type_code = lexi();

		/*
		 * If the last time around we output an identifier or a paren, then consider breaking the line here if it's too long.
		 * A similar check is performed at the end of the loop, after we've put the token on the line.
		 */
		if (
			settings.max_col > 0
			&& buf_break != NULL
			&& (
				(
					ppst->last_token == ident
					&& type_code != comma
					&& type_code != semicolon
					&& type_code != newline
					&& type_code != form_feed
					&& type_code != rparen
					&& type_code != struct_delim
				)
				|| (
					ppst->last_token == rparen
					&& type_code != comma
					&& type_code != rparen
				)
			)
			&& output_line_length() > settings.max_col
		)
			*pbreak_line = true;

		if (last_token_ends_sp > 0)
			last_token_ends_sp--;

		is_procname_definition = (ppst->procname[0] != '\0' && ppst->in_parameter_declaration) || ppst->classname[0] != '\0';

		/*
		 * The following code moves everything following an if (), while (), else, etc. Up to the
		 * start of the following stmt to a buffer. This allows proper handling of both kinds of brace placement.
		 */
		flushed_nl = false;

		if (!search_brace(&type_code, &force_nl, &flushed_nl, &last_else, &is_procname_definition, pbreak_line))
			return poem_punt;

		if (type_code == code_eof) {
			if (s_lab != e_lab || s_code != e_code || s_com != e_com)
				dump_line(true, &paren_target, pbreak_line);

			/* check for balanced braces */
			if (ppst->tos > 1) {
				ERROR(_("Unexpected end of file"), 0, 0);
				file_exit_value = poem_error;
			}

			if (settings.verbose) {
				printf(_("There were %d non-blank output lines and %d comments\n"), (int)out_lines, (int)com_lines);
				if (com_lines > 0 && code_lines > 0)
					printf(_("(Lines with comments)/(Lines with code): %6.3f\n"), (1.0 * com_lines) / code_lines);
			}

			flush_output();

			return file_exit_value;
		}

		if (
			type_code != comment
			&& type_code != cplus_comment
			&& type_code != newline
			&& type_code != preesc
			&& type_code != form_feed
		) {
			if (
				force_nl
				&& type_code != semicolon
				&& (
					type_code != lbrace
					|| (!ppst->in_decl && !settings.btype_2)
					|| (ppst->in_decl && !settings.braces_on_struct_decl_line)
					|| ppst->last_token == rbrace
				)
			) {
				if (settings.verbose && !flushed_nl)
					WARNING(_("Line broken 2"), 0, 0);

				flushed_nl = false;
				dump_line(true, &paren_target, pbreak_line);
				ppst->want_blank = false;
				force_nl = false;
			}
			
			/* Turn on flag which causes an extra level of indentation. this is turned off by a ; or } */
			ppst->in_stmt = true;

			if (s_com != e_com) {
				/*
				 * the code has an embedded comment in the line. Move it from the com buffer to the code buffer.
				 * Do not add a space before the comment if it is the first thing on the line.
				 */
				if (e_code != s_code) {
					set_buf_break(bb_embedded_comment_start, paren_target);
					*e_code++ = ' ';
					embedded_comment_on_line = 2;
				} else
					embedded_comment_on_line = 1;

				for (t_ptr = s_com; *t_ptr; ++t_ptr) {
					CHECK_SIZE(code)
					*e_code++ = *t_ptr;
				}

				set_buf_break(bb_embedded_comment_end, paren_target);

				*e_code++ = ' ';
				*e_code = '\0';
				
				ppst->want_blank = false;
				
				e_com = s_com;
			}
		} else if (
			type_code != comment
			&& type_code != cplus_comment
			&& !(settings.break_function_decl_args && ppst->last_token == comma)
			&& !(ppst->last_token == comma && !settings.leave_comma)
		)
			/*
			 * preserve force_nl thru a comment but cancel forced newline after newline, form feed, etc.
			 * however, don't cancel if last thing seen was comma-newline and -bc flag is on.
			 */
			force_nl = false;

		CHECK_SIZE(code)

		handle_the_token(
			type_code, &scase, &force_nl, &sp_sw, &flushed_nl, &hd_type, &dec_ind,
			&last_token_ends_sp, &file_exit_value, can_break, &last_else, is_procname_definition, pbreak_line
		);

		*e_code = '\0';

		if (
			type_code != comment
			&& type_code != cplus_comment
			&& type_code != newline
			&& type_code != preesc
			&& type_code != form_feed
		)
			ppst->last_token = type_code;

		/*
		 * Now that we've put the token on the line (in most cases), consider breaking the line because it's too long.
		 * Don't consider the cases of `unary_op', newlines, declaration types (int, etc.), if, while, for,
		 * identifiers (handled at the beginning of the loop), periods, or preprocessor commands.
		 */
		if (
			settings.max_col > 0
			&& buf_break != NULL
			&& (
				type_code == binary_op
				|| type_code == postop
				|| type_code == question
				|| (type_code == colon && (scase || squest <= 0))
				|| type_code == semicolon
				|| type_code == sp_nparen
				|| type_code == sp_else
				|| (type_code == ident && *token == '\"')
				|| type_code == struct_delim
				|| type_code == comma
			)
			&& output_line_length() > settings.max_col
		)
			*pbreak_line = true;
	}
}

static exit_values_ty poem(file_buffer_ty *this_file)
{
	BOOLEAN break_line = false;

	in_prog = in_prog_pos = this_file->data;
	in_prog_size = this_file->size;
	squest = false;
	n_real_blanklines = 0;
	postfix_blankline_requested = 0;

	clear_buf_break_list(&break_line);

	if (settings.decl_com_ind <= 0)
		settings.decl_com_ind = settings.ljust_decl ? (settings.com_ind <= 10 ? 2 : settings.com_ind - 8) : settings.com_ind;

	if (settings.continuation_indent == 0)
		settings.continuation_indent = settings.ind_size;

	if (settings.paren_indent == -1)
		settings.paren_indent = settings.continuation_indent;

	if (settings.case_brace_indent == -1)
		settings.case_brace_indent = settings.ind_size;

	fill_buffer();

	return __poem(&break_line);
}

static char *handle_profile(int argc, char *argv[])
{
	int i;
	char *profile_pathname = NULL;
	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-npro") || !strcmp(argv[i], "--ignore-profile") || !strcmp(argv[i], "+ignore-profile"))
			break;
	if (i >= argc)
		profile_pathname = set_profile();
	return profile_pathname;
}

static exit_values_ty process_args(int argc, char *argv[], BOOLEAN *using_stdin)
{
	int i;
	exit_values_ty exit_status = total_success;

	for (i = 1; i < argc; ++i) {
		if (*argv[i] != '-' && *argv[i] != '+') {
			/*
			 * fprintf(stderr, "argv[%d] may be a filename\n", i, argv[i]);
			 */
			if (settings.expect_output_file == true) {
				/*
				 * fprintf(stderr, "the last arg was -o, since you expect an output file.\n");
				 */
				if (out_name) {
					fprintf(stderr, _("poem: only one output file (2nd was %s)\n"), argv[i]);
					exit_status = invocation_error;
					break;
				}

				if (input_files > 1) {
					fprintf(stderr, _("poem: only one input file when output file is specified\n"));
					exit_status = invocation_error;
					break;
				}
				out_name = argv[i];
				settings.expect_output_file = false;
				continue;
			} else {
				/*
				 * fprintf("handle file %s here\n", argv[i]);
				 */
				if (*using_stdin) {
					fprintf(stderr, _("poem: can't have filenames when specifying standard input\n"));
					exit_status = invocation_error;
					break;
				}

				input_files++;

				if (input_files > 1) {	
					if (out_name) {
						fprintf(stderr, _("poem: only one input file when output file is specified\n"));
						exit_status = invocation_error;
						break;
					}

					if (settings.use_stdout) {
						fprintf(stderr, _("poem: only one input file when stdout is used\n"));
						exit_status = invocation_error;
						break;
					}

					if (input_files > max_input_files) {
						max_input_files *= 2;
						in_file_names = (char **)xrealloc((char *)in_file_names, max_input_files * sizeof(char *));
					}
				}
				in_file_names[input_files - 1] = argv[i];
			}
		} else {
			/*
			 * fprintf(stderr, "ordinary arg here.\n");
			 */
			if (!strcmp(argv[i], "-")) {
				if (input_files > 0) {
					fprintf(stderr, _("poem: can't have filenames when specifying standard input\n"));
					exit_status = invocation_error;
					break;
				}
				*using_stdin = true;
			} else
				i += set_option(argv[i], i < argc ? argv[i + 1] : 0, 1, _("command line"));
		}
	}
	return exit_status;
}

static exit_values_ty compose_multiple(void)
{
	exit_values_ty exit_status = total_success;
	int i;
	
	for (i = 0; input_files; i++, input_files--) {
		exit_values_ty status;
		struct stat file_stats;

		out_name = in_name = in_file_names[i];
		current_input = read_file(in_file_names[i], &file_stats);

		open_output(out_name, "r+");

		make_backup(current_input, &file_stats);

		reopen_output_trunc(out_name);

		reset_parser();

		status = poem(current_input);

		if (status > exit_status)
			exit_status = status;

		settings.preserve_mtime ? close_output(&file_stats, out_name) : close_output(NULL, out_name);
	}
	return exit_status;
}

static exit_values_ty compose_single(BOOLEAN using_stdin)
{
	exit_values_ty exit_status = total_success;
	struct stat file_stats;

	if (input_files == 0 || using_stdin) {
		input_files = 1;
		in_name = in_file_names[0] = "Standard input";
		current_input = read_stdin();
	} else {
		in_name = in_file_names[0];
		current_input = read_file(in_file_names[0], &file_stats);

		if (!out_name && !settings.use_stdout) {
			out_name = in_file_names[0];
			make_backup(current_input, &file_stats);
		}
	}

	settings.use_stdout || !out_name ? open_output(NULL, NULL) : open_output(out_name, "w");

	reset_parser();

	exit_status = poem(current_input);

	input_files > 0 && !using_stdin && settings.preserve_mtime ? close_output(&file_stats, out_name) : close_output(NULL, out_name);

	return exit_status;
}

static exit_values_ty compose(BOOLEAN using_stdin)
{
	return input_files > 1 ? compose_multiple() : compose_single(using_stdin);
}

int __main(int argc, char **argv)
{
	char *profile_pathname = NULL;
	BOOLEAN using_stdin = false;
	exit_values_ty exit_status = total_success;
#ifdef DEBUG
	if (debug)
		debug_init();
#endif
	init_parser();

	initialize_backups();

	input_files = 0;
	in_file_names = (char **)xmalloc(max_input_files * sizeof(char *));

	set_defaults();

	profile_pathname = handle_profile(argc, argv);

	exit_status = process_args(argc, argv, &using_stdin);
	
	if (exit_status == total_success) {
		if (settings.verbose && profile_pathname)
			fprintf(stderr, _("Read profile %s\n"), profile_pathname);
		/*
		 * free(profile_pathname);
		 */
		if (!exp_lc)
			settings.comment_max_col = settings.max_col;
		exit_status = compose(using_stdin);
	}

	return exit_status;
}

int main(int argc, char **argv)
{
	return __main(argc, argv);
}
