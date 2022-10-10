    IMPORT program_start

    SECTION start

    lds #$0C00
    ldy #$0000
    ldu #$0000
    jmp program_start

    ENDSECTION
