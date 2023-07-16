syn match cspydrComment "#.*$"
hi def link cspydrComment Comment

syn region cspydrMultilineComment start="#\[" end="#\]"
hi def link cspydrMultilineComment Comment

syn match cspydrCArrayModifier "'[cC]"
hi def link cspydrTypeModifier StorageClass

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

syn match cspydrEmptyOption "_[^a-zA-Z0-9_]"
hi def link cspydrEmptyOption Label

syn keyword cspydrOperatorKeyword alignof sizeof typeof len
hi def link cspydrOperatorKeyword Operator

syn match cspydrOperators "[!+^\-*/=%&|<>²³@$:,.;;]"
hi def link cspydrOperators Operator

syn match cspydrParens "[\[\](){}]"
hi def link cspydrParens Ignore

syn keyword cspydrOtherKeyword asm using noop
hi def link cspydrOtherKeyword Keyword

"top-level keywords
syn keyword cspydrTopLevelKeyword fn interface namespace type
hi def link cspydrTopLevelKeyword Keyword

" macros
syn match cspydrMacroIdent "[a-zA-Z_][a-zA-Z0-9_]*!"
hi def link cspydrMacroIdent Macro

syn keyword cspydrPreprocessorKeyword macro import
hi def link cspydrPreprocessorKeyword PreProc

" constants
syn match cspydrConstIdent "[A-Z_][A-Z0-9_]*"
hi def link cspydrConstIdent Constant

" default types
syn keyword cspydrPrimitiveType void i8 u8 i16 u16 i32 u32 i64 u64 bool f32 f64 f80
hi def link cspydrPrimitiveType Type

syn keyword cspydrStructure enum interface struct union
hi def link cspydrStructure Structure

syn keyword cspydrTypeModifier const embed extern let
hi def link cspydrTypeModifier StorageClass

syn match cspydrTypedef "[A-Z][a-zA-Z0-9_]*"
hi def link cspydrTypedef Typedef

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
syn region cspydrString start=+"+ end=+"+ skip=+\\"+
hi def link cspydrString String

syn match cspydrChar "'.'"
hi def link cspydrChar Character

syn match cspydrEscapedCHar "'\\.'"
hi def link cspydrEscapedCHar Character

syn match cspydrFnIdent "[a-zA-Z_][a-zA-Z0-9_]*("
hi def link cspudrFnIdent Function