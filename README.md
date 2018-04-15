# Stack CPU

Stack CPU Simulator

## Opcode List

```text
LIT - push memory to data stack
@ - pop data stack and load memory into data stack
! - store data from stack into memory
DROP - pop data stack
DUP - duplicate top of data stack
OVER - duplicate second of top of data stack
SWAP - swap top data stack
+ - add
- - minus
AND - and
OR - or
XOR - xor
IF - if top stack is 0, use next pc to jump
CALL - jump pc
EXIT - pop return stack then jump pc to poped data
HALT - end program
>R - pop data stack then push to return stack
R> - pop return stack then push to data stack
```

## License

GPL-3.0
