
if [ "$#" -ne 1 ]; then
    echo "Mod de utilizare: $0 <c>"
    exit 1
fi


character="$1"


count=0


while IFS= read -r line; do
    
    if [[ $line =~ ^[A-Z].*[a-zA-Z0-9\ ,.!?]$]] && [[ $line !=~ ,.*[ \t]*[ \t]*$]] && [[$line =~ [\.!?]$ ]]; then
        ((count++))
    fi
done


echo "Numărul de propoziții corecte care conțin caracterul $character: $count"
