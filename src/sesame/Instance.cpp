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


#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>

#include "sesame/packaging.hpp"
#include "sesame/version.hpp"
#include "sesame/Instance.hpp"
#include "sesame/crypto/MachineFactory.hpp"


namespace sesame
{
   void Instance::parse( std::istream& stream )
   {
      uint32_t majorVersion;
      unpack( stream, majorVersion );
      Protocol protocol;
      unpack( stream, protocol );
      Map<String,Vector<uint8_t>> params;
      unpack( stream, params );
      Vector<uint8_t> ciphertext;
      unpack( stream, ciphertext );
      Vector<uint8_t> hmac;
      unpack( stream, hmac );
      Vector<uint8_t> digest;
      unpack( stream, digest );
   }

   Instance::Instance() :
      m_Id( std::random_device()() ),
      m_DecryptAllByDefault( false ),
      m_Protocol( PROTOCOL_UNKNOWN )
   {
   }

   Instance::Instance(
      const Protocol protocol,
      const Map<String,Vector<uint8_t>>& params,
      const bool decryptAllByDefault
      ) :
      m_Id( std::random_device()() ),
      m_DecryptAllByDefault( decryptAllByDefault ),
      m_Protocol( protocol ),
      m_Params( params )
   {
      throwIfProtocolIsUnknown( m_Protocol );

      if ( ! getCryptoMachine().getKeyDerivationParams( m_Params ) )
      {
         throw std::runtime_error( "failed to get key derivation params" );
      }

      m_InitialDigest = calcDigest();
   }

   Instance::Instance( std::istream& stream, const String& password )
   {
      uint32_t majorVersion;
      unpack( stream, majorVersion );
      unpack( stream, m_Protocol );
      unpack( stream, m_Params );
      Vector<uint8_t> ciphertext;
      unpack( stream, ciphertext );
      std::streamsize hmacCheck( stream.tellg() );
      Vector<uint8_t> hmac;
      unpack( stream, hmac );
      std::streamsize digestCheck( stream.tellg() );
      Vector<uint8_t> digest;
      unpack( stream, digest );
      std::streamsize end( -1 );
      if ( ! stream.eof() )
      {
         end = stream.tellg();
      }

      // Check integrity.
      if ( stream.eof() )
      {
         stream.clear();
      }
      stream.seekg( 0, std::ios_base::beg );
      Vector<uint8_t> data( digestCheck );
      stream.read( reinterpret_cast<char*>( data.data() ), digestCheck );
      if ( end == -1 )
      {
         stream.seekg( 0, std::ios_base::end );
      }
      else
      {
         stream.seekg( end, std::ios_base::beg );
      }
      Vector<uint8_t> calculatedDigest;
      if ( ! getCryptoMachine().calcDigest( data, calculatedDigest ) )
      {
         throw std::runtime_error( "failed to calculate digest" );
      }

      if ( calculatedDigest != digest )
      {
         throw std::runtime_error( "integrity check failed" );
      }

      // Some checks.
      if ( majorVersion != 0 )
      {
         throw std::runtime_error( "incompatible major version" );
      }
      throwIfProtocolIsUnknown( m_Protocol );

      // Calc key.
      Vector<uint8_t> key;
      if ( ! getCryptoMachine().deriveKey( utils::toUtf8( password ), m_Params, key ) )
      {
         throw std::runtime_error( "key derivation failed" );
      }
      // Authenticity check.
      Vector<uint8_t> calculatedHmac;
      if ( ! getCryptoMachine().calcHmac( data.data(), hmacCheck, key, calculatedHmac ) )
      {
         throw std::runtime_error( "failed to calculate HMAC" );
      }
      if ( calculatedHmac != hmac )
      {
         throw std::runtime_error( "key is invalid" );
      }

      // Decrypt ciphertext.
      Vector<uint8_t> plaintext;
      if ( ! getCryptoMachine().decrypt( ciphertext, key, plaintext ) )
      {
         throw std::runtime_error( "decryption failed" );
      }

      // Deserialize.
      StringStream tmp( String( reinterpret_cast<char*>( plaintext.data() ), plaintext.size() ) );
      Instance instance;
      tmp >> instance;

      // Check.
      if ( instance.m_Protocol != m_Protocol )
      {
         throw std::runtime_error( "unexpected protocol" );
      }
      if ( instance.m_Params != m_Params )
      {
         throw std::runtime_error( "unexpected derivation params" );
      }

      // Decrypt data?
      if ( instance.getDecryptAllByDefault() )
      {
         instance.decryptEntries( key );
      }

      instance.m_InitialDigest = instance.calcDigest();

      // Now replace current instance with deserialized.
      *this = instance;
   }

   bool Instance::getDecryptAllByDefault() const
   {
      return m_DecryptAllByDefault;
   }

   bool Instance::isNew() const
   {
      return m_Hmac.empty();
   }

   Protocol Instance::getProtocol() const
   {
      return m_Protocol;
   }

   uint32_t Instance::getId()
   {
      return m_Id;
   }

   Set<Entry> Instance::getEntries( const Set<String>& tags ) const
   {
      if ( tags.empty() )
      {
         return m_Entries;
      }
      else
      {
         Set<Entry> entries;
         Set<String> tmp;
         Vector<String> intersection;

         for ( Set<Entry>::const_iterator it = m_Entries.cbegin(); it != m_Entries.cend(); ++it )
         {
            tmp = it->getTags();
            intersection.clear();

            std::set_intersection( tmp.begin(), tmp.end(), tags.begin(), tags.end(), std::back_inserter( intersection ) );
            if ( ! intersection.empty() )
            {
               entries.insert( *it );
            }
         }

         return entries;
      }
   }

   Set<String> Instance::getTags() const
   {
      Set<String> tags;
      Set<String> tmp;

      for ( Set<Entry>::const_iterator it = m_Entries.cbegin(); it != m_Entries.cend(); ++it )
      {
         tmp = it->getTags();
         tags.insert( tmp.begin(), tmp.end() );
      }

      return tags;
   }

   Entry Instance::findEntry( const String& hexId ) const
   {
      Set<Entry> entries( findEntries( hexId ) );
      if ( entries.empty() )
      {
         throw std::runtime_error( "entry not found" );
      }
      else if ( entries.size() > 1 )
      {
         throw std::runtime_error( "multiple entries found" );
      }
      else
      {
         return Entry( *( entries.begin() ) );
      }
   }

   Set<Entry> Instance::findEntries( const String& hexId ) const
   {
      // Adjust id.
      String s( hexId );

      if ( s.find( "#" ) == 0 )
      {
         s.replace( 0, 1, "" );
      }
      else if ( s.find( "0x" ) == 0 )
      {
         s.replace( 0, 2, "" );
      }

      Set<Entry> result;
      Set<Entry> entries( getEntries() );
      for ( auto entry : entries )
      {
         StringStream id;
         id << std::hex << std::setw( 8 ) << std::setfill( '0' ) << entry.getId();
         if ( String( id.str().substr( 0, s.size() ) ) == s )
         {
            result.insert( entry );
         }
      }

      return result;
   }

   void Instance::decryptEntry( Entry& entry, const String& password )
   {
      Vector<uint8_t> key;

      if ( ! getCryptoMachine().deriveKey( utils::toUtf8( password ), m_Params, key ) )
      {
         throw std::runtime_error( "key derivation failed" );
      }

      decryptEntry( entry, key );
   }

   void Instance::decryptEntry( Entry& entry, const Vector<uint8_t>& key )
   {
      if ( ! isKeyValid( key ) )
      {
         throw std::runtime_error( "key is invalid" );
      }

      crypto::IMachine& machine( getCryptoMachine() );

      for ( auto labeledData : entry.m_LabeledData )
      {
         if ( ! labeledData.second.isPlaintextAvailable() )
         {
            Vector<uint8_t> calculatedHmac;
            if ( machine.calcHmac( labeledData.second.m_Ciphertext, key, calculatedHmac ) )
            {
               if ( calculatedHmac == labeledData.second.m_Hmac )
               {
                  if ( labeledData.second.getType() == Data::TEXT )
                  {
                     Vector<uint8_t> buffer;
                     if ( machine.decrypt( labeledData.second.m_Ciphertext, key, buffer ) )
                     {
                        String tmp( utils::fromUtf8( reinterpret_cast<const char*>( buffer.data() ), buffer.size() ) );
                        labeledData.second.m_Plaintext =
                           Vector<uint8_t>(
                              reinterpret_cast<const uint8_t*>( tmp.data() ),
                              reinterpret_cast<const uint8_t*>( tmp.data() ) + tmp.size()
                              );
                     }
                     else
                     {
                        throw std::runtime_error( "decryption failed" );
                     }
                  }
                  else
                  {
                     if ( ! machine.decrypt( labeledData.second.m_Ciphertext, key, labeledData.second.m_Plaintext ) )
                     {
                        throw std::runtime_error( "decryption failed" );
                     }
                  }
               }
               else
               {
                  throw std::runtime_error( "key is invalid" );
               }
            }
            else
            {
               throw std::runtime_error( "failed to calculate HMAC" );
            }
         }
      }
   }

   void Instance::decryptEntries( const Vector<uint8_t>& key )
   {
      for ( auto entry : m_Entries )
      {
         decryptEntry( entry, key );
      }
   }

   bool Instance::addEntry( Entry& entry )
   {
      if ( m_Entries.find( entry ) != m_Entries.end() )
      {
         // Already known.
         return false;
      }
      else if ( ! entry.isPlain() )
      {
         // Entry isn't fully plain.
         return false;
      }
      else
      {
         // Entry is new.
         entry.reconfigure( m_Id );
         m_Entries.insert( entry );
         return true;
      }
   }

   const Vector<uint8_t> Instance::calcDigest() const
   {
      Vector<uint8_t> data;
      Vector<uint8_t> digest;
      packV( data, *this );
      if ( ! getCryptoMachine().calcDigest( data, digest ) )
      {
         throw std::runtime_error( "calculating digest of sesame failed" );
      }

      return digest;
   }

   void Instance::calcHmac( uint32_t value, const Vector<uint8_t>& key, Vector<uint8_t>& hmac ) const
   {
      Vector<uint8_t> v;
      v.push_back( 0x000000ff && ( value >> 24 ) );
      v.push_back( 0x000000ff && ( value >> 16 ) );
      v.push_back( 0x000000ff && ( value >> 8 ) );
      v.push_back( 0x000000ff && ( value ) );

      if ( ! getCryptoMachine().calcHmac( v, key, hmac ) )
      {
         throw std::runtime_error( "failed to calculate HMAC" );
      }
   }

   void Instance::useKey( const Vector<uint8_t>& key ) const
   {
      calcHmac( m_Id, key, m_Hmac );
   }

   bool Instance::isKeyValid( const Vector<uint8_t>& key ) const
   {
      bool success( false );

      // No key used so far? Use passed key.
      if ( isNew() )
      {
         useKey( key );
         success = true;
      }

      // Check.
      if ( ! success );
      {
         Vector<uint8_t> calculatedHmac;
         calcHmac( m_Id, key, calculatedHmac );
         if ( calculatedHmac == m_Hmac )
         {
            success = true;
         }
      }

      return success;
   }

   void Instance::encryptEntry( Entry& entry, const Vector<uint8_t>& key )
   {
      if ( ! isKeyValid( key ) )
      {
         throw std::runtime_error( "key is invalid" );
      }

      crypto::IMachine& machine( getCryptoMachine() );

      for ( auto labeledData : entry.m_LabeledData )
      {
         if ( labeledData.second.isDirty() )
         {
            if ( labeledData.second.getType() == Data::TEXT )
            {
               String utf8String( utils::toUtf8( labeledData.second.getPlaintext<String>() ) );
               if ( ! machine.encrypt(
                       reinterpret_cast<const uint8_t*>( utf8String.data() ),
                       utf8String.size(),
                       key,
                       labeledData.second.m_Ciphertext
                       )
                  )
               {
                  throw std::runtime_error( "encryption failed" );
               }
            }
            else
            {
               if ( ! machine.encrypt( labeledData.second.m_Plaintext, key, labeledData.second.m_Ciphertext ) )
               {
                  throw std::runtime_error( "encryption failed" );
               }
            }

            if ( machine.calcHmac( labeledData.second.m_Ciphertext, key, labeledData.second.m_Hmac ) )
            {
               labeledData.second.m_Dirty = false;
            }
            else
            {
               throw std::runtime_error( "failed to calculate HMAC" );
            }
         }
      }
   }

   void Instance::encryptEntries( const Vector<uint8_t>& key )
   {
      for ( auto entry : m_Entries )
      {
         encryptEntry( entry, key );
      }
   }


   bool Instance::updateEntry( const Entry& entry )
   {
      if ( entry.m_InstanceId != m_Id )
      {
         return false;
      }

      // Lookup.
      Set<Entry>::iterator it = m_Entries.find( entry );

      if ( it == m_Entries.end() )
      {
         // Entry not found.
         return false;
      }
      else
      {
         // Replace original entry with new one.
         m_Entries.erase( it );
         m_Entries.insert( entry );
         return true;
      }
   }

   bool Instance::deleteEntry( Entry& entry )
   {
      if ( entry.m_InstanceId != m_Id )
      {
         return false;
      }

      // Lookup.
      Set<Entry>::iterator it = m_Entries.find( entry );

      if ( it == m_Entries.end() )
      {
         // Entry not found.
         return false;
      }
      else
      {
         // Delete entry.
         m_Entries.erase( it );
         entry.clear();
         return true;
      }
   }

   void Instance::recalcInitialDigest()
   {
      m_InitialDigest = calcDigest();
   }

   bool Instance::isDirty() const
   {
      // 1. compare digests
      if ( m_InitialDigest != calcDigest() )
      {
         return true;
      }

      // 2. check entries
      for ( auto entry : m_Entries )
      {
         for ( auto data : entry.getLabeledData() )
         {
            if ( data.second.isDirty() )
            {
               return true;
            }
         }
      }

      // No changes so far.
      return false;
   }

   void Instance::write(
      std::ostream& stream,
      const String& password
      )
   {
      Vector<uint8_t> key;

      if ( ! getCryptoMachine().deriveKey( utils::toUtf8( password ), m_Params, key ) )
      {
         throw std::runtime_error( "key derivation failed" );
      }

      // 1. Encrypt all entries.
      encryptEntries( key );

      // 2. Pack and write meta data.
      StringStream data;

      // sesame major version
      pack( data, VERSION_MAJOR );

      // protocol
      pack( data, static_cast<int32_t>( m_Protocol ) );

      // derivation params
      pack( data, m_Params );

      // 3. Serialize and encrypt.
      Vector<uint8_t> ciphertext;
      {
         Vector<uint8_t> serialized;
         {
            StringStream tmp;
            tmp << *this;
            readIntoVector( tmp, serialized );
         }

         if ( ! getCryptoMachine().encrypt( serialized, key, ciphertext ) )
         {
            throw std::runtime_error( "encryption failed" );
         }
      }
      pack( data, ciphertext );

      // 4. Calc HMAC of data and append.
      Vector<uint8_t> hmac;
      {
         Vector<uint8_t> serialized;
         readIntoVector( data, serialized );

         if ( ! getCryptoMachine().calcHmac( serialized, key, hmac ) )
         {
            throw std::runtime_error( "failed to calculate HMAC" );
         }
      }
      pack( data, hmac );

      // 5. Calc digest and append.
      Vector<uint8_t> digest;
      {
         Vector<uint8_t> serialized;
         readIntoVector( data, serialized );

         if ( ! getCryptoMachine().calcDigest( serialized, digest ) )
         {
            throw std::runtime_error( "failed to calculate digest" );
         }
      }
      pack( data, digest );

      // 6. Write to file.
      data.seekg( 0, std::ios_base::beg );
      Vector<char> buffer( 128 );
      while ( data.good() )
      {
         data.read( buffer.data(), buffer.size() );
         stream.write( buffer.data(), data.gcount() );
      }
   }

   void Instance::throwIfProtocolIsUnknown( const Protocol protocol )
   {
      if ( protocol == PROTOCOL_UNKNOWN )
      {
         throw std::runtime_error( "unknown protocol" );
      }
   }

   Map<Protocol,std::shared_ptr<crypto::IMachine>> Instance::machines;

   crypto::IMachine& Instance::getCryptoMachine( Protocol protocol )
   {
      if ( machines.find( protocol ) == machines.end() )
      {
         machines[ protocol ] = crypto::MachineFactory::buildMachine( protocol );
      }

      return *( machines[ protocol ] );
   }

   crypto::IMachine& Instance::getCryptoMachine() const
   {
      return getCryptoMachine( m_Protocol );
   }
}
