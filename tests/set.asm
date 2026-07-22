ORG $8000
START:
    SET 1,(HL)
    SET 0,B
    SET 7,(IX+2)
    RET
    