
#include "php.h"
#include "php_oqs.h"
#include <ctype.h> // for algo constants

#include "Zend/zend_smart_string.h"
#include "oqs/oqs.h"
#include "oqs/rand.h"

/* Manual declarations to avoid header issues */
void php_info_print_table_start(void);
void php_info_print_table_header(int num_cols, ...);
void php_info_print_table_row(int num_cols, ...);
void php_info_print_table_end(void);

zend_class_entry *oqs_ce_kem = NULL;
zend_class_entry *oqs_ce_sig = NULL;
zend_class_entry *oqs_ce_exc = NULL; // definition for a Oqs\Exception

static zend_object_handlers oqs_kem_handlers;
static zend_object_handlers oqs_sig_handlers;

/* KEM object */
static zend_object *oqs_kem_create(zend_class_entry *ce) {
  php_oqs_kem_obj *o = zend_object_alloc(sizeof(*o), ce);
  o->kem = NULL;
  zend_object_std_init(&o->std, ce);
  object_properties_init(&o->std, ce);
  o->std.handlers = &oqs_kem_handlers;
  return &o->std;
}
static void oqs_kem_free(zend_object *obj) {
  php_oqs_kem_obj *o = php_oqs_kem_fetch(obj);
  if (o->kem) {
    OQS_KEM_free(o->kem);
    o->kem = NULL;
  }
  zend_object_std_dtor(&o->std);
}

/* SIG object */
static zend_object *oqs_sig_create(zend_class_entry *ce) {
  php_oqs_sig_obj *o = zend_object_alloc(sizeof(*o), ce);
  o->sig = NULL;
  zend_object_std_init(&o->std, ce);
  object_properties_init(&o->std, ce);
  o->std.handlers = &oqs_sig_handlers;
  return &o->std;
}
static void oqs_sig_free(zend_object *obj) {
  php_oqs_sig_obj *o = php_oqs_sig_fetch(obj);
  if (o->sig) {
    OQS_SIG_free(o->sig);
    o->sig = NULL;
  }
  zend_object_std_dtor(&o->std);
}

/* arginfo */

/* --- Oqs\Kem --- */
ZEND_BEGIN_ARG_INFO_EX(arginfo_Oqs_Kem___construct, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, algorithm, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Oqs_Kem_sizes, 0, 0, IS_ARRAY,
                                        0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Oqs_Kem_keypair, 0, 0, IS_ARRAY,
                                        0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Oqs_Kem_encap, 0, 1, IS_ARRAY,
                                        0)
ZEND_ARG_TYPE_INFO(0, publicKey, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Oqs_Kem_decap, 0, 2, IS_STRING,
                                        0)
ZEND_ARG_TYPE_INFO(0, ciphertext, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, secretKey, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Oqs_Kem_details, 0, 0, IS_ARRAY,
                                        0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Oqs_Kem_algorithms, 0, 0,
                                        IS_ARRAY, 0)
ZEND_END_ARG_INFO()

/* --- Oqs\Sig --- */

ZEND_BEGIN_ARG_INFO_EX(arginfo_Oqs_Sig___construct, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, algorithm, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Oqs_Sig_keypair, 0, 0, IS_ARRAY,
                                        0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Oqs_Sig_sign, 0, 2, IS_STRING,
                                        0)
ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, secretKey, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Oqs_Sig_verify, 0, 3, _IS_BOOL,
                                        0)
ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, signature, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, publicKey, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Oqs_Sig_details, 0, 0, IS_ARRAY,
                                        0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Oqs_Sig_algorithms, 0, 0,
                                        IS_ARRAY, 0)
ZEND_END_ARG_INFO()

/* method decls (implemented in kem.c / sig.c) */
PHP_METHOD(Kem, __construct);
PHP_METHOD(Kem, sizes);
PHP_METHOD(Kem, keypair);
PHP_METHOD(Kem, encap);
PHP_METHOD(Kem, decap);
PHP_METHOD(Kem, details);
PHP_METHOD(Kem, algorithms);

PHP_METHOD(Sig, __construct);
PHP_METHOD(Sig, keypair);
PHP_METHOD(Sig, sign);
PHP_METHOD(Sig, verify);
PHP_METHOD(Sig, details);
PHP_METHOD(Sig, algorithms);

/* function tables */
static const zend_function_entry kem_methods[] = {
    PHP_ME(Kem, __construct, arginfo_Oqs_Kem___construct,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(Kem, sizes, arginfo_Oqs_Kem_sizes, ZEND_ACC_PUBLIC) PHP_ME(
            Kem, keypair, arginfo_Oqs_Kem_keypair, ZEND_ACC_PUBLIC)
            PHP_ME(Kem, encap, arginfo_Oqs_Kem_encap, ZEND_ACC_PUBLIC) PHP_ME(
                Kem, decap, arginfo_Oqs_Kem_decap, ZEND_ACC_PUBLIC)
                PHP_ME(Kem, details, arginfo_Oqs_Kem_details, ZEND_ACC_PUBLIC)
                    PHP_ME(Kem, algorithms, arginfo_Oqs_Kem_algorithms,
                           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) PHP_FE_END};
static const zend_function_entry sig_methods[] = {
    PHP_ME(Sig, __construct, arginfo_Oqs_Sig___construct,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(Sig, keypair, arginfo_Oqs_Sig_keypair, ZEND_ACC_PUBLIC)
            PHP_ME(Sig, sign, arginfo_Oqs_Sig_sign, ZEND_ACC_PUBLIC) PHP_ME(
                Sig, verify, arginfo_Oqs_Sig_verify, ZEND_ACC_PUBLIC)
                PHP_ME(Sig, details, arginfo_Oqs_Sig_details, ZEND_ACC_PUBLIC)
                    PHP_ME(Sig, algorithms, arginfo_Oqs_Sig_algorithms,
                           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) PHP_FE_END};

PHP_MINIT_FUNCTION(oqs) {
  zend_class_entry ce;

  /* Oqs\Kem */
  INIT_NS_CLASS_ENTRY(ce, PHP_OQS_NS, "Kem", kem_methods);
  oqs_ce_kem = zend_register_internal_class(&ce);
  oqs_ce_kem->create_object = oqs_kem_create;
  memcpy(&oqs_kem_handlers, zend_get_std_object_handlers(),
         sizeof(zend_object_handlers));
  oqs_kem_handlers.offset = XtOffsetOf(php_oqs_kem_obj, std);
  oqs_kem_handlers.free_obj = oqs_kem_free;

  /* Oqs\Sig */
  INIT_NS_CLASS_ENTRY(ce, PHP_OQS_NS, "Sig", sig_methods);
  oqs_ce_sig = zend_register_internal_class(&ce);
  oqs_ce_sig->create_object = oqs_sig_create;
  memcpy(&oqs_sig_handlers, zend_get_std_object_handlers(),
         sizeof(zend_object_handlers));
  oqs_sig_handlers.offset = XtOffsetOf(php_oqs_sig_obj, std);
  oqs_sig_handlers.free_obj = oqs_sig_free;

  /* Oqs\Exception */
  INIT_NS_CLASS_ENTRY(ce, PHP_OQS_NS, "Exception", NULL);
  oqs_ce_exc = zend_register_internal_class_ex(&ce, zend_ce_exception);

  /* Register KEM Algorithm Constants */
  int kem_count = OQS_KEM_alg_count();
  for (int i = 0; i < kem_count; i++) {
    const char *name = OQS_KEM_alg_identifier(i);
    if (name && OQS_KEM_alg_is_enabled(name)) {
      char const_name[256];
      // Start with "ALG_" prefix
      strcpy(const_name, "ALG_");
      char *p = const_name + 4;

      // Sanitize the algorithm name for the constant
      for (const char *c = name; *c; c++, p++) {
        if (isalnum(*c)) {
          *p = toupper(*c);
        } else {
          *p = '_'; // Replace non-alphanumeric chars with underscore
        }
      }
      *p = '\0'; // Null-terminate the new constant name

      zend_declare_class_constant_string(oqs_ce_kem, const_name,
                                         strlen(const_name), name);
    }
  }

  /* Register SIG Algorithm Constants */
  int sig_count = OQS_SIG_alg_count();
  for (int i = 0; i < sig_count; i++) {
    const char *name = OQS_SIG_alg_identifier(i);
    if (name && OQS_SIG_alg_is_enabled(name)) {
      char const_name[256];
      strcpy(const_name, "ALG_");
      char *p = const_name + 4;

      for (const char *c = name; *c; c++, p++) {
        if (isalnum(*c)) {
          *p = toupper(*c);
        } else {
          *p = '_';
        }
      }
      *p = '\0';

      zend_declare_class_constant_string(oqs_ce_sig, const_name,
                                         strlen(const_name), name);
    }
  }

  // expose version
  REGISTER_STRING_CONSTANT("OQS\\VERSION_TEXT", OQS_VERSION_TEXT,
                           CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OQS\\VERSION_MAJOR", OQS_VERSION_MAJOR,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OQS\\VERSION_MINOR", OQS_VERSION_MINOR,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OQS\\VERSION_PATCH", OQS_VERSION_PATCH,
                         CONST_CS | CONST_PERSISTENT);

  REGISTER_STRING_CONSTANT("OQS_VERSION", PHP_OQS_VERSION,
                           CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("OQS_LIB_VERSION", PHP_OQS_LIB_VERSION,
                           CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("OQS_LIB_COMMIT", PHP_OQS_LIB_COMMIT,
                           CONST_CS | CONST_PERSISTENT);

  REGISTER_STRING_CONSTANT("OQS\\RAND_ALG_SYSTEM", "system",
                           CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("OQS\\RAND_ALG_OPENSSL", "OpenSSL",
                           CONST_CS | CONST_PERSISTENT);

  return SUCCESS;
}

PHP_MINFO_FUNCTION(oqs) {
  php_info_print_table_start();
  php_info_print_table_header(2, "oqs support", "enabled");
  php_info_print_table_row(2, "extension version", PHP_OQS_VERSION);
  php_info_print_table_row(2, "linked liboqs version", PHP_OQS_LIB_VERSION);
  php_info_print_table_row(2, "liboqs commit", PHP_OQS_LIB_COMMIT);

  // Build KEM list
  smart_string kem_list = {0};
  for (size_t i = 0; i < OQS_KEM_alg_count(); i++) {
    if (i > 0)
      smart_string_appends(&kem_list, ", ");
    smart_string_appends(&kem_list, OQS_KEM_alg_identifier(i));
  }
  smart_string_0(&kem_list);

  // Build SIG list
  smart_string sig_list = {0};
  for (size_t i = 0; i < OQS_SIG_alg_count(); i++) {
    if (i > 0)
      smart_string_appends(&sig_list, ", ");
    smart_string_appends(&sig_list, OQS_SIG_alg_identifier(i));
  }
  smart_string_0(&sig_list);

  php_info_print_table_row(2, "available KEMs", kem_list.c ? kem_list.c : "");
  php_info_print_table_row(2, "available SIGs", sig_list.c ? sig_list.c : "");

  smart_string_free(&kem_list);
  smart_string_free(&sig_list);

  php_info_print_table_end();
}

/* Oqs\randombytes_switch_algorithm(string $algorithm) */
PHP_FUNCTION(randombytes_switch_algorithm) {
  char *alg;
  size_t alg_len;
  ZEND_PARSE_PARAMETERS_START(1, 1)
  Z_PARAM_STRING(alg, alg_len)
  ZEND_PARSE_PARAMETERS_END();

  if (OQS_randombytes_switch_algorithm(alg) != OQS_SUCCESS) {
    zend_throw_exception(oqs_ce_exc, "Failed to switch RNG algorithm", 0);
    return;
  }
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_randombytes_switch_algorithm, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, algorithm, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry oqs_functions[] = {
    ZEND_NS_FE("Oqs", randombytes_switch_algorithm,
               arginfo_randombytes_switch_algorithm) PHP_FE_END};

zend_module_entry oqs_module_entry = {STANDARD_MODULE_HEADER,
                                      "oqs",
                                      oqs_functions,
                                      PHP_MINIT(oqs),
                                      NULL,
                                      NULL,
                                      NULL,
                                      PHP_MINFO(oqs),
                                      PHP_OQS_VERSION,
                                      STANDARD_MODULE_PROPERTIES};

/* Exported entry point for PHP to load this module */
#include "zend_portability.h"
ZEND_DLEXPORT zend_module_entry *get_module(void) { return &oqs_module_entry; }
