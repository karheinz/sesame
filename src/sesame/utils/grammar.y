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
#include "sesame/commands/ApgTask.hpp"
#include "sesame/commands/EntryTask.hpp"
#include "sesame/commands/HelpTask.hpp"
#include "sesame/commands/InstanceTask.hpp"
#include "sesame/utils/ParseResult.hpp"

using sesame::commands::ICommand;
using sesame::commands::ApgTask;
using sesame::commands::EntryTask;
using sesame::commands::HelpTask;
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

cmd_line ::= START.                         { parseResult->setCompleteCommand(); }
cmd_line ::= TECLA_EDIT_MODE.               { parseResult->setCompleteSpace(); }
cmd_line ::= TECLA_EDIT_MODE WHITESPACE.    { parseResult->setCompleteEditMode(); }


cmd_line ::= NEWLINE.
cmd_line ::= HELP(C) NEWLINE.
{
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new HelpTask( HelpTask::HELP ) ) );
}
cmd_line ::= NEW(C) NEWLINE.
{
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new InstanceTask( InstanceTask::NEW ) ) );
}
cmd_line ::= OPEN.             { parseResult->setCompleteSpace(); }
cmd_line ::= OPEN WHITESPACE.  { parseResult->setCompleteFile(); }
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
cmd_line ::= LIST.             { parseResult->setCompleteSpace(); }
cmd_line ::= LIST(C) NEWLINE.
{
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::LIST ) ) );
}
cmd_line ::= LIST WHITESPACE.  { parseResult->setCompleteTag(); }
cmd_line ::= LIST(C) WHITESPACE OTHER_ID(ID) NEWLINE.
{
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::LIST, ID ) ) );
}
cmd_line ::= TREE(C) NEWLINE.
{
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::TREE ) ) );
}
cmd_line ::= TAGS(C) NEWLINE.
{
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::TAGS ) ) );
}
cmd_line ::= SEARCH.             { parseResult->setCompleteSpace(); }
cmd_line ::= SEARCH WHITESPACE.
cmd_line ::= SEARCH(C) WHITESPACE ARGUMENT(A) NEWLINE.
{
    parseResult->addToken( A );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::SEARCH, A ) ) );
}
cmd_line ::= SHOW.             { parseResult->setCompleteSpace(); }
cmd_line ::= SHOW WHITESPACE.  { parseResult->setCompleteEntry(); }
cmd_line ::= SHOW(C) WHITESPACE OTHER_ID(ID) NEWLINE.
{
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::SHOW, ID ) ) );
}
cmd_line ::= DECRYPT.               { parseResult->setCompleteSpace(); }
cmd_line ::= DECRYPT WHITESPACE.    { parseResult->setCompleteEntry(); }
cmd_line ::= DECRYPT(C) WHITESPACE OTHER_ID(ID) NEWLINE.
{
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::DECRYPT, ID ) ) );
}
cmd_line ::= ADD(C) NEWLINE.
{
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::ADD ) ) );
}
cmd_line ::= DELETE.                { parseResult->setCompleteSpace(); }
cmd_line ::= DELETE WHITESPACE.     { parseResult->setCompleteEntry(); }
cmd_line ::= DELETE(C) WHITESPACE OTHER_ID(ID) NEWLINE.
{
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::DELETE, ID ) ) );
}
cmd_line ::= SELECT.                         { parseResult->setCompleteSpace(); }
cmd_line ::= SELECT WHITESPACE.              { parseResult->setCompleteEntry(); }
cmd_line ::= SELECT WHITESPACE ENTRY_ID WHITESPACE. { parseResult->setCompleteSelectCommand(); }
cmd_line ::= SELECT WHITESPACE ENTRY_ID(ID) WHITESPACE EXPORT_PASSWORD_OR_KEY WHITESPACE.
{
   parseResult->setEntryId( ID );
   parseResult->setCompletePasswordOrKey();
}
cmd_line ::= SELECT(C) WHITESPACE ENTRY_ID(ID) WHITESPACE EXPORT_PASSWORD_OR_KEY(E) WHITESPACE OTHER_ID(POS) NEWLINE.
{
    parseResult->addToken( POS );
    parseResult->addToken( E );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::EXPORT_PASSWORD_OR_KEY, ID, POS ) ) );
}
cmd_line ::= UPDATE.                         { parseResult->setCompleteSpace(); }
cmd_line ::= UPDATE WHITESPACE.              { parseResult->setCompleteEntry(); }
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) NEWLINE.
{
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::UPDATE, ID ) ) );
}
cmd_line ::= UPDATE WHITESPACE ENTRY_ID WHITESPACE. { parseResult->setCompleteUpdateCommand(); }
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
   parseResult->setEntryId( ID );
   parseResult->setCompleteAttribute();
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE DELETE_ATTRIBUTE(D) WHITESPACE OTHER_ID(POS) NEWLINE.
{
    parseResult->addToken( POS );
    parseResult->addToken( D );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::DELETE_ATTRIBUTE, ID, POS ) ) );
}
cmd_line ::= UPDATE WHITESPACE ENTRY_ID(ID) WHITESPACE UPDATE_ATTRIBUTE WHITESPACE.
{
   parseResult->setEntryId( ID );
   parseResult->setCompleteAttribute();
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE UPDATE_ATTRIBUTE(U) WHITESPACE OTHER_ID(POS) NEWLINE.
{
    parseResult->addToken( POS );
    parseResult->addToken( U );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::UPDATE_ATTRIBUTE, ID, POS ) ) );
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE ADD_PASSWORD(A) NEWLINE.
{
    parseResult->addToken( A );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::ADD_PASSWORD, ID ) ) );
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE ADD_KEY(A) NEWLINE.
{
    parseResult->addToken( A );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::ADD_KEY, ID ) ) );
}
cmd_line ::= UPDATE WHITESPACE ENTRY_ID(ID) WHITESPACE UPDATE_PASSWORD_OR_KEY WHITESPACE.
{
   parseResult->setEntryId( ID );
   parseResult->setCompletePasswordOrKey();
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE UPDATE_PASSWORD_OR_KEY(U) WHITESPACE OTHER_ID(POS) NEWLINE.
{
    parseResult->addToken( POS );
    parseResult->addToken( U );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::UPDATE_PASSWORD_OR_KEY, ID, POS ) ) );
}
cmd_line ::= UPDATE WHITESPACE ENTRY_ID(ID) WHITESPACE DELETE_PASSWORD_OR_KEY WHITESPACE.
{
   parseResult->setEntryId( ID );
   parseResult->setCompletePasswordOrKey();
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE DELETE_PASSWORD_OR_KEY(D) WHITESPACE OTHER_ID(POS) NEWLINE.
{
    parseResult->addToken( POS );
    parseResult->addToken( D );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::DELETE_PASSWORD_OR_KEY, ID, POS ) ) );
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE ADD_TAG(T) NEWLINE.
{
    parseResult->addToken( T );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::ADD_TAG, ID ) ) );
}
cmd_line ::= UPDATE WHITESPACE ENTRY_ID(ID) WHITESPACE UPDATE_TAG WHITESPACE.
{
   parseResult->setEntryId( ID );
   parseResult->setCompleteTag();
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE UPDATE_TAG(U) WHITESPACE OTHER_ID(POS) NEWLINE.
{
    parseResult->addToken( POS );
    parseResult->addToken( U );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::UPDATE_TAG, ID, POS ) ) );
}
cmd_line ::= UPDATE WHITESPACE ENTRY_ID(ID) WHITESPACE DELETE_TAG WHITESPACE.
{
   parseResult->setEntryId( ID );
   parseResult->setCompleteTag();
}
cmd_line ::= UPDATE(C) WHITESPACE ENTRY_ID(ID) WHITESPACE DELETE_TAG(T) WHITESPACE OTHER_ID(POS) NEWLINE.
{
    parseResult->addToken( POS );
    parseResult->addToken( T );
    parseResult->addToken( ID );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new EntryTask( EntryTask::DELETE_TAG, ID, POS ) ) );
}
cmd_line ::= APG(A) NEWLINE.
{
    parseResult->addToken( A );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new ApgTask( Vector<String>() ) ) );
}
cmd_line ::= APG(A) WHITESPACE arguments NEWLINE.
{
    parseResult->addToken( A );

    Vector<String> tokens( parseResult->getArgumentTokens() );
    tokens.insert( tokens.begin(), parseResult->getCommandToken() );
    parseResult->setCommand(
        std::shared_ptr<ICommand>( new ApgTask( tokens ) ) );
}
cmd_line ::= WRITE.                    { parseResult->setCompleteCommand(); }
cmd_line ::= WRITE WHITESPACE.         { parseResult->setCompleteFile(); }
cmd_line ::= WRITE(C) WHITESPACE ARGUMENT(A) NEWLINE.
{
    parseResult->addToken( A );
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new InstanceTask( InstanceTask::WRITE, A ) ) );
}
cmd_line ::= RECRYPT(C) NEWLINE.
{
    parseResult->addToken( C );

    parseResult->setCommand(
        std::shared_ptr<ICommand>( new InstanceTask( InstanceTask::RECRYPT ) ) );
}
arguments ::= ARGUMENT(A).
{
    parseResult->addToken( A );
}
arguments ::= ARGUMENT(A) WHITESPACE arguments.
{
    parseResult->addToken( A );
}
