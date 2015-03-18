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


#ifndef SESAME_ENTRY_HPP
#define SESAME_ENTRY_HPP

#include <iostream>

#include "types.hpp"
#include "sesame/Data.hpp"
#include "sesame/definitions.hpp"
#include "sesame/packaging.hpp"


namespace sesame
{
   class Instance;

   /**
    * Class of sesame entries.
    */
   class Entry
   {
      public:
         /**
          * Default constructor.
          */
         Entry();

         /**
          * Constructor.
          *
          * @param name name of the entry
          */
         Entry( const String& name );

         /**
          * Copy constructor.
          *
          * @param other the other sesame entry to copy
          */
         Entry( const Entry& other ) = default;

         /**
          * Assignment operator.
          *
          * @param other the other sesame entry to copy
          *
          * @return reference to self
          */
         Entry& operator=( const Entry& other ) = default;

         /** Destructor. */
         virtual ~Entry() = default;

         /**
          * Returns the unique id of the entry.
          *
          * @return the unique id of the entry
          */
         uint32_t getId() const;

         /**
          * Returns id as hex string.
          *
          * @return id as hex string
          */
         String getIdAsHexString() const;

         /**
          * Returns the name of the entry.
          *
          * @return the name of the entry
          */
         String getName() const;

         /**
          * Sets the name of the entry.
          *
          * @param name the name of the entry
          */
         void setName( const String& name );

         /**
          * Returns all attributes of the entry.
          *
          * @return the attributes of the entry
          */
         Map<String,String> getAttributes() const;

         /**
          * Adds an attribute to the entry.
          *
          * @param name the name of the attribute
          * @param value the value of the attribute
          *
          * @return <tt>true</tt> for success, otherwise <tt>false</tt>
          */
         bool addAttribute( const String& name, const String& value );

         /**
          * Updates an attribute.
          *
          * @param oldName the old name of the attribute to update
          * @param newName the new name of the attribute to update
          * @param value the new value of the attribute
          *
          * @return <tt>true</tt> for success, otherwise <tt>false</tt>
          */
         bool updateAttribute(
            const String& oldName,
            const String& newName,
            const String& value
            );

         /**
          * Deletes an attribute.
          *
          * @param name the name of the attribute to update
          * @param value the new value of the attribute
          *
          * @return <tt>true</tt> for success, otherwise <tt>false</tt>
          */
         bool deleteAttribute( const String& name );

         /**
          * Returns all labeled data assigned to entry.
          *
          * @return all labeled data assigned to entry
          */
         Map<String,Data> getLabeledData() const;

         /**
          * Adds data labeled with <tt>label</tt>.
          *
          * @param label the label to use
          * @param data the data
          *
          * @return <tt>true</tt> for success, otherwise <tt>false</tt>
          */
         bool addLabeledData( const String& label, const Data& data );

         /**
          * Updates data labeled with <tt>oldLabel</tt>.
          *
          * @param oldLabel the old label of the data to update
          * @param newLabel the new label of the data to update
          * @param data the data
          *
          * @return <tt>true</tt> for success, otherwise <tt>false</tt>
          */
         bool updateLabeledData( const String& oldLabel, const String& newLabel, const Data& data );

         /**
          * Deletes data labeled with <tt>label</tt>.
          *
          * @param label the label to use
          *
          * @return <tt>true</tt> for success, otherwise <tt>false</tt>
          */
         bool deleteLabeledData( const String& label );

         /**
          * Returns <tt>true</tt> if plaintexts of all
          * assigned <tt>Data</tt> are available,
          * so all <tt>Data</tt> and the <tt>Entry</tt>
          * data can be encrypted.
          *
          * @return <tt>true</tt> if plaintexts of all
          *    assigned <tt>Data</tt> are available
          */
         bool isPlain() const;

         /**
          * Returns the assigned tags for lookup.
          *
          * @return the assigned tags for lookup
          */
         Set<String> getTags() const;

         /**
          * Returns <tt>true</tt> if the entry has the specified tag.
          *
          * @param tag the tag to check
          *
          * @return <tt>true</tt> if the entry has the specified tag
          */
         bool hasTag( const String& tag ) const;

         /**
          * Adds a tag to the entry.
          *
          * @param tag the tag to add
          */
         void addTag( const String& tag );

         /**
          * Removes a tag from the entry.
          *
          * @param tag the tag to delete
          */
         void deleteTag( const String& tag );

         /**
          * Clears data so it can be used in other contexts.
          */
         void clear();

         /** == operator */
         bool operator==( const Entry& other ) const
         {
            return m_Id == other.m_Id;
         }

         /** != operator */
         bool operator!=( const Entry& other ) const
         {
            return ! ( m_Id == other.m_Id );
         }

         /** > operator */
         bool operator>( const Entry& other ) const
         {
            return ( m_Id > other.m_Id );
         }

         /** < operator */
         bool operator<( const Entry& other ) const
         {
            return ( m_Id < other.m_Id );
         }

      private:
         /**
          * Reconfigures entry so that it can be
          * used with a new instance.
          *
          * @param instanceId the id of the new instance
          */
         void reconfigure( uint32_t instanceId );


         /** unique id of the entry */
         uint32_t m_Id;

         /** id of the corresponding instance */
         uint32_t m_InstanceId;

         /** entry was created at seconds since epoch */
         int64_t m_CreatedAt;

         /** entry was updated at seconds since epoch */
         int64_t m_UpdatedAt;

         /** name of the entry */
         String m_Name;

         /** attributes describing the entry */
         Map<String,String> m_Attributes;

         /** labeled data */
         Map<String,Data> m_LabeledData;

         /** tags for lookup */
         Set<String> m_Tags;

      // (de)serialization
      public:
         MSGPACK_DEFINE( \
            m_Id, \
            m_InstanceId, \
            m_CreatedAt, \
            m_UpdatedAt, \
            m_Name, \
            m_Attributes, \
            m_LabeledData, \
            m_Tags
            )

      /**
       * Cryptograpic protocol is enforced by an <tt>Instance</tt>.
       * So instances are allowed to change internal state of <tt>Entry</tt>.
       */
      friend class Instance;
   };
}

/**
 * For easy serialization.
 *
 * @param o the stream to write to
 * @param e the entry to write
 *
 * @return the stream
 */
inline std::ostream& operator<<( std::ostream& o, const sesame::Entry& e )
{
   return sesame::pack<sesame::Entry>( o, e );
}

/**
 * For easy deserialization.
 *
 * @param i the stream to read from
 * @param e the entry to write
 *
 * @return the stream
 */
inline std::istream& operator>>( std::istream& i, sesame::Entry& e )
{
   return sesame::unpack<sesame::Entry>( i, e );
}

#endif
