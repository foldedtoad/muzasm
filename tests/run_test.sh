#
rm -rf ./output

# Must use absolute path to asm file.
FILE="/home/robin/MUZ-Workshop/MUZ/DPasm/tests/Instructions.asm"

# Invoke assembler...
../build/DPasm -l listing.txt -m memory.txt -h IntelHex.hex -od output -f $FILE
