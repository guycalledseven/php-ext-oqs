#include "php_oqs.h"

/* Oqs\Sig::__construct(string $algorithm) */
PHP_METHOD(Sig, __construct) {
  char *alg;
  size_t alg_len;
  ZEND_PARSE_PARAMETERS_START(1, 1)
  Z_PARAM_STRING(alg, alg_len)
  ZEND_PARSE_PARAMETERS_END();

  php_oqs_sig_obj *o = php_oqs_sig_fetch(Z_OBJ_P(getThis()));
  o->sig = OQS_SIG_new(alg);
  if (!o->sig) {
    zend_throw_exception(oqs_ce_exc, "SIG algorithm not enabled", 0);
    return;
  }
}

/* [pk, sk] */
PHP_METHOD(Sig, keypair) {
  php_oqs_sig_obj *o = php_oqs_sig_fetch(Z_OBJ_P(getThis()));
  if (!o->sig) {
    zend_throw_exception(oqs_ce_exc, "SIG not initialized", 0);
    return;
  }

  zend_string *pk = zend_string_alloc(o->sig->length_public_key, 0);
  zend_string *sk = zend_string_alloc(o->sig->length_secret_key, 0);
  if (OQS_SIG_keypair(o->sig, (uint8_t *)ZSTR_VAL(pk),
                      (uint8_t *)ZSTR_VAL(sk)) != OQS_SUCCESS) {
    zend_string_release(pk);
    zend_string_release(sk);
    zend_throw_exception(oqs_ce_exc, "sig keypair failed", 0);
    return;
  }
  ZSTR_VAL(pk)[ZSTR_LEN(pk)] = 0;
  ZSTR_VAL(sk)[ZSTR_LEN(sk)] = 0;

  array_init_size(return_value, 2);
  add_next_index_str(return_value, pk);
  add_next_index_str(return_value, sk);
}

/* string signature */
PHP_METHOD(Sig, sign) {
  php_oqs_sig_obj *o = php_oqs_sig_fetch(Z_OBJ_P(getThis()));
  zend_string *msg, *sk;
  ZEND_PARSE_PARAMETERS_START(2, 2)
  Z_PARAM_STR(msg)
  Z_PARAM_STR(sk)
  ZEND_PARSE_PARAMETERS_END();

  if (!o->sig) {
    zend_throw_exception(oqs_ce_exc, "SIG not initialized", 0);
    return;
  }

  const size_t max_len = o->sig->length_signature;
  zend_string *sig = zend_string_alloc(max_len, 0); // non-persistent
  size_t siglen = 0;

  if (OQS_SIG_sign(o->sig, (uint8_t *)ZSTR_VAL(sig), &siglen,
                   (const uint8_t *)ZSTR_VAL(msg), ZSTR_LEN(msg),
                   (const uint8_t *)ZSTR_VAL(sk)) != OQS_SUCCESS) {
    zend_string_release(sig);
    zend_throw_exception(oqs_ce_exc, "sign failed", 0);
    return;
  }

  if (siglen > max_len) { // defensive
    zend_string_release(sig);
    zend_throw_exception(oqs_ce_exc, "signature overflow", 0);
    return;
  }

  sig = zend_string_truncate(sig, siglen, 0); // returns new/updated pointer
  ZSTR_VAL(sig)[ZSTR_LEN(sig)] = 0;           // optional NUL
  RETURN_STR(sig);
}

/* bool */
PHP_METHOD(Sig, verify) {
  php_oqs_sig_obj *o = php_oqs_sig_fetch(Z_OBJ_P(getThis()));
  zend_string *msg, *sig, *pk;
  ZEND_PARSE_PARAMETERS_START(3, 3)
  Z_PARAM_STR(msg)
  Z_PARAM_STR(sig)
  Z_PARAM_STR(pk)
  ZEND_PARSE_PARAMETERS_END();

  if (!o->sig) {
    zend_throw_exception(oqs_ce_exc, "SIG not initialized", 0);
    return;
  }

  int rc = OQS_SIG_verify(o->sig, (const uint8_t *)ZSTR_VAL(msg), ZSTR_LEN(msg),
                          (const uint8_t *)ZSTR_VAL(sig), ZSTR_LEN(sig),
                          (const uint8_t *)ZSTR_VAL(pk));
  RETURN_BOOL(rc == OQS_SUCCESS);
}

/* array details(): metadata */
PHP_METHOD(Sig, details) {
  php_oqs_sig_obj *o = php_oqs_sig_fetch(Z_OBJ_P(getThis()));
  if (!o->sig) {
    RETURN_NULL();
  }

  array_init(return_value);
  add_assoc_string(return_value, "name", (char *)o->sig->method_name);
  add_assoc_string(return_value, "version", (char *)o->sig->alg_version);
  add_assoc_long(return_value, "claimed_nist_level",
                 (zend_long)o->sig->claimed_nist_level);
  add_assoc_bool(return_value, "euf_cma", o->sig->euf_cma);
  add_assoc_bool(return_value, "suf_cma", o->sig->suf_cma);
  add_assoc_bool(return_value, "sig_with_ctx_support",
                 o->sig->sig_with_ctx_support);
  add_assoc_long(return_value, "length_public_key",
                 (zend_long)o->sig->length_public_key);
  add_assoc_long(return_value, "length_secret_key",
                 (zend_long)o->sig->length_secret_key);
  add_assoc_long(return_value, "length_signature",
                 (zend_long)o->sig->length_signature);
}

/* static array algorithms(): enabled SIG names */
PHP_METHOD(Sig, algorithms) {
  int n = OQS_SIG_alg_count();
  array_init(return_value);
  for (int i = 0; i < n; i++) {
    const char *name = OQS_SIG_alg_identifier(i);
    if (name && OQS_SIG_alg_is_enabled(name)) {
      add_next_index_string(return_value, name);
    }
  }
}
