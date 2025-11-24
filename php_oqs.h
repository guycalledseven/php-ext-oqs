#ifndef PHP_OQS_H
#define PHP_OQS_H

#include "php.h"
#include "zend_exceptions.h"
#include <oqs/oqs.h>

extern zend_module_entry oqs_module_entry;
#define phpext_oqs_ptr &oqs_module_entry

#define PHP_OQS_NS "Oqs"

#define PHP_OQS_VERSION "0.1.0"
#define PHP_OQS_LIB_VERSION "liboqs 0.11.0"
#ifndef PHP_OQS_LIB_COMMIT
#define PHP_OQS_LIB_COMMIT "unknown"
#endif

typedef struct {
  OQS_KEM *kem;
  zend_object std;
} php_oqs_kem_obj;

typedef struct {
  OQS_SIG *sig;
  zend_object std;
} php_oqs_sig_obj;

extern zend_class_entry *oqs_ce_kem;
extern zend_class_entry *oqs_ce_sig;
extern zend_class_entry *oqs_ce_exc; /* Oqs\Exception */

static inline php_oqs_kem_obj *php_oqs_kem_fetch(zend_object *obj) {
  return (php_oqs_kem_obj *)((char *)(obj)-XtOffsetOf(php_oqs_kem_obj, std));
}
static inline php_oqs_sig_obj *php_oqs_sig_fetch(zend_object *obj) {
  return (php_oqs_sig_obj *)((char *)(obj)-XtOffsetOf(php_oqs_sig_obj, std));
}

static inline void memwipe(void *p, size_t n) {
  if (!p || !n)
    return;
  volatile unsigned char *v = (volatile unsigned char *)p;
  while (n--)
    *v++ = 0;
}

#endif
