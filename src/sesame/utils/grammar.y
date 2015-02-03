// Copyright (c) 2015, Karsten Heinze <karsten.heinze@sidenotes.de>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


/*
 * The command line parser is used for parsing of (partial) command lines.
 * Rules ending with NEWLINE are actual commands. Other rules are used
 * for auto completion. Parse results belonging to partial command lines
 * have to be invalidated to trigger a syntax error when applied outside
 * the auto completion context.
 */
%include {
#include <memory>
#include <iostream>
#include "types.hpp"
#include "sesame/commands/ICommand.hpp"
#include "sesame/commands/InstanceTask.hpp"
#include "sesame/commands/EntryTask.hpp"
#include "sesame/utils/ParseResult.hpp"

using sesame::commands::ICommand;
using sesame::commands::EntryTask;
using sesame::commands::InstanceTask;
}
%token_type{ char* }
%extra_argument{ sesame::utils::ParseResult* parseResult }
%syntax_error{
   parseResult->invalidate();
}
%parse_accept{
}
%parse_failure{
   parseResult->invalidate();
}

cmd_line ::= START.                         { parseResult->invalidate(); parseResult->setCompleteCommand(); }
cmd_line ::= TECLA_EDIT_MODE.               { parseResult->invalidate(); parseResult->setCompleteSpace(); }
cmd_line ::= TECLA_EDIT_MODE WHITESPACE.    { parseResult->invalidate(); parseResult->setCompleteEditMode(); }


cmd_line ::= NEWLINE.
cmd_line ::= HELP(C) NEWLINE.
{
    parseResult->addToken( C );
}
cmd_line ::= NEW(C) NEWLINE.
{
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new InstanceTask( InstanceTask::NEW ) ) );
}
cmd_line ::= OPEN.             { parseResult->invalidate(); parseResult->setCompleteSpace(); }
cmd_line ::= OPEN WHITESPACE.  { parseResult->invalidate(); parseResult->setCompleteFile(); }
cmd_line ::= OPEN(C) WHITESPACE ARGUMENT(A) NEWLINE.
{
    parseResult->addToken( A );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new InstanceTask( InstanceTask::OPEN, A ) ) );
}
cmd_line ::= CLOSE(C) NEWLINE.
{
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new InstanceTask( InstanceTask::CLOSE ) ) );
}
cmd_line ::= LIST(C) NEWLINE.
{
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::LIST ) ) );
}
cmd_line ::= SEARCH(C) WHITESPACE arguments NEWLINE.
{
    parseResult->addToken( C );
}
cmd_line ::= SHOW.             { parseResult->invalidate(); parseResult->setCompleteSpace(); }
cmd_line ::= SHOW WHITESPACE.  { parseResult->invalidate(); parseResult->setCompleteEntry(); }
cmd_line ::= SHOW(C) WHITESPACE OTHER_ID(ID) NEWLINE.
{
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::SHOW, ID ) ) );
}
cmd_line ::= DECRYPT.               { parseResult->invalidate(); parseResult->setCompleteSpace(); }
cmd_line ::= DECRYPT WHITESPACE.    { parseResult->invalidate(); parseResult->setCompleteEntry(); }
cmd_line ::= DECRYPT(C) WHITESPACE OTHER_ID(ID) NEWLINE.
{
    parseResult->addToken( ID );
    parseResult->addToken( C );
}
cmd_line ::= ADD(C) NEWLINE.
{
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::ADD ) ) );
}
cmd_line ::= DELETE.                { parseResult->invalidate(); parseResult->setCompleteSpace(); }
cmd_line ::= DELETE WHITESPACE.     { parseResult->invalidate(); parseResult->setCompleteEntry(); }
cmd_line ::= DELETE(C) WHITESPACE OTHER_ID(ID) NEWLINE.
{
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::DELETE, ID ) ) );
}
cmd_line ::= UPDATE.                         { parseResult->invalidate(); parseResult->setCompleteSpace(); }
cmd_line ::= UPDATE WHITESPACE.              { parseResult->invalidate(); parseResult->setCompleteEntry(); }
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) NEWLINE.
{
    parseResult->addToken( ID );
    parseResult->addToken( C );
}
cmd_line ::= UPDATE WHITESPACE ENTRY_ID WHITESPACE. { parseResult->invalidate(); parseResult->setCompleteSubCommand(); }
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE ADD_ATTRIBUTE(A) NEWLINE.
{
    parseResult->addToken( A );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::ADD_ATTRIBUTE, ID ) ) );
}
cmd_line ::= UPDATE WHITESPACE ENTRY_ID(ID) WHITESPACE DELETE_ATTRIBUTE WHITESPACE.
{
   parseResult->invalidate();
   parseResult->setEntryId( ID );
   parseResult->setCompleteAttribute();
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE DELETE_ATTRIBUTE(D) WHITESPACE OTHER_ID(ID2) NEWLINE.
{
    parseResult->addToken( ID2 );
    parseResult->addToken( D );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::DELETE_ATTRIBUTE, ID, ID2 ) ) );
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE UPDATE_ATTRIBUTE(U) WHITESPACE OTHER_ID(ID2) NEWLINE.
{
    parseResult->addToken( ID2 );
    parseResult->addToken( U );
    parseResult->addToken( ID );
    parseResult->addToken( C );
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE ADD_TEXT(A) NEWLINE.
{
    parseResult->addToken( A );
    parseResult->addToken( ID );
    parseResult->addToken( C );
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE UPDATE_TEXT(U) WHITESPACE OTHER_ID(ID2) NEWLINE.
{
    parseResult->addToken( ID2 );
    parseResult->addToken( U );
    parseResult->addToken( ID );
    parseResult->addToken( C );
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE DELETE_TEXT(D) WHITESPACE OTHER_ID(ID2) NEWLINE.
{
    parseResult->addToken( ID2 );
    parseResult->addToken( D );
    parseResult->addToken( ID );
    parseResult->addToken( C );
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE ADD_DATA(A) NEWLINE.
{
    parseResult->addToken( A );
    parseResult->addToken( ID );
    parseResult->addToken( C );
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE UPDATE_DATA(U) WHITESPACE OTHER_ID(ID2) NEWLINE.
{
    parseResult->addToken( ID2 );
    parseResult->addToken( U );
    parseResult->addToken( ID );
    parseResult->addToken( C );
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE DELETE_DATA(D) WHITESPACE OTHER_ID(ID2) NEWLINE.
{
    parseResult->addToken( ID2 );
    parseResult->addToken( D );
    parseResult->addToken( ID );
    parseResult->addToken( C );
}
cmd_line ::= WRITE.                    { parseResult->invalidate(); parseResult->setCompleteCommand(); }
cmd_line ::= WRITE WHITESPACE.         { parseResult->invalidate(); parseResult->setCompleteFile(); }
cmd_line ::= WRITE(C) WHITESPACE ARGUMENT(A) NEWLINE.
{
    parseResult->addToken( A );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new InstanceTask( InstanceTask::WRITE, A ) ) );
}
arguments ::= ARGUMENT(A).
{
    parseResult->addToken( A );
}
arguments ::= ARGUMENT(A) WHITESPACE arguments.
{
    parseResult->addToken( A );
}
