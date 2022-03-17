| Mnemoic | OPCode  | Arguments         | Stack [Before --> After] |
| ------- | ------- | ----------------- | ------------------------ |
| push    | 0       | value             | --> value                |
| print   | 1       |                   |                          |
| add     | 2       |                   | value, value --> result  |
| store   | 3       | register          | value -->                |
| load    | 4       | register          | --> value                |
| alloc   | 5       |                   | size --> ptr             |
| put     | 6       |                   | ptr, value -->           |
| get     | 7       |                   | ptr --> value            |
| ret     | 8       |                   |                          |
| call    | 9       | byte code address |                          |
