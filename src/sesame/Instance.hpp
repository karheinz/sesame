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


#ifndef SESAME_INSTANCE_HPP
#define SESAME_INSTANCE_HPP

#include <cstdint>
#include <iostream>

#include "types.hpp"

#include "msgpack.hpp"
#include "msgpack/type/String.hpp"
#include "msgpack/type/Vector.hpp"

#include "sesame/definitions.hpp"
#include "sesame/packaging.hpp"
#include "sesame/Entry.hpp"
#include "sesame/crypto/IMachine.hpp"


namespace sesame
{
   /**
    * Class of sesame containers protecting entries
    * with password(s) and/or key(s).
    */
   class Instance
   {
      public:
         /**
          * Parses data read from stream.
          *
          * @throw std::runtime_error if parsing fails
          */
         static void parse( std::istream& stream );

         /**
          * Creates an empty instance.
          *
          * @param protocol the protocol to use
          * @param params the params to use for key derivation
          * @param decryptAllByDefault <tt>true</tt> if data
          *    should be decrypted when opening sesame
          *
          * @throw std::runtime_error if protocol is unknown
          */
         explicit Instance(
            const Protocol protocol,
            const Map<String,Vector<uint8_t>>& params = Map<String,Vector<uint8_t>>(),
            const bool decryptAllByDefault = false
            );

         /**
          * Constructs instance out of stream.
          *
          * @param stream the stream to construct instance from
          * @param password the password used to derive key
          *
          * @throw std::runtime_error if construction fails
          */
         Instance( std::istream& stream, const String& password );

         /** Destructor. */
         virtual ~Instance() = default;

         /**
          * Returns <tt>true</tt> if data is decrypted when opening sesame.
          *
          * @return <tt>true</tt> if data is decrypted when opening sesame
          */
         bool getDecryptAllByDefault() const;

         /**
          * Returns <tt>true</tt> if the container is new,
          * wasn't encrypted before.
          *
          * @return <tt>true</tt> if the container is new
          */
         bool isNew() const;

         /**
          * Returns the protocol used for encryption/decryption.
          *
          * @return the current protocol
          */
         Protocol getProtocol() const;

         /**
          * Returns the crypto machine used by the instance.
          *
          * @return the crypto machine
          *
          * @throw if no machine is available
          */
         crypto::IMachine& getCryptoMachine() const;

         /**
          * Returns the unique id of the instance.
          *
          * @return the unique id of the instance
          */
         uint32_t getId();

         /**
          * Returns all entries of the container with specified tags applied.
          *
          * @param tags tags to use as filter
          *
          * @return all entries of the container with specified tags applied.
          */
         Set<Entry> getEntries(
            const Set<String>& tags = Set<String>() ) const;

         /**
          * Returns all tags applied to container entries.
          *
          * @return all tags applied to container entries
          */
         Set<String> getTags() const;

         /**
          * Finds entry by (partial) hex id.
          *
          * @param id the (partial) hex id
          *
          * @return the entry found
          *
          * @throw std::runtime_error if no entry was or
          *     multiple entries were found
          */
         Entry findEntry( const String& hexId ) const;

         /**
          * Finds entries by (partial) hex id.
          *
          * @param id the (partial) hex id
          *
          * @return set with the entries found
          */
         Set<Entry> findEntries( const String& hexId ) const;

         /**
          * Decrypts an entry.
          *
          * @param entry the entry to decrypt
          * @param password the password to use
          *
          * @throw std::runtime_error on failure
          */
         void decryptEntry( Entry& entry, const String& password );

         /**
          * Adds the passed entry.
          *
          * @param[in,out] entry the entry to add
          *
          * @return <tt>true</tt> for success, otherwise <tt>false</tt>
          */
         bool addEntry( Entry& entry );

         /**
          * Updates the passed entry.
          *
          * @param entry the entry to update
          *
          * @return <tt>true</tt> for success, otherwise <tt>false</tt>
          */
         bool updateEntry( const Entry& entry );

         /**
          * Deletes the passed entry.
          *
          * @param entry the entry to delete
          *
          * @return <tt>true</tt> for success, otherwise <tt>false</tt>
          */
         bool deleteEntry( Entry& entry );

         /**
          * Recalcs initial digest.
          */
         void recalcInitialDigest();

         /**
          * Returns <tt>true</tt> if there are unsafed changes.
          *
          * @return <tt>true</tt> if there are unsafed changes
          */
         bool isDirty() const;

         /**
          * Encrypts the container with passed key and writes it to <tt>stream</tt>.
          *
          * @param stream the stream to write to
          * @param password the password
          *
          * @throw std::runtime_error on failure
          */
         void write(
            std::ostream& stream,
            const String& password
            );

      private:
         /**
          * Creates an empty instance, required by msgpack.
          */
         Instance();

         /**
          * Copy constructor.
          *
          * @param other other instance
          */
         Instance( const Instance& other ) = default;

         /**
          * Assignment operator.
          *
          * @param other other instance
          */
         Instance& operator=( const Instance& other ) = default;

         /**
          * Calculates digest of container (used to detect changes).
          *
          * @return digest of container
          */
         const Vector<uint8_t> calcDigest() const;

         /**
          * Calculates the HMAC of the passed value.
          *
          * @param value the value to calc HMAC for
          * @param[out] hmac the calculated HMAC
          *
          * @throw std::runtime_error on failure
          */
         void calcHmac( uint32_t value, const Vector<uint8_t>& key, Vector<uint8_t>& hmac ) const;

         /**
          * Use key for symmetric encryption/decryption.
          * Recalculates HMAC of id to be able to check key later.
          *
          * @param key the key to use
          *
          * @throw std::runtime_error on failure
          */
         void useKey( const Vector<uint8_t>& key ) const;

         /**
          * Returns <tt>true</tt> if the passed key is valid.
          * If no key was used so far (HMAC of id empty),
          * HMAC of id is calculated using the passed key.
          *
          * @param key the key to check
          *
          * @return <tt>true</tt> if the key is valid, otherwise <tt>false</tt>
          */
         bool isKeyValid( const Vector<uint8_t>& key ) const;

         /**
          * Encrypts an entry.
          *
          * @param entry the entry to encrypt
          * @param key the key to use
          *
          * @throw std::runtime_error on failure
          */
         void encryptEntry( Entry& entry, const Vector<uint8_t>& key );

         /**
          * Encrypts all entries.
          *
          * @param key the key to use
          *
          * @throw std::runtime_error on failure
          */
         void encryptEntries( const Vector<uint8_t>& key );

         /**
          * Decrypts an entry.
          *
          * @param entry the entry to decrypt
          * @param key the key to use
          *
          * @throw std::runtime_error on failure
          */
         void decryptEntry( Entry& entry, const Vector<uint8_t>& key );

         /**
          * Decrypts all entries.
          *
          * @param key the key to use
          *
          * @throw std::runtime_error on failure
          */
         void decryptEntries( const Vector<uint8_t>& key );

         /**
          * Throws an exception if protocol is unknown.
          *
          * @param protocol the protocol to check
          */
         void throwIfProtocolIsUnknown( const Protocol protocol );

         /**
          * Returns the crypto machine to use for crypto operations,
          * according to passed protocol.
          *
          * @param protocol the protocol
          * @return the crypto machine
          *
          * @throw if no machine is available for the passed protocol
          */
         static crypto::IMachine& getCryptoMachine( Protocol protocol );


         /** a map with crypto machine for each used protocol */
         static Map<Protocol,std::shared_ptr<crypto::IMachine>> machines;
         /** unique id of the instance */
         uint32_t m_Id;
         /** HMAC of id, used to check password */
         mutable Vector<uint8_t> m_Hmac;
         /** initial digest of container */
         Vector<uint8_t> m_InitialDigest;
         /** decrypt all data when opening sesame */
         bool m_DecryptAllByDefault;
         /** the protocol to use */
         Protocol m_Protocol;
         /** the key derivation params to use */
         Map<String,Vector<uint8_t>> m_Params;
         /** the covered entries */
         Set<Entry> m_Entries;

      // (de)serialization
      public:
         MSGPACK_DEFINE( m_Id, m_Hmac, m_DecryptAllByDefault, m_Protocol, m_Params, m_Entries )


      friend Instance msgpack::object::as<Instance>() const;
      friend std::istream& unpack<Instance>( std::istream& i, Instance& e );
   };
}

/**
 * For easy serialization.
 *
 * @param o the stream to write to
 * @param x the instance to write
 *
 * @return the stream
 */
inline std::ostream& operator<<( std::ostream& o, const sesame::Instance& x )
{
   return sesame::pack( o, x );
}

/**
 * For easy deserialization.
 *
 * @param i the stream to read from
 * @param x the instance to write
 *
 * @return the stream
 */
inline std::istream& operator>>( std::istream& i, sesame::Instance& x )
{
   return sesame::unpack( i, x );
}

#endif
