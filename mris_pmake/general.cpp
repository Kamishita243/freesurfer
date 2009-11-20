#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "general.h"
#include "pathconvert.h"

extern char  *Progname;
extern string  G_SELF;

void
lprintf(int lw, const char* format, ...) {
    char pch_buffer[65536];
    va_list vp_arg;
    va_start(vp_arg, format);
    vsnprintf(pch_buffer, 65536, format, vp_arg);
    va_end(vp_arg);
    printf("%*s", lw, pch_buffer);
    fflush(stdout);
}

void
colprintf(int lw, int rw, const char* pch_lstr, const char* format, ...) {
    char pch_buffer[65536];
    va_list vp_arg;
    va_start(vp_arg, format);
    vsnprintf(pch_buffer, 65536, format, vp_arg);
    va_end(vp_arg);
    printf("%*s", lw, pch_lstr);
    printf("%*s", rw, pch_buffer);
    fflush(stdout);
}


float
V3D_normalizedDirection_find(
  st_V3D&  V_A,
  st_V3D&  V_B,
  st_V3D*  pV_C) {
  //
  // PRECONDITIONS
  // o V_A, V_B, and pV_C must be valid vectors.
  //
  // POSTCONDITIONS
  // o return the normalized direction vector in pV_C.
  // o return the magnitude in function argument.
  //  - if magnitude is zero, pV_C will be unchanged from its
  //    its input value
  //
  // HISTORY
  //  14 October 2004
  // Initial design and coding
  //

  float f_distance = 0.;

  f_distance   = sqrt((V_B.f_x - V_A.f_x)*(V_B.f_x - V_A.f_x) +
                      (V_B.f_y - V_A.f_y)*(V_B.f_y - V_A.f_y) +
                      (V_B.f_z - V_A.f_z)*(V_B.f_z - V_A.f_z));
  if (!f_distance)
    return 0;

  pV_C->f_x = (V_B.f_x - V_A.f_x) / f_distance;
  pV_C->f_y = (V_B.f_y - V_A.f_y) / f_distance;
  pV_C->f_z = (V_B.f_z - V_A.f_z) / f_distance;

  return f_distance;
}

float
V3D_dot(
  st_V3D&  V_A,
  st_V3D&  V_B) {
  //
  // PRECONDITIONS
  // o V_A and V_B must be valid vectors.
  //
  // POSTCONDITIONS
  // o return the dot product.
  //
  // HISTORY
  //  14 October 2004
  // Initial design and coding
  //

  return( V_A.f_x * V_B.f_x +
          V_A.f_y * V_B.f_y +
          V_A.f_z * V_B.f_z);
}

float
V3D_distance(
  st_V3D&  V_A,
  st_V3D&  V_B) {
  //
  // PRECONDITIONS
  // o V_A and V_B must be valid vectors.
  //
  // POSTCONDITIONS
  // o return the distance between the vectors.
  //
  // HISTORY
  //  09 March 2005
  // Initial design and coding
  //

  return( sqrt(
            (V_A.f_x - V_B.f_x)*(V_A.f_x - V_B.f_x) +
            (V_A.f_y - V_B.f_y)*(V_A.f_y - V_B.f_y) +
            (V_A.f_z - V_B.f_z)*(V_A.f_z - V_B.f_z)
          )
        );
}

bool
str_leadingWSremove(
  string&  astr_input
) {
  //
  // PRECONDITIONS
  // o An input string that is (possibly) prepended with white space
  //  i.e. tabs and / or spaces
  //
  // POSTCONDITIONS
  // o Leading WS is stripped from input string.
  //

  char const* str_sep  = " \t";
  int  pos  = 0;
  bool ret  = false;

  // find leading whitespace. Loop until whitespace pos != 0
  pos  = astr_input.find_first_of(str_sep);
  while (!pos) {
    astr_input.erase(0,1);
    ret = true;
    pos = astr_input.find_first_of(str_sep);
  }
  return ret;
}

bool
relDirSpec_test(
  string&  astr_dirSpec
) {
  //
  // ARGS
  //  astr_dirSpec  in  input string containing a
  //      (possibly) relative directory
  //      specification
  //
  // DESCRIPTION
  // Checks the directory (or file) specified by <astr_dirSpec>
  // and returns a TRUE if it is a relative specification, otherwise
  // returns a FALSE.
  //
  //  A relative specification is basically any string that does *not*
  // start with a "/" character.
  //
  // PRECONDITIONS
  //  o astr_dirSpec should specify a legal directory (or file).
  //
  // POSTCONDITIONS
  // o TRUE if <astr_dirSpec> is a relative directory/file, otherwise
  //   FALSE.
  //

  bool b_ret = false;
  char ch_first;

  ch_first = astr_dirSpec[0];
  b_ret = (ch_first == '/') ? false : true;
  return b_ret;
}


bool
str_rel2absDirSpec_change(
  string&  astr_rel,
  string&  astr_abs
) {
  //
  // ARGS
  //  astr_rel  in  input string containing a
  //      (possibly) relative directory
  //      specification
  //  astr_abs  out  output string containing the
  //      input as an absolute directory
  //      specification.
  //
  // DESCRIPTION
  //  Convert the (possibly) relative directory specification in
  // <astr_rel> to an absolute directory specification that is
  // stored in <astr_abs>.
  //
  // The actual conversion is performed by a lower C function,
  // 'rel2abs(...)'. This routine merely wraps some C++ string
  // machinery about the C function call.
  //
  // PRECONDITIONS
  //  o astr_rel should contain a legal directory specification.
  //
  // POSTCONDITIONS
  // o See 'rel2abs' for postcondition behaviour.
  //

  char pch_result[MAXPATHLEN];
  char pch_cwd[MAXPATHLEN];
  bool b_ret = false; // No conversion occurred.

  astr_abs = astr_rel;
  if (relDirSpec_test(astr_rel)) {
    if (!getcwd(pch_cwd, MAXPATHLEN))
      error_exit("converting relative to absolute dir spec in <str_rel2absDirSpec_change>",
                 "I cannot seem to read the current directory.", 1);
    rel2abs(astr_rel.c_str(), pch_cwd, pch_result, MAXPATHLEN);
    astr_abs = pch_result;
    b_ret = true;
  }
  return b_ret;
}


int
str_3parse(
  string&  astr_input,
  string&  astr_first,
  string&  astr_second,
  string&  astr_third
) {
  char const* str_sep  = " \t";
  //string str_sep  = " \t";
  unsigned  pos  = 0;

  // Remove any leading whitespace (WS)
  str_leadingWSremove(astr_input);

  // Parse for <first>
  pos  = astr_input.find_first_of(str_sep);
  if (pos == (unsigned)string::npos)
    return 0;
  astr_first = astr_input.substr(0, pos);
  astr_input.erase(0, astr_first.length()+1);

  // Parse for <second>
  str_leadingWSremove(astr_input); // Remove any leading WS
  pos  = astr_input.find_first_of(str_sep);
  if (pos == (unsigned)string::npos) {
    // No trailing spaces remaining
    if (!astr_input.length()) {
      astr_second = "";
      astr_third = "";
      return 1;
    } else {
      astr_second = astr_input;
      astr_input.erase(0, astr_second.length());
    }
  } else {
    // trailing space found
    astr_second = astr_input.substr(0, pos);
    astr_input.erase(0, astr_second.length()+1);
  }

  // Parse for [<third>]
  str_leadingWSremove(astr_input); // Remove any leading WS
  pos  = astr_input.find_first_of(str_sep);
  if (pos == (unsigned)string::npos) {
    // No trailing spaces remaining
    if (!astr_input.length()) {
      astr_third = "";
      return 2;
    } else {
      astr_third = astr_input;
      astr_input.erase(0, astr_third.length());
    }
  } else {
    // trailing space found
    astr_third = astr_input.substr(0, pos);
    astr_input.erase(0, astr_third.length()+1);
  }
  return 3;
}

void warn(
  string              str_action,
  string              str_errorMsg,
  int                 errorCode) {

  //
  // ARGS
  //  str_action      in              action that failed
  //  str_errorMsg    in              error message to dump to stderr
  //  errorCode       in              code to echo to stdout
  //
  // DESC
  //  Dumps a simple error message and then warns with
  //  errorCode.
  //

  cerr << endl << G_SELF;
  cerr << endl << "\tWarning, Will Robinson!";
  cerr << endl << "\tWhile I was "    << str_action;
  cerr << endl << "\t"                << str_errorMsg;
  cerr << endl;
  cerr << endl << "\tWarning code " << errorCode;
  cerr << endl;
}

void
error_exit(
  string              str_action,
  string              str_errorMsg,
  int                 errorCode) {

  //
  // ARGS
  //  str_action      in              action that failed
  //  str_errorMsg    in              error message to dump to stderr
  //  errorCode       in              code to return to OS
  //
  // DESC
  //  Dumps a simple error message and then exits to syste with
  //  errorCode.
  //

  cerr << endl << G_SELF;
  cerr << endl << "\tDanger, Will Robinson!";
  cerr << endl << "\tWhile I was "    << str_action;
  cerr << endl << "\t"                << str_errorMsg;
  cerr << endl;
  cerr << endl << "\tExiting to system with code " << errorCode;
  cerr << endl;
  exit(errorCode);
}

void command_line_error(char *fmt, ...) {

  va_list ap;

  va_start(ap, fmt);

  fprintf(stderr, "%s: ", Progname);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  fprintf(stderr, "run %s with no arguments for usage\n", Progname);

  va_end(ap);

  return;

} /* end command_line_error() */

/* eof */
