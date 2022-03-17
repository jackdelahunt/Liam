@print
    push 1
    push 2
    add
    print
    pop
    ret

@main
    call @print
    push 1
    push 10
    add
    print
    ret