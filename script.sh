
if [ "$#" -ne 1 ]; then
    echo "Mod de utilizare: $0 <c>"
    exit 1
fi


character="$1"
count=0
regex="^[[:upper:]][[:alnum:][:space:],.!?]*[.!?]$"


while IFS= read -r line; do
  if [[ $line =~ $regex && ! $line =~ ,\ *și ]]; then
    if [[ $line =~ $character ]]; then
      ((count++))
    fi
  fi
done

echo "Numărul de propoziții corecte care conțin caracterul $character: $count"
