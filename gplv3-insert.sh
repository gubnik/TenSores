#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <project_name> <brief_description> <author_name>"
    exit 1
fi

# Assign command-line arguments to variables
PROJECT_NAME="$1"
BRIEF_DESCRIPTION="$2"
AUTHOR_NAME="$3"

# Define the GPL v3 notice with variables
GPL_NOTICE=$(cat <<EOF
/*
    $PROJECT_NAME, $BRIEF_DESCRIPTION
    Copyright (C) $(date +%Y), $AUTHOR_NAME

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

EOF
)

for ext in "java" "c" "cpp" "h" "hpp"; do
    find . -type f -name "*.$ext" | while read -r file; do
        if ! grep -q "GNU General Public License" "$file"; then
            echo -e "$GPL_NOTICE\n$(cat "$file")" > "$file"
            echo "Inserted GPL notice into $file"
        else
            echo "GPL notice already present in $file"
        fi
    done
done
