@add
    load 0
    load 1
    add
    print
    ret

@main
    push 10
    store 0 
    call @add
    ret