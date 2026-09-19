#ifndef PHP_OQS_H
#define PHP_OQS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"
#include <oqs/oqs.h>

extern zend_module_entry oqs_module_entry;
#define phpext_oqs_ptr &oqs_module_entry

#define PHP_OQS_NS "Oqs"

#define PHP_OQS_VERSION "0.1.0"
/* version of the liboqs headers this extension was compiled against */
#define PHP_OQS_LIB_VERSION "liboqs " OQS_VERSION_TEXT
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

/* liboqs reads fixed-size keys/ciphertexts without a length argument, so every
 * such input must be length-checked before it is passed down.
 * Throws Oqs\Exception and returns 0 on mismatch. */
static inline int php_oqs_check_len(const char *what, size_t got,
                                    size_t expected) {
  if (got != expected) {
    zend_throw_exception_ex(oqs_ce_exc, 0,
                            "Invalid %s length: expected %zu bytes, got %zu",
                            what, expected, got);
    return 0;
  }
  return 1;
}

#endif
