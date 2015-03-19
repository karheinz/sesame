Sesame - a cmd line password manager
====================================

Introduction
------------

Sesame is a command line tool to manage passwords and keys.
The program is written in C++ and released under the BSD 2-Clause License.
The name of the program is derived from the folk-tale "Ali Baba and the
Forty Thieves" (Open Sesame!).

A container managed by sesame consists of various entries. Each entry has
a name and is identified by an unique id. An arbitrary number of tags,
attributes and labeled data (text or binary) can be assigned to an entry.

The program and container layout is designed to support various
crypto protocols and backends. Currently one backend is available,
which uses scrypt for password based key derivation (extended PBKDF2)
and OpenSSL for symmetric cryptography (AES256), checksums (SHA256)
and message authentication codes (HMAC-SHA256). A custom memory allocator
is used to ensure that text and binary data are zeroed after usage.

Containers are (de)serialized using MessagePack binary format.
Interactive command line editing facilities are provided by Tecla.


Features
--------

What you can do so far is:

* create, open, close and write containers
* list, show, add, update and delete entries
* add, update and delete attributes, passwords and keys
* export passwords and keys
* embed container into jpeg image
* generate passwords

What is missing/planned?

* signal handler for clean shutdown (on SIGTERM etc.)
* man page and documentation
* ...


Installation
------------

1. Install the required programs and libraries:
```
   # apt-get install flex libfl-dev nasm openssl libtecla1-dev libx11-dev
```

2. Build sesame and run tests.
```
   $ mkdir build
   $ cd build
   $ cmake <src-dir>
   $ make
   $ make test
```

The binary is located under build/src.


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
* libjpeg-turbo (modified)
  - http://www.libjpeg-turbo.org/

The sources and - at least - the licenses of the stated libraries
can be found in the libs directory. The LEMON sources are located
under src/lemon.

