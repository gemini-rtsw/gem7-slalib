#!/bin/bash
# Per-checkout bootstrap. Run once after cloning, then plain `make` forever:
#
#     ./gemini-rtsw-ci/dev_environment.sh --el 9
#     ./setup.sh && make
#
# The environment itself needs no user setup: the gem-epics3134gem7 RPM ships
# /etc/profile.d/gem7.sh and dev_environment.sh starts a login shell, so EPICS,
# HOST_ARCH and WIND_BASE are already set. This script only does the part that
# cannot be baked into an image -- applSetup.pl stamps the checkout's ABSOLUTE
# path into config/, so it must run where the checkout actually is.
#
# The spec's %build calls this same script, so an interactive build and a CI
# build run identical steps and cannot drift.
set -e
cd "$(cd "$(dirname "$0")" && pwd)"

# Fall back to the repo-local env if the RPM's profile script is absent (a
# bare container, or a tree mounted at the Solaris paths).
if [ -z "${EPICS_BASE:-}" ]; then
    if [ -f /etc/profile.d/gem7.sh ]; then . /etc/profile.d/gem7.sh
    else
        echo "ERROR: no EPICS environment. Install gem-epics3134gem7 or source" >&2
        echo "       its /etc/profile.d/gem7.sh before running this." >&2
        exit 1
    fi
fi

# applSetup PRESERVES APPLIC_INSTALL from an existing config/CONFIG.Defs, so a
# checkout bootstrapped in another container would silently keep a stale path.
rm -rf config bin lib include dbd data Distfile .applTop
find . -type d -name 'O.*' -prune -exec rm -rf {} + 2>/dev/null || true

echo "APPLIC_TOP = $PWD" > .applTop
perl "$EPICS_BASE/bin/$HOST_ARCH/applSetup.pl" -T ppc604 -I src -I startup


# Match production's debug format. gcc 2.7.2 emitted stabs by default; gcc
# 2.96 defaults to DWARF, so without this the objects carry .debug_info where
# every deployed GEM7 object carries .stab/.stabstr. Nothing at runtime reads
# either -- vxWorks ld loads .text/.data/.bss and ignores debug sections, and
# .symtab (which lkup and stack traces use) is present either way -- but the
# Tornado 2.0-era debugger reads stabs, so DWARF would cost source lines and
# locals in any future debugging session against a crate.
#
# DEBUG_CFLAGS is in the CFLAGS chain in CONFIG_COMMON and assigned nowhere,
# so it is a free hook. It goes in the generated config rather than on a make
# command line so that a developer's plain `make` gets it too -- and because a
# command-line assignment would override, not append to, USR_CFLAGS.
echo "DEBUG_CFLAGS = -gstabs" >> config/CONFIG.Defs

for f in config/CONFIG config/CONFIG.Defs; do
    [ -f "$f" ] || { echo "ERROR: applSetup.pl did not produce $f" >&2; exit 1; }
done
echo
echo "Setup complete -- run 'make'."
