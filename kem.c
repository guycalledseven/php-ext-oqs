#include "php_oqs.h"

/* Oqs\Kem::__construct(string $algorithm) */
PHP_METHOD(Kem, __construct)
{
    char *alg; size_t alg_len;
    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STRING(alg, alg_len)
    ZEND_PARSE_PARAMETERS_END();

    php_oqs_kem_obj *o = php_oqs_kem_fetch(Z_OBJ_P(getThis()));
    o->kem = OQS_KEM_new(alg);
    if (!o->kem) {
        zend_throw_exception(oqs_ce_exc, "KEM algorithm not enabled", 0);
        return;
    }
}

/* array sizes(): ['pk'=>..,'sk'=>..,'ct'=>..,'ss'=>..] */
PHP_METHOD(Kem, sizes)
{
    php_oqs_kem_obj *o = php_oqs_kem_fetch(Z_OBJ_P(getThis()));
    if (!o->kem) { RETURN_NULL(); }
    array_init(return_value);
    add_assoc_long(return_value, "pk", (zend_long)o->kem->length_public_key);
    add_assoc_long(return_value, "sk", (zend_long)o->kem->length_secret_key);
    add_assoc_long(return_value, "ct", (zend_long)o->kem->length_ciphertext);
    add_assoc_long(return_value, "ss", (zend_long)o->kem->length_shared_secret);
}

/* [pk, sk] */
PHP_METHOD(Kem, keypair)
{
    php_oqs_kem_obj *o = php_oqs_kem_fetch(Z_OBJ_P(getThis()));
    if (!o->kem) { zend_throw_exception(oqs_ce_exc, "KEM not initialized", 0); return; }

    zend_string *pk = zend_string_alloc(o->kem->length_public_key, 0);
    zend_string *sk = zend_string_alloc(o->kem->length_secret_key, 0);

    if (OQS_KEM_keypair(o->kem, (uint8_t*)ZSTR_VAL(pk), (uint8_t*)ZSTR_VAL(sk)) != OQS_SUCCESS) {
        zend_string_release(pk); zend_string_release(sk);
        zend_throw_exception(oqs_ce_exc, "KEM keypair failed", 0);
        return;
    }
    ZSTR_VAL(pk)[ZSTR_LEN(pk)] = 0;
    ZSTR_VAL(sk)[ZSTR_LEN(sk)] = 0;

    array_init_size(return_value, 2);
    add_next_index_str(return_value, pk);
    add_next_index_str(return_value, sk);
}

/* [ct, ss] */
PHP_METHOD(Kem, encap)
{
    php_oqs_kem_obj *o = php_oqs_kem_fetch(Z_OBJ_P(getThis()));
    zend_string *pk;
    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STR(pk)
    ZEND_PARSE_PARAMETERS_END();

    if (!o->kem) { zend_throw_exception(oqs_ce_exc, "KEM not initialized", 0); return; }

    zend_string *ct = zend_string_alloc(o->kem->length_ciphertext, 0);
    zend_string *ss = zend_string_alloc(o->kem->length_shared_secret, 0);

    if (OQS_KEM_encaps(o->kem,
            (uint8_t*)ZSTR_VAL(ct), (uint8_t*)ZSTR_VAL(ss),
            (const uint8_t*)ZSTR_VAL(pk)) != OQS_SUCCESS) {
        memwipe(ZSTR_VAL(ct), ZSTR_LEN(ct));
        memwipe(ZSTR_VAL(ss), ZSTR_LEN(ss));
        zend_string_release(ct); zend_string_release(ss);
        zend_throw_exception(oqs_ce_exc, "KEM encaps failed", 0);
        return;
    }
    ZSTR_VAL(ct)[ZSTR_LEN(ct)] = 0;
    ZSTR_VAL(ss)[ZSTR_LEN(ss)] = 0;

    array_init_size(return_value, 2);
    add_next_index_str(return_value, ct);
    add_next_index_str(return_value, ss);
}

/* string ss */
PHP_METHOD(Kem, decap)
{
    php_oqs_kem_obj *o = php_oqs_kem_fetch(Z_OBJ_P(getThis()));
    zend_string *ct, *sk;
    ZEND_PARSE_PARAMETERS_START(2,2)
        Z_PARAM_STR(ct)
        Z_PARAM_STR(sk)
    ZEND_PARSE_PARAMETERS_END();

    if (!o->kem) { zend_throw_exception(oqs_ce_exc, "KEM not initialized", 0); return; }

    zend_string *ss = zend_string_alloc(o->kem->length_shared_secret, 0);
    if (OQS_KEM_decaps(o->kem,
            (uint8_t*)ZSTR_VAL(ss),
            (const uint8_t*)ZSTR_VAL(ct),
            (const uint8_t*)ZSTR_VAL(sk)) != OQS_SUCCESS) {
        memwipe(ZSTR_VAL(ss), ZSTR_LEN(ss));
        zend_string_release(ss);
        zend_throw_exception(oqs_ce_exc, "KEM decaps failed", 0);
        return;
    }
    ZSTR_VAL(ss)[ZSTR_LEN(ss)] = 0;
    RETURN_STR(ss);
}

/* array details(): metadata */
PHP_METHOD(Kem, details)
{
    php_oqs_kem_obj *o = php_oqs_kem_fetch(Z_OBJ_P(getThis()));
    if (!o->kem) { RETURN_NULL(); }

    array_init(return_value);
    add_assoc_string(return_value, "name", (char*)o->kem->method_name);
    add_assoc_string(return_value, "version", (char*)o->kem->alg_version);
    add_assoc_long(return_value, "claimed_nist_level", (zend_long)o->kem->claimed_nist_level);
    add_assoc_bool(return_value, "ind_cca", o->kem->ind_cca);
    add_assoc_long(return_value, "length_public_key", (zend_long)o->kem->length_public_key);
    add_assoc_long(return_value, "length_secret_key", (zend_long)o->kem->length_secret_key);
    add_assoc_long(return_value, "length_ciphertext", (zend_long)o->kem->length_ciphertext);
    add_assoc_long(return_value, "length_shared_secret", (zend_long)o->kem->length_shared_secret);
}

/* static array algorithms(): enabled KEM names */
PHP_METHOD(Kem, algorithms)
{
    int n = OQS_KEM_alg_count();
    array_init(return_value);
    for (int i = 0; i < n; i++) {
        const char *name = OQS_KEM_alg_identifier(i);
        if (name && OQS_KEM_alg_is_enabled(name)) {
            add_next_index_string(return_value, name);
        }
    }
}
