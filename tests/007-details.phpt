--TEST--
Check Kem::details and Sig::details
--FILE--
<?php
use Oqs\Kem;
use Oqs\Sig;

$kem = new Kem('ML-KEM-768');
$details = $kem->details();
var_dump($details['name']);
var_dump($details['claimed_nist_level']);
var_dump($details['ind_cca']);
var_dump($details['length_public_key']);

$sig = new Sig('ML-DSA-65');
$details = $sig->details();
var_dump($details['name']);
var_dump($details['claimed_nist_level']);
var_dump($details['euf_cma']);
var_dump($details['length_public_key']);
?>
--EXPECT--
string(10) "ML-KEM-768"
int(3)
bool(true)
int(1184)
string(9) "ML-DSA-65"
int(3)
bool(true)
int(1952)
