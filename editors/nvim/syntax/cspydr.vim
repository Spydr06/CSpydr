syn match cspydrComment "#.*$"
syntax region cspydrComment start="#\[" end="]#"
hi def link cspydrComment Comment

syn match cspydrCArrayModifier "'[cC]"
hi def link cspydrCArrayModifier StorageClass

" true, false, nil
syn keyword cspydrLiteralKeyword true false nil
hi def link cspydrLiteralKeyword Boolean

" statements
syn keyword cspydrConditionalKeyword defer do else if match unless with ret
hi def link cspydrConditionalKeyword Conditional

syn match cspydrArrowReturn "<-"
hi def link cspydrArrowReturn Conditional

syn keyword cspydrLoopKeyword for while loop
hi def link cspydrLoopKeyword Repeat

syn keyword cspydrLabelKeyword break continue
hi def link cspydrLabelKeyword Label

syn match cspydrEmptyOption "_\ze[^a-zA-Z0-9?'_]"
hi def link cspydrEmptyOption Label

syn keyword cspydrOperatorKeyword alignof sizeof typeof len
hi def link cspydrOperatorKeyword Operator

syn match cspydrOperators "[!+^\-*/=%&|<>²³@$:,.;;]"
hi def link cspydrOperators Operator

syn match cspydrParens "[\[\](){}]"
hi def link cspydrParens Ignore

syn keyword cspydrOtherKeyword asm using noop
hi def link cspydrOtherKeyword Keyword

" top-level keywords
syn keyword cspydrTopLevelKeyword fn interface namespace type operator
hi def link cspydrTopLevelKeyword Keyword

" macros
syn match cspydrMacroIdent "[a-zA-Z_][a-zA-Z0-9?'_]*!"
hi def link cspydrMacroIdent Macro

syn keyword cspydrPreprocessorKeyword macro import
hi def link cspydrPreprocessorKeyword PreProc

" default types
syn keyword cspydrPrimitiveType void bool char i8 u8 i16 u16 i32 u32 i64 u64 f32 f64 f80
hi def link cspydrPrimitiveType Type

syn keyword cspydrStructure enum interface struct union
hi def link cspydrStructure Structure

syn keyword cspydrTypeModifier dyn const embed extern let
hi def link cspydrTypeModifier Statement

syn match cspydrTypedef "[A-Z][a-zA-Z0-9?'_]*"
hi def link cspydrTypedef Typedef

" function calls
syn match cspydrFnIdent "[a-zA-Z_][a-zA-Z0-9_]*\s*\ze("
hi def link cspydrFnIdent Function

" number literals:
syn match cspydrNumber "[0-9][0-9_]*"
hi def link cspydrNumber Number

syn match cspydrHexNumber "0[xX][0-9a-fA-F][0-9a-fA-F_]*"
hi def link cspydrHexNumber Number

syn match cspydrBinNumber "0[bB][01][01_]*"
hi def link cspydrBinNumber Number

syn match cspydrOctNumber "0[oO][0-7][0-7_]*"
hi def link cspydrOctNumber Number

syn match cspydrFltNumber "[0-9][0-9_]*\.[0-9_][0-9_]*"
hi def link cspydrFltNumber Float

" strings
syn match cspydrEscapedChar "'\\.'" contained
hi def link cspydrEscapedChar SpecialChar

syn region cspydrString start=+"+ end=+"+ skip=+\\"+ contains=cspydrEscapedChar
hi def link cspydrString String

syn match cspydrChar "'.'"
syn match cspydrChar "'\\.'"
hi def link cspydrChar Character

