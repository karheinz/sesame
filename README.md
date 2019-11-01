[![Build Status](https://travis-ci.org/karheinz/sesame.svg?branch=master)](https://travis-ci.org/karheinz/sesame)

Sesame - A command line password manager
========================================

Introduction
------------

Sesame is a command line tool to manage confidential data.  The
name of the program is derived from the folk-tale "Ali Baba and
the Forty Thieves".

The program comes with its own shell.  It organizes data into
entries which live in an encrypted container.  Additionally each
confidential data element is encrypted on its own.  An entry has
an unique identifier and a name. It covers attributes (meta data)
and confidential data (passwords, keys, etc.). Entries can be
grouped by tags.  A container can be stored as binary blob or
embedded into a jpeg image.

The program and container layout is designed to support various
crypto protocols and backends. Currently one backend is
available, which uses scrypt for password based key derivation
(extended PBKDF2) and OpenSSL for symmetric cryptography
(AES256), checksums (SHA256) and message authentication codes
(HMAC-SHA256). A custom memory allocator is used to ensure that
text and binary data are zeroed after usage.  If the sesame
binary is owned by root and the suid bit is set, all memory
allocated by sesame will be locked (no swapping) and no core
files will be written.

Containers are (de)serialized using MessagePack binary format.
Interactive command line editing facilities are provided by
Tecla.

Sesame currently builds for GNU/Linux and (Open)BSD.


Features
--------

* create, open, close and write containers
* list, search, show, add, update and delete entries
* add, update and delete attributes, passwords, keys and tags
* export a password (text) to clipboard
* export a key (binary data) to file
* generate passwords
* embed a container into a jpeg image

If the binary is owned by root and suid bit is set:

* memory will be locked (no swapping)
* no core files will be written

What is missing/planned?

* add man page and more documentation
* ...


Usage
-----

Without container:

```
sesame> help
Currently available commands:

       help
              prints this message

       edit-mode (emacs|vi)
              sets editing mode to either emacs or vi

       clear
              clears the screen

       quit
              quits sesame

       new
              creates a new empty container

       open FILE
              opens an existing container stored in FILE

sesame>
```

With container:

```
sesame #b894ed8a> help
Currently available commands:

       help
              prints this message

       edit-mode (emacs|vi)
              sets editing mode to either emacs or vi

       clear
              clears the screen

       write FILE
              writes current container to FILE or
              embeds container, if FILE is a JPEG image

       recrypt
              recrypts the container with new crypto params and/or password/phrase

       close
              closes the current container

       quit
              quits sesame

       apg [OPTION...]
              runs (a) (p)assword (g)enerator, use -h for help

       list [TAG]
              lists entries of the container (tagged with TAG)

       search PATTERN
              searches for entries matching PATTERN (ERE)

       tree
              lists all entries by tags

       tags
              lists all tags assigned to entries

       add
              adds an entry to the current container

      (decrypt|show|update|delete) ENTRY
              decrypts, shows, updates or deletes the specified ENTRY

       update ENTRY (add_tag|add_attribute|add_password|add_key)
              adds a tag, attribute, password or key to the specified ENTRY

       update ENTRY (update_tag|update_attribute|update_password_or_key) ID
              updates tag, attribute, password or key with ID of the specified ENTRY

       update ENTRY (delete_tag|delete_attribute|delete_password_or_key) ID
              deletes tag, attribute, password or key with ID of the specified ENTRY

       select ENTRY export_password_or_key ID
              exports password with ID of the specified ENTRY to clipboard or
              exports key with ID of the specified ENTRY to file

sesame #b894ed8a>
```


Installation
------------

1. Install the required programs and libraries:
   * GNU/Linux (Ubuntu)

      ```
      $ sudo apt-get install cmake nasm openssl libx11-dev
      ```
   * (Open)BSD

      ```
      $ sudo pkg_add cmake gcc g++ nasm openssl
      ```
2. Set environment (OpenBSD):

   ```
   $ . set_env_openbsd.sh
   ```
3. Build sesame and run tests:

   ```
   $ mkdir build
   $ cd build
   $ cmake <src-dir>
   $ make
   $ make test
   ```
4. Install:

   ```
   $ sudo make install
   ```


Used libraries
--------------

* Scrypt
  - http://www.tarsnap.com/scrypt.html
* OpenSSL
  - https://www.openssl.org
* Msgpack-C
  - https://github.com/msgpack/msgpack-c
* Tecla
  - http://www.astro.caltech.edu/~mcs/tecla/
* flex
  - http://flex.sourceforge.net
* LEMON
  - http://www.hwaci.com/sw/lemon/
* zlib
  - http://www.zlib.net
* googletest
  - http://code.google.com/p/googletest/
* apg (modified)
  - http://www.adel.nursat.kz/apg/
* xsel (modified)
  - http://www.vergenet.net/~conrad/software/xsel/
* libjpeg-turbo
  - http://www.libjpeg-turbo.org/

The (patched) sources of some of the libraries can be found in
the libs directory. The LEMON sources are located under src/lemon.
