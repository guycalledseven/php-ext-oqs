--TEST--
OQS version constants are defined and have correct types
--SKIPIF--
<?php if (!extension_loaded('oqs')) die("skip"); ?>
--FILE--
<?php

use const Oqs\VERSION_TEXT;
use const Oqs\VERSION_MAJOR;
use const Oqs\VERSION_MINOR;
use const Oqs\VERSION_PATCH;

// 1. Test that constants are defined
echo "VERSION_TEXT defined: " . (defined('Oqs\VERSION_TEXT') ? 'Yes' : 'No') . "\n";
echo "VERSION_MAJOR defined: " . (defined('Oqs\VERSION_MAJOR') ? 'Yes' : 'No') . "\n";
echo "VERSION_MINOR defined: " . (defined('Oqs\VERSION_MINOR') ? 'Yes' : 'No') . "\n";
echo "VERSION_PATCH defined: " . (defined('Oqs\VERSION_PATCH') ? 'Yes' : 'No') . "\n";

echo "---\n";

// 2. Test that constants have the correct types
echo "Type of VERSION_TEXT: " . gettype(VERSION_TEXT) . "\n";
echo "Type of VERSION_MAJOR: " . gettype(VERSION_MAJOR) . "\n";
echo "Type of VERSION_MINOR: " . gettype(VERSION_MINOR) . "\n";
echo "Type of VERSION_PATCH: " . gettype(VERSION_PATCH) . "\n";

echo "---\n";

// 3. Test for reasonable values (not empty or negative)
echo "VERSION_TEXT is not empty: " . (strlen(VERSION_TEXT) > 0 ? 'Yes' : 'No') . "\n";
echo "VERSION_MAJOR is non-negative: " . (VERSION_MAJOR >= 0 ? 'Yes' : 'No') . "\n";

// 4. Print the actual version for context in the test output
echo "Reported version: " . VERSION_TEXT . "\n";

?>
--EXPECTF--
VERSION_TEXT defined: Yes
VERSION_MAJOR defined: Yes
VERSION_MINOR defined: Yes
VERSION_PATCH defined: Yes
---
Type of VERSION_TEXT: string
Type of VERSION_MAJOR: integer
Type of VERSION_MINOR: integer
Type of VERSION_PATCH: integer
---
VERSION_TEXT is not empty: Yes
VERSION_MAJOR is non-negative: Yes
Reported version: %s