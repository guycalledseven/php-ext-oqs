--TEST--
SIG negative verify and SLH-DSA smoke
--SKIPIF--
<?php if (!extension_loaded('oqs')) die("skip"); ?>
--FILE--
<?php
use Oqs\Sig;

$algs = Sig::algorithms();
echo "count=".count($algs)."\n";

$sig = new Sig($algs[0]); // e.g., ML-DSA-65
[$pk,$sk] = $sig->keypair();
$s1 = $sig->sign("hello", $sk);
var_dump($sig->verify("hello", $s1, $pk));   // true
var_dump($sig->verify("bye",   $s1, $pk));   // false

// Try an SLH-DSA variant if present
$slh = null;
foreach ($algs as $a) if (str_starts_with($a, "SLH-DSA")) { $slh = $a; break; }
if ($slh) {
    $x = new Sig($slh);
    [$pk2,$sk2] = $x->keypair();
    $s2 = $x->sign("data", $sk2);
    var_dump($x->verify("data", $s2, $pk2)); // true
    echo "slh-ok\n";
} else {
    echo "no-slh\n";
}
?>
--EXPECTF--
count=%d
bool(true)
bool(false)
%r(slh-ok|no-slh)\s*%r