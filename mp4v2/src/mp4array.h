/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is MPEG4IP.
 * 
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2001.  All Rights Reserved.
 * 
 * Contributor(s): 
 *		Dave Mackie		dmackie@cisco.com
 */


#ifndef MP4V2_IMPL_MP4ARRAY_H
#define MP4V2_IMPL_MP4ARRAY_H

#include <vector>
using namespace std;

typedef uint32_t    MP4ArrayIndex;

template<class T>
class Array
{
    /* -----------------------------------------------------------------------------------------------------------------
     ------------------------------------------------------------------------------------------------------------------- */
    public:
        inline MP4ArrayIndex Size(void)
        {
            return m_vector.size();
        }

        inline void Add(T newElement)
        {
            m_vector.push_back(newElement);
        }

        inline void Insert(T newElement, MP4ArrayIndex newIndex)
        {
            if(newIndex > m_vector.size()) {
                throw new mp4v2::impl::MP4Error(ERANGE, "MP4Array::Insert");
            }

            m_vector.insert(m_vector.begin() + newIndex, newElement);
        }

        inline void Delete(MP4ArrayIndex index)
        {
            if(index >= m_vector.size()) {
                throw new mp4v2::impl::MP4Error(ERANGE, "MP4Array::Delete");
            }

            typename vector<T>::iterator    arrayIter = m_vector.begin() + index;
            m_vector.erase(arrayIter);
        }

        inline void Resize(MP4ArrayIndex newSize)
        {
            m_vector.resize(newSize);
        }

        inline T &operator[](MP4ArrayIndex index)
        {
            if(index >= m_vector.size()) {
                throw new mp4v2::impl::MP4Error(ERANGE, "index %u of %u", "MP4Array::[]", index, m_vector.size());
            }

            return m_vector[index];
        }

    /* -----------------------------------------------------------------------------------------------------------------
     ------------------------------------------------------------------------------------------------------------------- */
    protected:
        vector<T>   m_vector;
};

#define MP4ARRAY_DECL(name, type)   typedef Array<type>  name##Array;

MP4ARRAY_DECL(MP4Integer8, uint8_t)

MP4ARRAY_DECL(MP4Integer16, uint16_t)

MP4ARRAY_DECL(MP4Integer32, uint32_t)

MP4ARRAY_DECL(MP4Integer64, uint64_t)

MP4ARRAY_DECL(MP4Float32, float)

MP4ARRAY_DECL(MP4Float64, double)

MP4ARRAY_DECL(MP4String, char *)

MP4ARRAY_DECL(MP4Bytes, uint8_t *)

#endif /* MP4V2_IMPL_MP4ARRAY_H */
