#!/bin/bash
SDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

kitty -c $SDIR/kitty.conf /opt/homebrew/bin/zsh --interactive -c "docker run -ti -v '$SDIR/..:/repo' lkp-aarch64 /bin/bash"

