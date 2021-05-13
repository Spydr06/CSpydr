	.text
	.file	"main"
	.globl	_start                          # -- Begin function _start
	.p2align	4, 0x90
	.type	_start,@function
_start:                                 # @_start
	.cfi_startproc
# %bb.0:                                # %entry
	retq
.Lfunc_end0:
	.size	_start, .Lfunc_end0-_start
	.cfi_endproc
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
