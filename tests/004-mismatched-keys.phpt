--TEST--
Operations fail with mismatched keys
--SKIPIF--
<?php if (!extension_loaded('oqs')) die("skip"); ?>
--FILE--
<?php
use Oqs\Kem;
use Oqs\Sig;

$kem = new Kem('ML-KEM-768');
[$pk1, $sk1] = $kem->keypair();
[$pk2, $sk2] = $kem->keypair(); // A different keypair

$sig = new Sig('ML-DSA-65');
[$spk1, $ssk1] = $sig->keypair();
[$spk2, $ssk2] = $sig->keypair(); // A different signature keypair

// Test 1: Signature from sk1 should not verify with pk2
$signature = $sig->sign("message", $ssk1);
$verified = $sig->verify("message", $signature, $spk2);
var_dump($verified);

// Test 2: Verify should fail with a corrupted signature
$corrupted_sig = substr_replace($signature, "\0", 5, 1);
$verified_corrupt = $sig->verify("message", $corrupted_sig, $spk1);
var_dump($verified_corrupt);

?>
--EXPECT--
bool(false)
bool(false)