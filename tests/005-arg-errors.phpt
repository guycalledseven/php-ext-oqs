--TEST--
Test for argument count and type errors
--SKIPIF--
<?php if (!extension_loaded('oqs')) die("skip"); ?>
--FILE--
<?php
use Oqs\Kem;
use Oqs\Sig;

try {
    new Kem();
} catch (ArgumentCountError $e) {
    echo "Caught: " . $e->getMessage() . "\n";
}

try {
    $kem = new Kem('ML-KEM-768');
    $kem->decap("ciphertext"); // Missing second argument
} catch (ArgumentCountError $e) {
    echo "Caught: " . $e->getMessage() . "\n";
}

try {
    $sig = new Sig('ML-DSA-65');
    $sig->sign("message", ["not a string"]); // Wrong type
} catch (TypeError $e) {
    echo "Caught: " . $e->getMessage() . "\n";
}
?>
--EXPECTF--
Caught: Oqs\Kem::__construct() expects exactly 1 argument, 0 given
Caught: Oqs\Kem::decap() expects exactly 2 arguments, 1 given
Caught: Oqs\Sig::sign(): Argument #2 ($secretKey) must be of type string, array given