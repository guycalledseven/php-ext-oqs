<?php

use Oqs\Kem;
use Oqs\Sig;

$kem = new Kem('ML-KEM-768');
$s = $kem->sizes();                 // ['pk'=>..,'sk'=>..,'ct'=>..,'ss'=>..]
[$pk, $sk] = $kem->keypair();        // returns [pk, sk] as binary strings
[$ct, $ssA] = $kem->encap($pk);      // encaps → [ciphertext, shared_secret]
$ssB = $kem->decap($ct, $sk);       // decaps → shared_secret
assert($ssA === $ssB);

$sig = new Sig('ML-DSA-65');
[$spk, $ssk] = $sig->keypair();
$signature = $sig->sign("hello", $ssk);
$ok = $sig->verify("hello", $signature, $spk); // bool

Kem::algorithms(); // list enabled KEM names
Sig::algorithms(); // list enabled SIG names

echo "KEM algos" . PHP_EOL;
print_r(Kem::algorithms());
echo "Sig algos" . PHP_EOL;
print_r(Sig::algorithms());

var_dump(bin2hex($ssA));
var_dump(bin2hex($signature));
var_dump($ok);

$kem = new Kem('ML-KEM-768');
[$pk, $sk] = $kem->keypair();
[$ct, $ss1] = $kem->encap($pk);
$ss2 = $kem->decap($ct, $sk);
assert(hash_equals($ss1, $ss2));

$sig = new Sig('ML-DSA-65');
[$spk, $ssk] = $sig->keypair();
$s = $sig->sign("hello", $ssk);
assert($sig->verify("hello", $s, $spk) === true);

/**
 * details test
 */

echo "KEM details test" . PHP_EOL;

$kem = new Kem('ML-KEM-768');
$kDetails = $kem->details();

assert($kDetails['name'] === 'ML-KEM-768');
assert($kDetails['claimed_nist_level'] === 3);
assert($kDetails['ind_cca'] === true);
assert($kDetails['length_public_key'] === 1184);

echo "SIG details test" . PHP_EOL;

$sig = new Sig('ML-DSA-65');
$sDetails = $sig->details();

assert($sDetails['name'] === 'ML-DSA-65');
assert($sDetails['claimed_nist_level'] === 3);
assert($sDetails['euf_cma'] === true);
assert($sDetails['length_public_key'] === 1952);


/**
 * exceptions
 */

echo "Exception test" . PHP_EOL;

use Oqs\Exception as OqsException;

try {
    // This will now throw an Oqs\Exception
    $kem = new Kem('Some-Invalid-Algorithm');

} catch (OqsException $e) {
    // This block will ONLY catch exceptions thrown from your extension.
    echo "Caught an OQS-specific error: " . $e->getMessage() . PHP_EOL;

} catch (\Exception $e) {
    // Other general errors would be caught here.
    echo "Caught a generic error: " . $e->getMessage() . PHP_EOL;
}


/**
 * constants for algo names
 */

echo "Constants test" . PHP_EOL;

// Safe, autocompletes, self-documenting
$kem = new Kem(Oqs\Kem::ALG_ML_KEM_768);
$sig = new Sig(Oqs\Sig::ALG_ML_DSA_65);

// You can still see the original value
echo Oqs\Kem::ALG_ML_KEM_768; // Outputs: ML-KEM-768


// version constants
use const Oqs\VERSION_MAJOR;
use const Oqs\VERSION_MINOR;
use const Oqs\VERSION_TEXT;

// Print the full version string provided by liboqs
printf("The php-oqs extension is compiled against liboqs version: %s\n", VERSION_TEXT);

// You can also use the version numbers for programmatic checks
if (VERSION_MAJOR >= 0 && VERSION_MINOR >= 10) {
    echo "Detected liboqs version 0.10.0 or newer.\n";
}

// You can also access them with their fully qualified name
echo "The patch version is: " . \Oqs\VERSION_PATCH . "\n";
