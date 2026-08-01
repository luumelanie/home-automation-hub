/* adc_test.h - ADC helper for LPC2478 (AD0.2 on P0.25)
 *
 * Usage:
 *   1) Call setupADC() once at startup.
 *   2) Call readADC() whenever you need a sample (0..1023).
 *   3) Convert to mV with ADC_CODE_TO_MV(code) assuming Vref ˜ 3.3V.
 */

#ifndef ADC_TEST_H
#define ADC_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialise ADC peripheral and pin P0.25 -> AD0.2 */
void setupADC(void);

/* Take one blocking sample from AD0.2, returns 10-bit code (0..1023). */
int  readADC(void);

/* Convenience macros */
#define ADC_VREF_MV    3300      /* QVGA Base Board Vref = 3.3V */
#define ADC_MAX_CODE   1023      /* 10-bit ADC */
#define ADC_CODE_TO_MV(code)  ( (int)(((code) * ADC_VREF_MV) / ADC_MAX_CODE) )

#ifdef __cplusplus
}
#endif

#endif /* ADC_TEST_H */
