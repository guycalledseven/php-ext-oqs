--TEST--
OQS basic Kem/Sig
--SKIPIF--
<?php if (!extension_loaded('oqs')) die("skip"); ?>
--FILE--
<?php
use Oqs\Kem;
use Oqs\Sig;

$kems = Kem::algorithms();
$sigs = Sig::algorithms();
echo (is_array($kems) && is_array($sigs)) ? "algos\n" : "fail\n";

$kem = new Kem($kems[0]);
[$pk,$sk] = $kem->keypair();
[$ct,$ssA] = $kem->encap($pk);
$ssB = $kem->decap($ct, $sk);
echo ($ssA === $ssB) ? "kem-ok\n" : "kem-fail\n";

$sig = new Sig($sigs[0]);
[$spk,$ssk] = $sig->keypair();
$s = $sig->sign("hello", $ssk);
echo $sig->verify("hello", $s, $spk) ? "sig-ok\n" : "sig-fail\n";
?>
--EXPECTF--
algos
kem-ok
sig-ok
