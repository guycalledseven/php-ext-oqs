# php-ext-oqs: PHP bindings for liboqs

<!-- 
![alt text](https://github.com/guycalledseven/php-oqs/actions/workflows/ci.yml/badge.svg)
-->

![alt text](https://img.shields.io/badge/license-MIT-blue.svg)

The php-ext-oqs extension makes quantum-resistant cryptography accessible in PHP by providing a direct, high-performance wrapper for the C library [liboqs](https://github.com/open-quantum-safe/liboqs/) from the [Open Quantum Safe (OQS)](https://openquantumsafe.org/) project.

It provides an object-oriented API for the NIST-standardized PQC algorithms ML-KEM (Kyber), ML-DSA (Dilithium), SLH-DSA (SPHINCS+), and other algorithms supported by liboqs.

Influenced and inspired by great [DEF CON 33 - Post Quantum Panic talk by K Karagiannis](https://www.youtube.com/watch?v=OkVYJx1iLNs).

## Features
- Key Encapsulation Mechanisms (KEMs): Securely generate and exchange shared secrets using algorithms like ML-KEM.
- Digital Signature Schemes (SIGs): Create and verify signatures with algorithms like ML-DSA.
- Modern API: A clean, object-oriented interface (Oqs\Kem, Oqs\Sig).
- Type Safe: Exposes algorithm identifiers as class constants (e.g., Oqs\Kem::ALG_ML_KEM_768) for safe, typo-free code and IDE autocompletion.
- Exception-based Error Handling: Throws Oqs\Exception for clear error handling.

## Requirements
- PHP 8.1+
- PHP development headers (php-dev, phpize)
- A C compiler (e.g., GCC, Clang) and build tools (make, cmake)
- pkg-config
- liboqs installed as a shared library on the system.

## Project files

- config.m4 - contains the code that will go into the configure script. checks for the oqs/oqs.h header and uses pkg-config to find the liboqs library, fallback to a hardcoded prefix
- php_oqs.h - header file containing C declarations and macro definitionsto be shared between several source files.  defines the necessary object structs (php_oqs_kem_obj, php_oqs_sig_obj) and utility functions for fetching these objects
- oqs.c (module init + class registration)  sets up the module entry, defines the custom object handlers, and registers the Oqs\Kem and Oqs\Sig classes with their respective methods
- sig.c (module init + class registration)
- kem.c (module init + class registration)
- tests - unit tests


## Roadmap
- [x] Expose Algorithm Details 
- [x] RNG Control 
- [ ] Implement Dedicated Key Objects?
- [x] Expose Algorithm Names as Constants
- [x] Add Documentation on KDFs

## Development Notes
- Safety: The extension is thread-safe (request-bound) and does not use globals.
- Output Format: All cryptographic outputs (keys, ciphertexts, signatures) are returned as raw binary strings.
- liboqs Version: This extension is developed and tested against the main branch of liboqs. Built against Oct 2025 commit 52169a1edfaf8b5f1593ef3a002e2553f33271a0. It is recommended to use a recent version of the library.


## Installation

Debian:

```sh
sudo apt install cmake libssl-dev php-dev
```

macOS: 

```sh
brew install cmake
```


### Build liboqs for static linking

```bash
rm -rf liboqs/build
cmake -S liboqs -B liboqs/build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DOQS_BUILD_ONLY_LIB=ON \
  -DOQS_USE_OPENSSL=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build liboqs/build --parallel
sudo cmake --build liboqs/build --target install
```

test

```sh
test -f /usr/local/lib/liboqs.a || echo "liboqs.a missing"
ls /usr/local/lib/liboqs*.dylib 2>/dev/null || echo "no dylib present (good)"
```

build php extension

```sh
export PKG_CONFIG=/usr/bin/false
rm -rf build run-tests.php
phpize
OQS_COMMIT=$(git -C liboqs rev-parse --short HEAD)
./configure --with-oqs=/usr/local \
  CPPFLAGS="-DPHP_OQS_LIB_COMMIT=\\\"$OQS_COMMIT\\\"" \
  LDFLAGS="-Wl,-force_load,/usr/local/lib/liboqs.a -Wl,-dead_strip -fvisibility=hidden"
make clean && make -j
```

confirm if static

```sh
otool -L modules/oqs.so
# Expected: does NOT list liboqs.*dylib. Only /usr/lib/libSystem.B.dylib.
```

```sh
# debian build flags!
LDFLAGS="-Wl,--whole-archive,/usr/local/lib/liboqs.a,--no-whole-archive -Wl,--exclude-libs,ALL -fvisibility=hidden"
```


### Build the extension with dynamically linked liboqs (optional)


#### Build liboqs

```bash
git clone --depth=1 https://github.com/open-quantum-safe/liboqs
cmake -S liboqs -B liboqs/build -DBUILD_SHARED_LIBS=ON -DOQS_DIST_BUILD=ON
cmake --build liboqs/build --parallel
sudo cmake --build liboqs/build --target install
sudo ldconfig || true
```

#### Build extension

```bash
rm -rf build/* run-tests.php
phpize

# macOS
./configure --with-php-config=/opt/homebrew/opt/php/bin/php-config

# Debian 
# ./configure --enable-oqs
phpize
OQS_COMMIT=$(git -C /path/to/liboqs rev-parse --short HEAD) \
./configure --with-oqs=/usr/local

make clean && make -j
```

### Test and use

```sh
php -q run_tests.php
cp modules/oqs.so "$(php -r 'echo ini_get("extension_dir") . "\n";')"

# enable macOS
xattr -dr com.apple.quarantine /opt/homebrew/lib/php/pecl/20240924/oqs.so
install_name_tool -add_rpath /opt/homebrew/lib "$(php -i | awk -F'=> ' '/^extension_dir/ {print $2}')/oqs.so"

# enable on linux
php -i | grep extension_dir
echo "extension=oqs.so" | sudo tee /etc/php/$(php -r 'echo PHP_MAJOR_VERSION.".".PHP_MINOR_VERSION;')/mods-available/oqs.ini
sudo phpenmod oqs 2>/dev/null || true



# php -r '$f = php_ini_scanned_files(); echo $f ? dirname(explode(",", $f)[0]) . "\n" : "none\n";'
```

`php -q --info`

test in browser with phpinfo

```sh
php -S localhost:8000 -t www
open http://localhost:8000
```

macOS config after setup 

```conf
/opt/homebrew/etc/php/8.4/conf.d/oqs.ini
extension=oqs
```

### A Stub & Docs Generator Script

Generate the IDE stub file:

```sh
php generate-docs.php --format=stub > oqs.stub.php
```

Generate the Markdown file: (This command is unchanged)

```sh
php generate-docs.php --format=markdown > OQS_CONSTANTS.md
```

### Test if working

```sh
php -m | grep oqs
php -r '[$k]=[new Oqs\Kem("ML-KEM-768")]; var_dump($k->sizes());'
```

### Debugging

macOS:

```sh
 otool -L oqs.so
oqs.so:
	@rpath/liboqs.8.dylib (compatibility version 8.0.0, current version 0.14.1)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1356.0.0)

nm -gU oqs.so
00000000000009dc T _get_module
00000000000081d8 S _oqs_ce_kem
00000000000081e0 S _oqs_ce_sig
0000000000008128 D _oqs_module_entry
00000000000009e8 T _zim_Kem___construct
0000000000000f58 T _zim_Kem_algorithms
0000000000000d9c T _zim_Kem_decap
0000000000000cec T _zim_Kem_encap
0000000000000b78 T _zim_Kem_keypair
0000000000000ac0 T _zim_Kem_sizes
0000000000000fe8 T _zim_Sig___construct
0000000000001658 T _zim_Sig_algorithms
00000000000010c0 T _zim_Sig_keypair
0000000000001234 T _zim_Sig_sign
00000000000014d8 T _zim_Sig_verify
00000000000009a4 T _zm_info_oqs
00000000000006d0 T _zm_startup_oqs
```

linux: 

```sh
ldd modules/oqs.so
	linux-vdso.so.1 (0x0000f8f78ca51000)
	liboqs.so.8 => /usr/local/lib/liboqs.so.8 (0x0000f8f78c430000)
	libc.so.6 => /lib/aarch64-linux-gnu/libc.so.6 (0x0000f8f78c270000)
	/lib/ld-linux-aarch64.so.1 (0x0000f8f78ca14000)
	libcrypto.so.3 => /lib/aarch64-linux-gnu/libcrypto.so.3 (0x0000f8f78bdf0000)

nm -gU modules/oqs.so 
0000000000001f30 T get_module
0000000000020398 B oqs_ce_kem
0000000000020390 B oqs_ce_sig
0000000000020150 D oqs_module_entry
00000000000027a4 T zim_Kem_algorithms
0000000000001f40 T zim_Kem___construct
0000000000002570 T zim_Kem_decap
0000000000002320 T zim_Kem_encap
0000000000002190 T zim_Kem_keypair
00000000000020c0 T zim_Kem_sizes
0000000000003050 T zim_Sig_algorithms
0000000000002830 T zim_Sig___construct
00000000000029b0 T zim_Sig_keypair
0000000000002b40 T zim_Sig_sign
0000000000002e50 T zim_Sig_verify
0000000000001ab0 T zm_info_oqs
0000000000001d30 T zm_startup_oqs

objdump --disassemble-all modules/oqs.so
```


## Important Security Point - clarification of the role of a KDF

KEM implementation returns a raw shared secret. In practice, this shared secret **should almost never be used directly as an encryption key**. It should be used as input to a Key Derivation Function (KDF) like HKDF to produce keys of the desired length and properties.

It is strongly recommended to use PHP's built-in hash_hkdf() function.

```php
// Example of best practice
[$ct, $shared_secret] = $kem->encap($pk);

// Now, derive a 32-byte AES key and a 16-byte IV from the shared secret
$keying_material = hash_hkdf('sha256', $shared_secret, 48, 'my-app-encryption', 'some-salt');
$aes_key = substr($keying_material, 0, 32);
$iv = substr($keying_material, 32, 16);
```



## Quick start

```php
<?php
use Oqs\Kem;
use Oqs\Sig;

// 1. Key Encapsulation (KEM) Example with ML-KEM
$kem = new Kem(Kem::ALG_ML_KEM_768);

// Generate a public/private key pair
[$publicKey, $secretKey] = $kem->keypair();

// A client encapsulates a secret against the public key,
// generating a ciphertext and a shared secret.
[$ciphertext, $sharedSecretA] = $kem->encap($publicKey);

// The server decapsulates the ciphertext with the secret key
// to derive the same shared secret.
$sharedSecretB = $kem->decap($ciphertext, $secretKey);

assert($sharedSecretA === $sharedSecretB);
echo "KEM shared secrets match!\n";


// IMPORTANT: The raw shared secret should be passed through a KDF
// like HKDF before being used as an encryption key.
$encryption_key = hash_hkdf('sha256', $sharedSecretA, 32, 'my-app-aes-key');


// 2. Digital Signature (SIG) Example with ML-DSA
$sig = new Sig(Sig::ALG_ML_DSA_65);

// Generate a signing key pair
[$signingPublicKey, $signingSecretKey] = $sig->keypair();

$message = "This is the message to sign.";

// Sign the message with the secret key
$signature = $sig->sign($message, $signingSecretKey);

// Verify the signature against the message and public key
$isValid = $sig->verify($message, $signature, $signingPublicKey);

assert($isValid === true);
echo "Signature is valid!\n";

```

### 3. Algorithm Details

You can retrieve detailed metadata about an algorithm using the `details()` method.

```php
$kem = new Oqs\Kem('Kyber512');
$details = $kem->details();
print_r($details);
/* Output:
Array
(
    [name] => Kyber512
    [version] => ...
    [claimed_nist_level] => 1
    [ind_cca] => 1
    [length_public_key] => 800
    [length_secret_key] => 1632
    [length_ciphertext] => 768
    [length_shared_secret] => 32
)
*/
```

### 4. Random Number Generator (RNG)

You can switch the underlying random number generator used by `liboqs`.

```php
// Switch to System RNG (default)
try {
    Oqs\randombytes_switch_algorithm(Oqs\RAND_ALG_SYSTEM);
} catch (Oqs\Exception $e) {
    // Handle error
}

// Switch to OpenSSL RNG
try {
    Oqs\randombytes_switch_algorithm(Oqs\RAND_ALG_OPENSSL);
} catch (Oqs\Exception $e) {
    // Handle error (e.g., OpenSSL not supported in liboqs build)
}
```



### PHP API test

```php
use Oqs\Kem;
use Oqs\Sig;
use Oqs\Exception as OqsException;

$kem = new Kem('ML-KEM-768');
$s = $kem->sizes();                 // ['pk'=>..,'sk'=>..,'ct'=>..,'ss'=>..]
[$pk,$sk] = $kem->keypair();        // returns [pk, sk] as binary strings
[$ct,$ssA] = $kem->encap($pk);      // encaps → [ciphertext, shared_secret]
$ssB = $kem->decap($ct, $sk);       // decaps → shared_secret
assert($ssA === $ssB);

$sig = new Sig('ML-DSA-65');
[$spk,$ssk] = $sig->keypair();
$signature = $sig->sign("hello", $ssk);
$ok = $sig->verify("hello", $signature, $spk); // bool

Kem::algorithms(); // list enabled KEM names
Sig::algorithms(); // list enabled SIG names


try {
    // This will now throw an Oqs\Exception
    $kem = new Kem('Some-Invalid-Algorithm');

} catch (OqsException $e) {
    // This block will ONLY catch exceptions thrown from your extension.
    echo "Caught an OQS-specific error: " . $e->getMessage();

} catch (\Exception $e) {
    // Other general errors would be caught here.
    echo "Caught a generic error: " . $e->getMessage();
}
```


Comparation with similar php extension [php-liboqs](https://github.com/secudoc/php-liboqs):

* Object-oriented API design vs static.
* Error handling with a custom `Oqs\Exception`.
* Complete function signatures (`arginfo`) for better IDE/static analysis support.
* An extra useful method (`Kem::sizes()`).


### Global Constants

These constants provide version information for the underlying `liboqs` C library that the extension was compiled against.

| Constant Name | Value |
|---------------|-------|
| `Oqs\VERSION_MAJOR` | `0` |
| `Oqs\VERSION_MINOR` | `14` |
| `Oqs\VERSION_PATCH` | `1` |
| `Oqs\VERSION_TEXT` | `0.14.1-dev` |

### `Oqs\Kem` Constants

| Constant Name | Algorithm Value |
|---------------|-----------------|
| `ALG_BIKE_L1` | `BIKE-L1` |
| `ALG_BIKE_L3` | `BIKE-L3` |
| `ALG_BIKE_L5` | `BIKE-L5` |
| `ALG_CLASSIC_MCELIECE_348864` | `Classic-McEliece-348864` |
| `ALG_CLASSIC_MCELIECE_348864F` | `Classic-McEliece-348864f` |
| `ALG_CLASSIC_MCELIECE_460896` | `Classic-McEliece-460896` |
| `ALG_CLASSIC_MCELIECE_460896F` | `Classic-McEliece-460896f` |
| `ALG_CLASSIC_MCELIECE_6688128` | `Classic-McEliece-6688128` |
| `ALG_CLASSIC_MCELIECE_6688128F` | `Classic-McEliece-6688128f` |
| `ALG_CLASSIC_MCELIECE_6960119` | `Classic-McEliece-6960119` |
| `ALG_CLASSIC_MCELIECE_6960119F` | `Classic-McEliece-6960119f` |
| `ALG_CLASSIC_MCELIECE_8192128` | `Classic-McEliece-8192128` |
| `ALG_CLASSIC_MCELIECE_8192128F` | `Classic-McEliece-8192128f` |
| `ALG_FRODOKEM_1344_AES` | `FrodoKEM-1344-AES` |
| `ALG_FRODOKEM_1344_SHAKE` | `FrodoKEM-1344-SHAKE` |
| `ALG_FRODOKEM_640_AES` | `FrodoKEM-640-AES` |
| `ALG_FRODOKEM_640_SHAKE` | `FrodoKEM-640-SHAKE` |
| `ALG_FRODOKEM_976_AES` | `FrodoKEM-976-AES` |
| `ALG_FRODOKEM_976_SHAKE` | `FrodoKEM-976-SHAKE` |
| `ALG_KYBER1024` | `Kyber1024` |
| `ALG_KYBER512` | `Kyber512` |
| `ALG_KYBER768` | `Kyber768` |
| `ALG_ML_KEM_1024` | `ML-KEM-1024` |
| `ALG_ML_KEM_512` | `ML-KEM-512` |
| `ALG_ML_KEM_768` | `ML-KEM-768` |
| `ALG_NTRU_HPS_2048_509` | `NTRU-HPS-2048-509` |
| `ALG_NTRU_HPS_2048_677` | `NTRU-HPS-2048-677` |
| `ALG_NTRU_HPS_4096_1229` | `NTRU-HPS-4096-1229` |
| `ALG_NTRU_HPS_4096_821` | `NTRU-HPS-4096-821` |
| `ALG_NTRU_HRSS_1373` | `NTRU-HRSS-1373` |
| `ALG_NTRU_HRSS_701` | `NTRU-HRSS-701` |
| `ALG_SNTRUP761` | `sntrup761` |

### `Oqs\Sig` Constants

| Constant Name | Algorithm Value |
|---------------|-----------------|
| `ALG_CROSS_RSDPG_128_BALANCED` | `cross-rsdpg-128-balanced` |
| `ALG_CROSS_RSDPG_128_FAST` | `cross-rsdpg-128-fast` |
| `ALG_CROSS_RSDPG_128_SMALL` | `cross-rsdpg-128-small` |
| `ALG_CROSS_RSDPG_192_BALANCED` | `cross-rsdpg-192-balanced` |
| `ALG_CROSS_RSDPG_192_FAST` | `cross-rsdpg-192-fast` |
| `ALG_CROSS_RSDPG_192_SMALL` | `cross-rsdpg-192-small` |
| `ALG_CROSS_RSDPG_256_BALANCED` | `cross-rsdpg-256-balanced` |
| `ALG_CROSS_RSDPG_256_FAST` | `cross-rsdpg-256-fast` |
| `ALG_CROSS_RSDPG_256_SMALL` | `cross-rsdpg-256-small` |
| `ALG_CROSS_RSDP_128_BALANCED` | `cross-rsdp-128-balanced` |
| `ALG_CROSS_RSDP_128_FAST` | `cross-rsdp-128-fast` |
| `ALG_CROSS_RSDP_128_SMALL` | `cross-rsdp-128-small` |
| `ALG_CROSS_RSDP_192_BALANCED` | `cross-rsdp-192-balanced` |
| `ALG_CROSS_RSDP_192_FAST` | `cross-rsdp-192-fast` |
| `ALG_CROSS_RSDP_192_SMALL` | `cross-rsdp-192-small` |
| `ALG_CROSS_RSDP_256_BALANCED` | `cross-rsdp-256-balanced` |
| `ALG_CROSS_RSDP_256_FAST` | `cross-rsdp-256-fast` |
| `ALG_CROSS_RSDP_256_SMALL` | `cross-rsdp-256-small` |
| `ALG_FALCON_1024` | `Falcon-1024` |
| `ALG_FALCON_512` | `Falcon-512` |
| `ALG_FALCON_PADDED_1024` | `Falcon-padded-1024` |
| `ALG_FALCON_PADDED_512` | `Falcon-padded-512` |
| `ALG_MAYO_1` | `MAYO-1` |
| `ALG_MAYO_2` | `MAYO-2` |
| `ALG_MAYO_3` | `MAYO-3` |
| `ALG_MAYO_5` | `MAYO-5` |
| `ALG_ML_DSA_44` | `ML-DSA-44` |
| `ALG_ML_DSA_65` | `ML-DSA-65` |
| `ALG_ML_DSA_87` | `ML-DSA-87` |
| `ALG_OV_III` | `OV-III` |
| `ALG_OV_III_PKC` | `OV-III-pkc` |
| `ALG_OV_III_PKC_SKC` | `OV-III-pkc-skc` |
| `ALG_OV_IP` | `OV-Ip` |
| `ALG_OV_IP_PKC` | `OV-Ip-pkc` |
| `ALG_OV_IP_PKC_SKC` | `OV-Ip-pkc-skc` |
| `ALG_OV_IS` | `OV-Is` |
| `ALG_OV_IS_PKC` | `OV-Is-pkc` |
| `ALG_OV_IS_PKC_SKC` | `OV-Is-pkc-skc` |
| `ALG_OV_V` | `OV-V` |
| `ALG_OV_V_PKC` | `OV-V-pkc` |
| `ALG_OV_V_PKC_SKC` | `OV-V-pkc-skc` |
| `ALG_SLH_DSA_PURE_SHA2_128F` | `SLH_DSA_PURE_SHA2_128F` |
| `ALG_SLH_DSA_PURE_SHA2_128S` | `SLH_DSA_PURE_SHA2_128S` |
| `ALG_SLH_DSA_PURE_SHA2_192F` | `SLH_DSA_PURE_SHA2_192F` |
| `ALG_SLH_DSA_PURE_SHA2_192S` | `SLH_DSA_PURE_SHA2_192S` |
| `ALG_SLH_DSA_PURE_SHA2_256F` | `SLH_DSA_PURE_SHA2_256F` |
| `ALG_SLH_DSA_PURE_SHA2_256S` | `SLH_DSA_PURE_SHA2_256S` |
| `ALG_SLH_DSA_PURE_SHAKE_128F` | `SLH_DSA_PURE_SHAKE_128F` |
| `ALG_SLH_DSA_PURE_SHAKE_128S` | `SLH_DSA_PURE_SHAKE_128S` |
| `ALG_SLH_DSA_PURE_SHAKE_192F` | `SLH_DSA_PURE_SHAKE_192F` |
| `ALG_SLH_DSA_PURE_SHAKE_192S` | `SLH_DSA_PURE_SHAKE_192S` |
| `ALG_SLH_DSA_PURE_SHAKE_256F` | `SLH_DSA_PURE_SHAKE_256F` |
| `ALG_SLH_DSA_PURE_SHAKE_256S` | `SLH_DSA_PURE_SHAKE_256S` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHA2_128F` | `SLH_DSA_SHA2_224_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHA2_128S` | `SLH_DSA_SHA2_224_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHA2_192F` | `SLH_DSA_SHA2_224_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHA2_192S` | `SLH_DSA_SHA2_224_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHA2_256F` | `SLH_DSA_SHA2_224_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHA2_256S` | `SLH_DSA_SHA2_224_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHAKE_128F` | `SLH_DSA_SHA2_224_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHAKE_128S` | `SLH_DSA_SHA2_224_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHAKE_192F` | `SLH_DSA_SHA2_224_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHAKE_192S` | `SLH_DSA_SHA2_224_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHAKE_256F` | `SLH_DSA_SHA2_224_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHA2_224_PREHASH_SHAKE_256S` | `SLH_DSA_SHA2_224_PREHASH_SHAKE_256S` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHA2_128F` | `SLH_DSA_SHA2_256_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHA2_128S` | `SLH_DSA_SHA2_256_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHA2_192F` | `SLH_DSA_SHA2_256_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHA2_192S` | `SLH_DSA_SHA2_256_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHA2_256F` | `SLH_DSA_SHA2_256_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHA2_256S` | `SLH_DSA_SHA2_256_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHAKE_128F` | `SLH_DSA_SHA2_256_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHAKE_128S` | `SLH_DSA_SHA2_256_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHAKE_192F` | `SLH_DSA_SHA2_256_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHAKE_192S` | `SLH_DSA_SHA2_256_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHAKE_256F` | `SLH_DSA_SHA2_256_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHA2_256_PREHASH_SHAKE_256S` | `SLH_DSA_SHA2_256_PREHASH_SHAKE_256S` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHA2_128F` | `SLH_DSA_SHA2_384_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHA2_128S` | `SLH_DSA_SHA2_384_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHA2_192F` | `SLH_DSA_SHA2_384_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHA2_192S` | `SLH_DSA_SHA2_384_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHA2_256F` | `SLH_DSA_SHA2_384_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHA2_256S` | `SLH_DSA_SHA2_384_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHAKE_128F` | `SLH_DSA_SHA2_384_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHAKE_128S` | `SLH_DSA_SHA2_384_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHAKE_192F` | `SLH_DSA_SHA2_384_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHAKE_192S` | `SLH_DSA_SHA2_384_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHAKE_256F` | `SLH_DSA_SHA2_384_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHA2_384_PREHASH_SHAKE_256S` | `SLH_DSA_SHA2_384_PREHASH_SHAKE_256S` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHA2_128F` | `SLH_DSA_SHA2_512_224_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHA2_128S` | `SLH_DSA_SHA2_512_224_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHA2_192F` | `SLH_DSA_SHA2_512_224_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHA2_192S` | `SLH_DSA_SHA2_512_224_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHA2_256F` | `SLH_DSA_SHA2_512_224_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHA2_256S` | `SLH_DSA_SHA2_512_224_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHAKE_128F` | `SLH_DSA_SHA2_512_224_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHAKE_128S` | `SLH_DSA_SHA2_512_224_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHAKE_192F` | `SLH_DSA_SHA2_512_224_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHAKE_192S` | `SLH_DSA_SHA2_512_224_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHAKE_256F` | `SLH_DSA_SHA2_512_224_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHA2_512_224_PREHASH_SHAKE_256S` | `SLH_DSA_SHA2_512_224_PREHASH_SHAKE_256S` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHA2_128F` | `SLH_DSA_SHA2_512_256_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHA2_128S` | `SLH_DSA_SHA2_512_256_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHA2_192F` | `SLH_DSA_SHA2_512_256_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHA2_192S` | `SLH_DSA_SHA2_512_256_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHA2_256F` | `SLH_DSA_SHA2_512_256_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHA2_256S` | `SLH_DSA_SHA2_512_256_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHAKE_128F` | `SLH_DSA_SHA2_512_256_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHAKE_128S` | `SLH_DSA_SHA2_512_256_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHAKE_192F` | `SLH_DSA_SHA2_512_256_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHAKE_192S` | `SLH_DSA_SHA2_512_256_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHAKE_256F` | `SLH_DSA_SHA2_512_256_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHA2_512_256_PREHASH_SHAKE_256S` | `SLH_DSA_SHA2_512_256_PREHASH_SHAKE_256S` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHA2_128F` | `SLH_DSA_SHA2_512_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHA2_128S` | `SLH_DSA_SHA2_512_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHA2_192F` | `SLH_DSA_SHA2_512_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHA2_192S` | `SLH_DSA_SHA2_512_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHA2_256F` | `SLH_DSA_SHA2_512_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHA2_256S` | `SLH_DSA_SHA2_512_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHAKE_128F` | `SLH_DSA_SHA2_512_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHAKE_128S` | `SLH_DSA_SHA2_512_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHAKE_192F` | `SLH_DSA_SHA2_512_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHAKE_192S` | `SLH_DSA_SHA2_512_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHAKE_256F` | `SLH_DSA_SHA2_512_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHA2_512_PREHASH_SHAKE_256S` | `SLH_DSA_SHA2_512_PREHASH_SHAKE_256S` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHA2_128F` | `SLH_DSA_SHA3_224_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHA2_128S` | `SLH_DSA_SHA3_224_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHA2_192F` | `SLH_DSA_SHA3_224_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHA2_192S` | `SLH_DSA_SHA3_224_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHA2_256F` | `SLH_DSA_SHA3_224_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHA2_256S` | `SLH_DSA_SHA3_224_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHAKE_128F` | `SLH_DSA_SHA3_224_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHAKE_128S` | `SLH_DSA_SHA3_224_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHAKE_192F` | `SLH_DSA_SHA3_224_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHAKE_192S` | `SLH_DSA_SHA3_224_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHAKE_256F` | `SLH_DSA_SHA3_224_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHA3_224_PREHASH_SHAKE_256S` | `SLH_DSA_SHA3_224_PREHASH_SHAKE_256S` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHA2_128F` | `SLH_DSA_SHA3_256_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHA2_128S` | `SLH_DSA_SHA3_256_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHA2_192F` | `SLH_DSA_SHA3_256_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHA2_192S` | `SLH_DSA_SHA3_256_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHA2_256F` | `SLH_DSA_SHA3_256_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHA2_256S` | `SLH_DSA_SHA3_256_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHAKE_128F` | `SLH_DSA_SHA3_256_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHAKE_128S` | `SLH_DSA_SHA3_256_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHAKE_192F` | `SLH_DSA_SHA3_256_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHAKE_192S` | `SLH_DSA_SHA3_256_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHAKE_256F` | `SLH_DSA_SHA3_256_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHA3_256_PREHASH_SHAKE_256S` | `SLH_DSA_SHA3_256_PREHASH_SHAKE_256S` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHA2_128F` | `SLH_DSA_SHA3_384_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHA2_128S` | `SLH_DSA_SHA3_384_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHA2_192F` | `SLH_DSA_SHA3_384_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHA2_192S` | `SLH_DSA_SHA3_384_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHA2_256F` | `SLH_DSA_SHA3_384_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHA2_256S` | `SLH_DSA_SHA3_384_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHAKE_128F` | `SLH_DSA_SHA3_384_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHAKE_128S` | `SLH_DSA_SHA3_384_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHAKE_192F` | `SLH_DSA_SHA3_384_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHAKE_192S` | `SLH_DSA_SHA3_384_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHAKE_256F` | `SLH_DSA_SHA3_384_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHA3_384_PREHASH_SHAKE_256S` | `SLH_DSA_SHA3_384_PREHASH_SHAKE_256S` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHA2_128F` | `SLH_DSA_SHA3_512_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHA2_128S` | `SLH_DSA_SHA3_512_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHA2_192F` | `SLH_DSA_SHA3_512_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHA2_192S` | `SLH_DSA_SHA3_512_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHA2_256F` | `SLH_DSA_SHA3_512_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHA2_256S` | `SLH_DSA_SHA3_512_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHAKE_128F` | `SLH_DSA_SHA3_512_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHAKE_128S` | `SLH_DSA_SHA3_512_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHAKE_192F` | `SLH_DSA_SHA3_512_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHAKE_192S` | `SLH_DSA_SHA3_512_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHAKE_256F` | `SLH_DSA_SHA3_512_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHA3_512_PREHASH_SHAKE_256S` | `SLH_DSA_SHA3_512_PREHASH_SHAKE_256S` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHA2_128F` | `SLH_DSA_SHAKE_128_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHA2_128S` | `SLH_DSA_SHAKE_128_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHA2_192F` | `SLH_DSA_SHAKE_128_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHA2_192S` | `SLH_DSA_SHAKE_128_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHA2_256F` | `SLH_DSA_SHAKE_128_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHA2_256S` | `SLH_DSA_SHAKE_128_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHAKE_128F` | `SLH_DSA_SHAKE_128_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHAKE_128S` | `SLH_DSA_SHAKE_128_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHAKE_192F` | `SLH_DSA_SHAKE_128_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHAKE_192S` | `SLH_DSA_SHAKE_128_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHAKE_256F` | `SLH_DSA_SHAKE_128_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHAKE_128_PREHASH_SHAKE_256S` | `SLH_DSA_SHAKE_128_PREHASH_SHAKE_256S` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHA2_128F` | `SLH_DSA_SHAKE_256_PREHASH_SHA2_128F` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHA2_128S` | `SLH_DSA_SHAKE_256_PREHASH_SHA2_128S` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHA2_192F` | `SLH_DSA_SHAKE_256_PREHASH_SHA2_192F` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHA2_192S` | `SLH_DSA_SHAKE_256_PREHASH_SHA2_192S` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHA2_256F` | `SLH_DSA_SHAKE_256_PREHASH_SHA2_256F` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHA2_256S` | `SLH_DSA_SHAKE_256_PREHASH_SHA2_256S` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHAKE_128F` | `SLH_DSA_SHAKE_256_PREHASH_SHAKE_128F` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHAKE_128S` | `SLH_DSA_SHAKE_256_PREHASH_SHAKE_128S` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHAKE_192F` | `SLH_DSA_SHAKE_256_PREHASH_SHAKE_192F` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHAKE_192S` | `SLH_DSA_SHAKE_256_PREHASH_SHAKE_192S` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHAKE_256F` | `SLH_DSA_SHAKE_256_PREHASH_SHAKE_256F` |
| `ALG_SLH_DSA_SHAKE_256_PREHASH_SHAKE_256S` | `SLH_DSA_SHAKE_256_PREHASH_SHAKE_256S` |
| `ALG_SNOVA_24_5_4` | `SNOVA_24_5_4` |
| `ALG_SNOVA_24_5_4_ESK` | `SNOVA_24_5_4_esk` |
| `ALG_SNOVA_24_5_4_SHAKE` | `SNOVA_24_5_4_SHAKE` |
| `ALG_SNOVA_24_5_4_SHAKE_ESK` | `SNOVA_24_5_4_SHAKE_esk` |
| `ALG_SNOVA_24_5_5` | `SNOVA_24_5_5` |
| `ALG_SNOVA_25_8_3` | `SNOVA_25_8_3` |
| `ALG_SNOVA_29_6_5` | `SNOVA_29_6_5` |
| `ALG_SNOVA_37_17_2` | `SNOVA_37_17_2` |
| `ALG_SNOVA_37_8_4` | `SNOVA_37_8_4` |
| `ALG_SNOVA_49_11_3` | `SNOVA_49_11_3` |
| `ALG_SNOVA_56_25_2` | `SNOVA_56_25_2` |
| `ALG_SNOVA_60_10_4` | `SNOVA_60_10_4` |
| `ALG_SPHINCS__SHA2_128F_SIMPLE` | `SPHINCS+-SHA2-128f-simple` |
| `ALG_SPHINCS__SHA2_128S_SIMPLE` | `SPHINCS+-SHA2-128s-simple` |
| `ALG_SPHINCS__SHA2_192F_SIMPLE` | `SPHINCS+-SHA2-192f-simple` |
| `ALG_SPHINCS__SHA2_192S_SIMPLE` | `SPHINCS+-SHA2-192s-simple` |
| `ALG_SPHINCS__SHA2_256F_SIMPLE` | `SPHINCS+-SHA2-256f-simple` |
| `ALG_SPHINCS__SHA2_256S_SIMPLE` | `SPHINCS+-SHA2-256s-simple` |
| `ALG_SPHINCS__SHAKE_128F_SIMPLE` | `SPHINCS+-SHAKE-128f-simple` |
| `ALG_SPHINCS__SHAKE_128S_SIMPLE` | `SPHINCS+-SHAKE-128s-simple` |
| `ALG_SPHINCS__SHAKE_192F_SIMPLE` | `SPHINCS+-SHAKE-192f-simple` |
| `ALG_SPHINCS__SHAKE_192S_SIMPLE` | `SPHINCS+-SHAKE-192s-simple` |
| `ALG_SPHINCS__SHAKE_256F_SIMPLE` | `SPHINCS+-SHAKE-256f-simple` |
| `ALG_SPHINCS__SHAKE_256S_SIMPLE` | `SPHINCS+-SHAKE-256s-simple` |

