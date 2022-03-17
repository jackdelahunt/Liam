@add
    load 0
    load 1
    add
    pop_ret

@main
    push 1
    push 2
    call @add 2
    print
    pop
    ret
    