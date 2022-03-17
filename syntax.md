| Mnemoic | OPCode  | Arguments         | Stack [Before --> After] |
| ------- | ------- | ----------------- | ------------------------ |
| push    | 0       | value             | --> value                |
| pop     | 1       |                   | value -->                |
| print   | 2       |                   |                          |
| add     | 3       |                   | value, value --> result  |
| store   | 4       | register          | value -->                |
| load    | 5       | register          | --> value                |
| alloc   | 6       |                   | size --> ptr             |
| put     | 7       |                   | ptr, value -->           |
| get     | 8       |                   | ptr --> value            |
| ret     | 9       |                   |                          |
| pop_ret | 10      |                   | value -->                |
| call    | 11      | label, arg count  | [args] -->               |
| goto    | 12      | label             | -->                      |
