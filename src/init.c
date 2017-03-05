#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME: 
   Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP RVowpalWabbit_Rvw(SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"RVowpalWabbit_Rvw", (DL_FUNC) &RVowpalWabbit_Rvw, 1},
    {NULL, NULL, 0}
};

void R_init_RVowpalWabbit(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
