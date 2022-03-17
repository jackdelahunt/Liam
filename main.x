@main
    push 0
    store 0

#loop
    load 0
    push 1
    add
    print
    store 0
    goto #loop