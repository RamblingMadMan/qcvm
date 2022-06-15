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

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // !QCVM_AMATH_H
