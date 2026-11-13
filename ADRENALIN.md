# FEX Adrenalin Edition

This fork retains the existing macOS host bridge and Apple Silicon translation changes on top of FEX-2610 (`14c92681f4d62cf84d901460e0358de09c8847a7`). It is the single FEX source checkout used by GooseHUB.

Keep this checkout beside `GooseHUB`, `wine-adrenalin-edition` and `dxmt-adrenalin-edition`. GooseHUB builds the ARM64EC and WoW64 translation modules and the Darwin Unix libraries from these sources. Build files go to `~/Library/Caches/GooseHUB/Build/FEX`; installed libraries go inside the app. No precompiled engine is distributed in this repository.

Upstream is [FEX-Emu/FEX](https://github.com/FEX-Emu/FEX). The upstream license, contribution instructions and acknowledgements are retained. See `LICENSE` and `CONTRIBUTING.md`.
