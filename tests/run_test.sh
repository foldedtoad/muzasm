#
rm -rf ./output

# Must use absolute path to asm file.
FILE="/home/robin/MUZ-Workshop/MUZ/muzasm_linux/tests/Instructions.asm"

# Invoke assembler...
../build/muzasm -l listing.txt -m memory.txt -h IntelHex.hex --trace --log log.txt -od output -f $FILE
