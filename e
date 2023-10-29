#!/bin/bash
[[ -z "${EDITOR}" ]] && editor="nano" || editor="${EDITOR}"
find ./src -name $1 -exec $editor {} +
