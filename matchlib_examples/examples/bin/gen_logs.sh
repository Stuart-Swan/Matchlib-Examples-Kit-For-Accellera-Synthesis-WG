#!/bin/bash

basename=${1:-"channel_logs"}

if [ ! -d "${basename}" ]; then
  mkdir "${basename}"
fi

# Pre-create empty files for all IDs
while IFS=" " read -r var1 var2 ; do
  echo Processing $var2
  touch "${basename}/$var2.txt"
  touch "${basename}/$var2.txt.times"
done < ${basename}_names.txt

# Use awk for efficient single-pass processing
awk -v basename="${basename}" '
BEGIN {
  # Read the names file and build ID->filename mapping
  while ((getline < (basename "_names.txt")) > 0) {
    id_to_filename[$1] = $2
  }
  close(basename "_names.txt")

  FS = "|"
}

{
  # Process data file
  # Extract ID from first field and trim whitespace
  gsub(/^[ \t]+|[ \t]+$/, "", $1)
  id = $1

  # Only process if we have a filename for this ID
  if (id in id_to_filename) {
    filename = id_to_filename[id]

    # Extract data field (second field) - trim leading space and remove exactly 1 trailing space
    data = $2
    gsub(/^[ \t]+/, "", data)  # trim leading
    gsub(/[ \t]$/, "", data)    # trim exactly 1 trailing space

    # Extract and trim time field (third field)
    time = $3
    gsub(/^[ \t]+|[ \t]+$/, "", time)

    # Append to appropriate files
    print data >> basename "/" filename ".txt"
    print time >> basename "/" filename ".txt.times"
  }
}
' ${basename}_data.txt

echo $0: Done
