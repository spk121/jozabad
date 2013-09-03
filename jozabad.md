

Message formats
===============


X.121 Addresses
---------------

The original X.25 spec used X.121 addresses, which is as good a scheme as any, for now.

14-digit decimal number.

First 4 digits are a country code, aka Data Network Identification Code, or DNIC.
- Digit 1 is 2..7
- Digit 2 to 4 is 0..9

Next 4 are sub-country grouping.

Remaining 6 digits are the local network number.
