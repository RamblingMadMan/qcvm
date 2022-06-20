#ifndef QCVM_AMATH_H
#define QCVM_AMATH_H 1

/**
 * @defgroup AMathC Arbitrary precision math
 * @{
 */

#include "common.h"

#include "gmp.h"
#include "mpfr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Handle to an arbitrary precision integer
*/
typedef struct QC_Aint{
	mpz_t _mpz;
} QC_Aint;

/**
 * @brief Handle to an arbitrary precision floating point number
 */
typedef struct QC_Afloat{
	mpfr_t _mpfr;
} QC_Afloat;

/**
 * @brief Initialize an arbitrary precision integer
 * @note \ref qcClearAint must be called on \p aint when it is no longer required
 * @param aint handle to initialize
 * @see qcClearAint
 */
static inline void qcInitAint(QC_Aint *aint){
	QCVM_ASSERT(aint);
	mpz_init(aint->_mpz);
}

static inline void qcInitAintStr(QC_Aint *aint, const char *str, int base QCVM_DEFAULT_VALUE(10)){
	QCVM_ASSERT(aint);
	mpz_init_set_str(aint->_mpz, str, base);
}

/**
 * @brief Initialize an arbitrary precision floating-point number
 * @note \ref qcClearAfloat must be called on \p afloat when it is no longer required
 * @param afloat handle to initialize
 * @see qcClearAfloat
 */
static inline void qcInitAfloat(QC_Afloat *afloat){
	QCVM_ASSERT(afloat);
	mpfr_init(afloat->_mpfr);
}

static inline void qcInitAfloatStr(QC_Afloat *afloat, const char *str, int base QCVM_DEFAULT_VALUE(10)){
	QCVM_ASSERT(afloat);
	mpfr_init_set_str(afloat->_mpfr, str, 10, MPFR_RNDN);
}

/**
 * @brief Clear a previously initialized arbitrary precision integer
 * @param aint handle to clear
 * @see qcInitAint
 */
static inline void qcClearAint(QC_Aint *aint){
	QCVM_ASSERT(aint);
	mpz_clear(aint->_mpz);
}

/**
 * @brief Clear a previously initialized arbitrary precision floating-point number
 * @param afloat handle to clear
 * @see qcInitAfloat
 */
static inline void qcClearAfloat(QC_Afloat *afloat){
	QCVM_ASSERT(afloat);
	mpfr_clear(afloat->_mpfr);
}

static inline void qcSetAint(QC_Aint *ret, const QC_Aint *other){
	mpz_set(ret->_mpz, other->_mpz);
}

static inline void qcSetAintStr(QC_Aint *ret, const char *str, int base QCVM_DEFAULT_VALUE(10)){
	mpz_set_str(ret->_mpz, str, base);
}

static inline void qcAddAint(QC_Aint *ret, const QC_Aint *lhs, const QC_Aint *rhs){
	mpz_add(ret->_mpz, lhs->_mpz, rhs->_mpz);
}

static inline void qcSubAint(QC_Aint *ret, const QC_Aint *lhs, const QC_Aint *rhs){
	mpz_sub(ret->_mpz, lhs->_mpz, rhs->_mpz);
}

static inline void qcMulAint(QC_Aint *ret, const QC_Aint *lhs, const QC_Aint *rhs){
	mpz_mul(ret->_mpz, lhs->_mpz, rhs->_mpz);
}

static inline void qcDivAint(QC_Aint *ret, const QC_Aint *lhs, const QC_Aint *rhs){
	mpz_div(ret->_mpz, lhs->_mpz, rhs->_mpz);
}

static inline void qcSetAfloat(QC_Afloat *ret, const QC_Afloat *other){
	mpfr_set(ret->_mpfr, other->_mpfr, MPFR_RNDN);
}

static inline void qcSetAfloatStr(QC_Afloat *ret, const char *str, int base QCVM_DEFAULT_VALUE(10)){
	mpfr_set_str(ret->_mpfr, str, base, MPFR_RNDN);
}

//! Add arbitrary precision floating-point numbers
static inline void qcAddAfloat(QC_Afloat *ret, const QC_Afloat *lhs, const QC_Afloat *rhs){
	mpfr_add(ret->_mpfr, lhs->_mpfr, rhs->_mpfr, MPFR_RNDN);
}

//! Subtract arbitrary precision floating-point numbers
static inline void qcSubAfloat(QC_Afloat *ret, const QC_Afloat *lhs, const QC_Afloat *rhs){
	mpfr_sub(ret->_mpfr, lhs->_mpfr, rhs->_mpfr, MPFR_RNDN);
}

//! Multiply arbitrary precision floating-point numbers
static inline void qcMulAfloat(QC_Afloat *ret, const QC_Afloat *lhs, const QC_Afloat *rhs){
	mpfr_mul(ret->_mpfr, lhs->_mpfr, rhs->_mpfr, MPFR_RNDN);
}

//! Divide arbitrary precision floating-point numbers
static inline void qcDivAfloat(QC_Afloat *ret, const QC_Afloat *lhs, const QC_Afloat *rhs){
	mpfr_div(ret->_mpfr, lhs->_mpfr, rhs->_mpfr, MPFR_RNDN);
}

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // !QCVM_AMATH_H
