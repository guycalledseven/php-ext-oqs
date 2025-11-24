--TEST--
Oqs\Exception is thrown for invalid constructor arguments
--SKIPIF--
<?php if (!extension_loaded('oqs')) die("skip"); ?>
--FILE--
<?php
use Oqs\Kem;
use Oqs\Sig;
use Oqs\Exception as OqsException;

// Test for non-existent KEM algorithm
try {
    new Kem('NON_EXISTENT_ALGORITHM');
} catch (OqsException $e) {
    echo "Caught KEM construct failure: " . $e->getMessage() . "\n";
}

// Test for non-existent SIG algorithm
try {
    new Sig('NON_EXISTENT_ALGORITHM');
} catch (OqsException $e) {
    echo "Caught SIG construct failure: " . $e->getMessage() . "\n";
}

?>
--EXPECT--
Caught KEM construct failure: KEM algorithm not enabled
Caught SIG construct failure: SIG algorithm not enabled