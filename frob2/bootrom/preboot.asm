    IMPORT program_start

    SECTION start

    lds #$2000
    ldy #$0000
    ldu #$0000
    pshs y,u
    pshs y,u
    jmp _main

    ENDSECTION
