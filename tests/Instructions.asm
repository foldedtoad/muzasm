

.DATA , SAVE
.ORG 8100h
SomeData: .DB "This is some data I want in my program."

.CODE
.ORG 8000h
JR NotSoFar
.DS 0x20
NotSoFar:
LD HL,-8000  ;00C7: 21 C0 E1      0034              LD   HL,-8000       ;Set delay time
LD DE, SomeData
ADC A, IXh
ADC IXL
adc a, 8

#define bc "AB" + 1 ; "BC"
#define ascii66 "B" + 1 ; "C"
#define b1 "B" + "1" ; "B1"

.DATA

; all EQU styles should bring the same result
Label1: .EQU 1
Label2 .EQU 2
Label3: EQU 3
Label4 EQU 4

; test single character strings
achar equ "A"+1 ; should bring 66 and not "A1"

; test other strings
astring equ "AB" + 1 ; should bring 0 because "BC" "not a number

#DEFINE string "this is a string with embedded \" escaped double quote"
 .DB string


kWrongHex: .EQU 0x0B

kNull: .EQU 0
kNewLine: .EQU 13
szDevices:  .DB  "Devices detected:",kNewLine,kNull

 kVersMajor: .EQU 1
.ORG 2000h
.DB  '0'+kVersMajor,'.'

.DB  '0','.'
store: .DW 0

.CODE
LD (store), HL
