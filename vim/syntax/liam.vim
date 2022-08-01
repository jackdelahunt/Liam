" Vim syntax file
" Language: Liam
" Latest Revision: 2022-05-25

if exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

let s:liam_syntax_keywords = {
    \   'liamConditional' :["if"
    \ ,                     "else"
    \ ,                    ]
    \ , 'liamRepeat' :["while"
    \ ,                "for"
    \ ,               ]
    \ , 'liamExecution' :["return"
    \ ,                  ]
    \ , 'liamBoolean' :["true"
    \ ,                 "false"
    \ ,                ]
    \ , 'liamKeyword' :["fn"
    \ ,                 "extern"
    \ ,                 "import"
    \ ,                 "insert"
    \ ,                 "continue"
    \ ,                ]
    \ , 'liamMacro' :["is"
    \ ,               "new"
    \ ,              ]
    \ , 'liamWordOperator' :["and"
    \ ,                      "or"
    \ ,                     ]
    \ , 'liamVarDecl' :["let"
    \ ,                ]
    \ , 'liamType' :["str"
    \ ,              "s8"
    \ ,              "s16"
    \ ,              "s32"
    \ ,              "s64"
    \ ,              "u8"
    \ ,              "u16"
    \ ,              "u32"
    \ ,              "u64"
    \ ,              "f32"
    \ ,              "f64"
    \ ,              "bool"
    \ ,              "void"
    \ ,             ]
    \ , 'liamConstant' :["this"
    \ ,                 ]
    \ , 'liamStructure' :["struct"
    \ ,                   "enum"
    \ ,                  ]
    \ , }

function! s:syntax_keyword(dict)
  for key in keys(a:dict)
    execute 'syntax keyword' key join(a:dict[key], ' ')
  endfor
endfunction

call s:syntax_keyword(s:liam_syntax_keywords)

syntax match liamDecNumber display   "\v<\d%(_?\d)*"

syntax match liamOperator display "\V\[-+*=^!><:;,@/%]"

syntax match liamFunction /\w\+\s*(/me=e-1,he=e-1
syntax match liamFunction /\w\+\s*\[/me=e-1,he=e-1

syntax match liamStructDecl /struct\s\+\w\+/lc=6
syntax match liamEnumDecl /enum\s\+\w\+/lc=4
syntax region liamBlock start="{" end="}" transparent fold

syntax region liamCommentLine start="#" end="$"



syntax region liamString matchgroup=liamStringDelimiter start=+"+ skip=+\\\\\|\\"+ end=+"+ oneline contains=liamEscape
syntax region liamChar matchgroup=liamCharDelimiter start=+'+ skip=+\\\\\|\\'+ end=+'+ oneline contains=liamEscape
syntax match liamEscape        display contained /\\./

highlight default link liamDecNumber liamNumber
highlight default link liamHexNumber liamNumber
highlight default link liamOctNumber liamNumber
highlight default link liamBinNumber liamNumber

highlight default link liamWordOperator liamOperator
highlight default link liamFatArrowOperator liamOperator
highlight default link liamRangeOperator liamOperator

highlight default link liamStructDecl liamType
highlight default link liamEnumDecl liamType

highlight default link liamKeyword Keyword
highlight default link liamType Type
highlight default link liamCommentLine Comment
highlight default link liamString String
highlight default link liamStringDelimiter String
highlight default link liamChar String
highlight default link liamCharDelimiter String
highlight default link liamEscape Special
highlight default link liamBoolean Boolean
highlight default link liamConstant Constant
highlight default link liamNumber Number
highlight default link liamOperator Operator
highlight default link liamStructure Structure
highlight default link liamExecution Special
highlight default link liamMacro Macro
highlight default link liamConditional Conditional
highlight default link liamRepeat Repeat
highlight default link liamVarDecl Define
highlight default link liamException Exception
highlight default link liamFunction Function
highlight default link liamVisModifier Label

delfunction s:syntax_keyword

let b:current_syntax = "liam"

let &cpo = s:cpo_save
unlet! s:cpo_save
