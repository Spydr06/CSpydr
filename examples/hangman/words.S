    .global words_addr
    .global words_size_addr
    .section .rodata
words_addr:
    .incbin "words.txt"
words_size_addr:
    .int words_size_addr - words_addr
