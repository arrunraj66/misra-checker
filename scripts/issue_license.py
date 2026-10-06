#!/usr/bin/env python3
"""Vendor-side tool: issue a signed license file.

Usage: issue_license.py --customer ACME --expires 2027-12-31 \
           --features convert --key-file vendor.key > acme.lic
The key must equal the MISRA_LICENSE_HMAC_KEY the product was built with.
Keep the key out of the repository.
"""
import argparse
import hashlib
import hmac

p = argparse.ArgumentParser()
p.add_argument("--customer", required=True)
p.add_argument("--expires", required=True, help="YYYY-MM-DD")
p.add_argument("--features", default="convert")
p.add_argument("--key-file", required=True)
a = p.parse_args()
key = open(a.key_file, "rb").read().strip()
canonical = f"customer={a.customer}\nexpires={a.expires}\nfeatures={a.features}\n"
sig = hmac.new(key, canonical.encode(), hashlib.sha256).hexdigest()
print(canonical + f"signature={sig}")
