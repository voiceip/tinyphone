/***************************************************************************
	TemplateSmartPtr.h: header file

	Header of smart ptr template class

****************************************************************************/

#ifndef TEMPLATESMARTPTR_H
#define TEMPLATESMARTPTR_H

#define bTEMPLATESMARTPTR_NOTOWNED	false

template <typename TYPE> class CTemplateSmartPtrArray 
{
public:
	CTemplateSmartPtrArray()	
		{ 
			m_pBuffer = NULL;/* For later construction*/;
			m_bOwnObject = true;
		}
	CTemplateSmartPtrArray(IN const int nArrayLength)
		{
			m_bOwnObject = true;
			InitBuffer(nArrayLength);
		}	
	CTemplateSmartPtrArray(TYPE* pBuffer, IN const bool bOwned=true)
		{
			m_pBuffer = NULL;
			Assign(pBuffer, bOwned);
		}	
	virtual ~CTemplateSmartPtrArray()
		{
			// Auto-delete, but only if we own it!
			if (m_pBuffer && m_bOwnObject)
			{
				delete [] m_pBuffer;
			}
		}
	void		InitBuffer(IN const int nArrayLength)
		{
			m_pBuffer = new TYPE[nArrayLength];
		}
	void		Assign(TYPE* pBuffer, IN const bool bOwned=true)
		{
			if (m_pBuffer && m_bOwnObject)
			{
				delete []m_pBuffer;
			}
			m_pBuffer = pBuffer;
			m_bOwnObject = bOwned;
		}
	TYPE*		GetBuffer()
		{
			ASSERT(m_pBuffer);
			return m_pBuffer;
		}
	bool		IsNull()
		{
			return (m_pBuffer == NULL);
		}
	// Determine who gets to delete the buffer
	void		SetBufferOwnership(IN const bool bOwnObject)
		{
			m_bOwnObject = bOwnObject;
		}

	TYPE*		m_pBuffer;
	bool		m_bOwnObject;
};

template <typename TYPE> class CTemplateSmartPtr 
{
public:
	CTemplateSmartPtr()
		{
			m_bOwnObject = true;
			Init();
		}	
	CTemplateSmartPtr(TYPE* pThing)	// also permits NULL initialisation for later assigment
		{
			m_bOwnObject = true;
			m_pThing = NULL;
			Assign(pThing);
		}	
	virtual ~CTemplateSmartPtr()
		{
			// Auto-delete, but only if we own it!
			if (m_pThing && m_bOwnObject)
			{
				delete m_pThing;
			}
		}

	void		Init()
		{
			m_pThing = new TYPE;
		}
	void		Assign(TYPE* pThing)
		{
			if (m_pThing)
			{
				delete m_pThing;
			}
			m_pThing = pThing;
		}
	void		Detach()
		{
			// Stop the smart pointer managing the object
			m_pThing = NULL;
		}
	bool		IsNull()
		{
			return (m_pThing == NULL);
		}
	TYPE*		GetPtr()
		{
			ASSERT(m_pThing);
			return GetPtrSafe();
		}
	TYPE*		GetPtrSafe()
		{
			// Don't assert. That's the only "safe" thing about it
			return m_pThing;
		}
	void		SetOwnObject(IN const bool bOwnObject)
		{
			m_bOwnObject = bOwnObject;
		}

	TYPE*		m_pThing;
	bool		m_bOwnObject;
};


#endif