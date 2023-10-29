#!/bin/bash
if [ "$1" = "-inc" ]; then
	find ./src/c/include/ -name "*.h" -print0 | xargs -0 grep --color $2
	exit
fi

find ./src/c/ -name      "*.[ch]" -print0 | xargs -0 grep --color "$1"
find ./src/website -name "*.js"   -print0 | xargs -0 grep --color "$1"
find ./src/website -name "*.html" -print0 | xargs -0 grep --color "$1"
find ./src/website -name "*.css"  -print0 | xargs -0 grep --color "$1"
