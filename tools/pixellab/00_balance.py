#!/usr/bin/env python3
# Key'i dogrula + kredi/abonelik durumunu goster.
#   source tools/pixellab/secrets.sh && python3 tools/pixellab/00_balance.py
import lib, json
b = lib.get("/balance")
print(json.dumps(b, indent=2))
