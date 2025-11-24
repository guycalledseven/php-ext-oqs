--TEST--
Check RNG switching
--FILE--
<?php
use Oqs\Kem;

// Try switching to system (should always work)
try {
    Oqs\randombytes_switch_algorithm(Oqs\RAND_ALG_SYSTEM);
    echo "Switched to system\n";
} catch (Oqs\Exception $e) {
    echo "Failed to switch to system: " . $e->getMessage() . "\n";
}

// Try switching to OpenSSL (might fail if liboqs built without OpenSSL)
try {
    Oqs\randombytes_switch_algorithm(Oqs\RAND_ALG_OPENSSL);
    echo "Switched to OpenSSL\n";
} catch (Oqs\Exception $e) {
    // If it fails, we just print a message that we can match in EXPECTF or just ignore
    // But to make test pass on both, we can print "Switched to OpenSSL" if we want to fake it, 
    // OR better: print "OpenSSL attempt done" and rely on the fact that we caught the exception.
    // Let's just print "Switched to OpenSSL" if successful, or "OpenSSL not available" if failed.
    echo "OpenSSL not available\n";
}

// Switch back to system
try {
    Oqs\randombytes_switch_algorithm(Oqs\RAND_ALG_SYSTEM);
    echo "Switched back to system\n";
} catch (Oqs\Exception $e) {
    echo "Failed to switch back to system: " . $e->getMessage() . "\n";
}
?>
--EXPECTF--
Switched to system
%s
Switched back to system
