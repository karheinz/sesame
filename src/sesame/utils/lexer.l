%{
/*
 * Copyright (c) 2015, Karsten Heinze <karsten.heinze@sidenotes.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "sesame/utils/grammar.h"
%}

    /* options */
%option reentrant
%option noyywrap

    /*
        Inclusive start conditions are declared with %s.
        Meaning rules without start condition are considered.
        Exclusive start conditions are declared with %x.
        Meaning rules without start condition are not considered.
    */
%s START_COND
%s CMD_COND
%s ENTRY_COND
%s UPDATE_COND
%s UPDATE_ENTRY_COND
%s UPDATE_OTHER_COND

    /* aliases */
WS  [ \t]
CH  [^ \t\r\n]
DG  [0-9]
HX  [0-9a-fA-F]
NL  (\r?\n)
%%

<INITIAL>{CH}+                            { BEGIN( START_COND ); yyless( 0 ); }
<START_COND>edit-mode                     { BEGIN( CMD_COND ); return TECLA_EDIT_MODE; }
<START_COND>help                          { BEGIN( CMD_COND ); return HELP; }
<START_COND>new                           { BEGIN( CMD_COND ); return NEW; }
<START_COND>open                          { BEGIN( CMD_COND ); return OPEN; }
<START_COND>close                         { BEGIN( CMD_COND ); return CLOSE; }
<START_COND>list                          { BEGIN( CMD_COND ); return LIST; }
<START_COND>search                        { BEGIN( CMD_COND ); return SEARCH; }
<START_COND>show                          { BEGIN( ENTRY_COND ); return SHOW; }
<START_COND>decrypt                       { BEGIN( ENTRY_COND ); return DECRYPT; }
<START_COND>add                           { BEGIN( CMD_COND ); return ADD; }
<START_COND>delete                        { BEGIN( ENTRY_COND ); return DELETE; }
<START_COND>update                        { BEGIN( UPDATE_COND ); return UPDATE; }
<START_COND>write                         { BEGIN( CMD_COND ); return WRITE; }
<START_COND>{CH}+                         { return START; }
<UPDATE_COND>#{HX}+                       { BEGIN( UPDATE_ENTRY_COND ); return ENTRY_ID; }
<UPDATE_ENTRY_COND>add{WS}+text           { BEGIN( CMD_COND ); return ADD_TEXT; }
<UPDATE_ENTRY_COND>update{WS}+text        { BEGIN( UPDATE_OTHER_COND ); return UPDATE_TEXT; }
<UPDATE_ENTRY_COND>delete{WS}+text        { BEGIN( UPDATE_OTHER_COND ); return DELETE_TEXT; }
<UPDATE_ENTRY_COND>add{WS}+data           { BEGIN( CMD_COND ); return ADD_DATA; }
<UPDATE_ENTRY_COND>update{WS}+data        { BEGIN( UPDATE_OTHER_COND ); return UPDATE_DATA; }
<UPDATE_ENTRY_COND>delete{WS}+data        { BEGIN( UPDATE_OTHER_COND ); return DELETE_DATA; }
<UPDATE_ENTRY_COND>add{WS}+attribute      { BEGIN( CMD_COND ); return ADD_ATTRIBUTE; }
<UPDATE_ENTRY_COND>update{WS}+attribute   { BEGIN( UPDATE_OTHER_COND ); return UPDATE_ATTRIBUTE; }
<UPDATE_ENTRY_COND>delete{WS}+attribute   { BEGIN( UPDATE_OTHER_COND ); return DELETE_ATTRIBUTE; }
<ENTRY_COND,UPDATE_OTHER_COND>#{HX}+      { BEGIN( CMD_COND ); return OTHER_ID; }
^{WS}*                                    /* ignore */
{WS}*{NL}                                 { BEGIN( INITIAL ); return NEWLINE; }
{WS}+                                     { return WHITESPACE; }
{CH}+                                     { return ARGUMENT; }
