--TEST--
Wrong-length keys, ciphertexts and signatures are rejected
--SKIPIF--
<?php if (!extension_loaded('oqs')) die("skip"); ?>
--FILE--
<?php
use Oqs\Kem;
use Oqs\Sig;

function attempt(string $label, callable $fn): void
{
    try {
        $fn();
        echo "$label: no exception\n";
    } catch (Oqs\Exception $e) {
        echo "$label: " . $e->getMessage() . "\n";
    }
}

$kem = new Kem('ML-KEM-768');
[$pk, $sk] = $kem->keypair();
[$ct, $ss] = $kem->encap($pk);

attempt('encap short pk', fn () => $kem->encap(substr($pk, 0, -1)));
attempt('encap empty pk', fn () => $kem->encap(''));
attempt('encap long pk', fn () => $kem->encap($pk . "\0"));
attempt('decap short ct', fn () => $kem->decap(substr($ct, 0, -1), $sk));
attempt('decap short sk', fn () => $kem->decap($ct, 'short'));
attempt('decap long sk', fn () => $kem->decap($ct, $sk . "\0"));

$sig = new Sig('ML-DSA-65');
[$spk, $ssk] = $sig->keypair();
$signature = $sig->sign('message', $ssk);

attempt('sign short sk', fn () => $sig->sign('message', 'short'));
attempt('sign long sk', fn () => $sig->sign('message', $ssk . "\0"));
attempt('verify short pk', fn () => $sig->verify('message', $signature, 'short'));

// Signatures are variable-length, so a bad length is "not valid", not an error
var_dump($sig->verify('message', '', $spk));
var_dump($sig->verify('message', substr($signature, 0, -1), $spk));
var_dump($sig->verify('message', $signature . "\0", $spk));
var_dump($sig->verify('message', $signature, $spk));
?>
--EXPECT--
encap short pk: Invalid public key length: expected 1184 bytes, got 1183
encap empty pk: Invalid public key length: expected 1184 bytes, got 0
encap long pk: Invalid public key length: expected 1184 bytes, got 1185
decap short ct: Invalid ciphertext length: expected 1088 bytes, got 1087
decap short sk: Invalid secret key length: expected 2400 bytes, got 5
decap long sk: Invalid secret key length: expected 2400 bytes, got 2401
sign short sk: Invalid secret key length: expected 4032 bytes, got 5
sign long sk: Invalid secret key length: expected 4032 bytes, got 4033
verify short pk: Invalid public key length: expected 1952 bytes, got 5
bool(false)
bool(false)
bool(false)
bool(true)
