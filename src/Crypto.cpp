#include "stdafx.h"
#include "Crypto.h"

//	Tell the linker to link the to the Cryptography API.
#pragma comment(lib, "Advapi32.lib")

using namespace MFC;

//	Constructor, intialises Crypto API.
CCrypto::CCrypto() : m_hCryptProv(NULL), m_hKey(NULL), m_hHash(NULL)
{
	//	Create the Crypt context.
	if(!::CryptAcquireContext(&m_hCryptProv, NULL, NULL, PROV_RSA_FULL, 0))  {
		if (::GetLastError() == NTE_BAD_KEYSET)
		{
			if (!::CryptAcquireContext(&m_hCryptProv, 
				NULL,
				NULL, 
				PROV_RSA_FULL, 
				CRYPT_NEWKEYSET))
			{
				return;
			}
		} else {
			return;
		}
	}

	//	Create an empty hash object.
	if(!::CryptCreateHash(m_hCryptProv, CALG_MD5, 0, 0, &m_hHash)) 
		return;

	//	Memory files are opened automatically on construction, we don't
	//	explcitly call open.
}

//	Destructor, frees Crypto stuff.
CCrypto::~CCrypto()
{
	//	Close the file.
	m_file.Close();

	// Clean up.
	if(m_hHash)
		::CryptDestroyHash(m_hHash);

	if(m_hKey)
		::CryptDestroyKey(m_hKey);

	if(m_hCryptProv)
		::CryptReleaseContext(m_hCryptProv, 0);
}

//	Derive a key from a password.
bool CCrypto::DeriveKey(CString strPassword)
{
	//	Return failure if we don't have a context or hash.
	if(m_hCryptProv == NULL || m_hHash == NULL)
		return false;

	//	If we already have a hash, trash it.
	if(m_hHash)
	{
		CryptDestroyHash(m_hHash);
		m_hHash = NULL;
		if(!CryptCreateHash(m_hCryptProv, CALG_MD5, 0, 0, &m_hHash)) 
			return false;
	}

	//	If we already have a key, destroy it.
	if(m_hKey)
	{
		::CryptDestroyKey(m_hKey);
		m_hKey = NULL;
	}

	//	Hash the password. This will have a different result in UNICODE mode, as it
	//	will hash the UNICODE string (this is by design, allowing for UNICODE passwords, but
	//	it's important to be aware of this behaviour.
	if(!CryptHashData(m_hHash, (const BYTE*)strPassword.GetString(), strPassword.GetLength() * sizeof(TCHAR), 0)) 
		return false;
	
	//	Create a session key based on the hash of the password.
	if(!CryptDeriveKey(m_hCryptProv, CALG_RC2, m_hHash, CRYPT_EXPORTABLE, &m_hKey))
		return false;

	//	And we're done.
	return true;
}

bool CCrypto::Encrypt(const CObject& serializable, CByteArray& arData)
{
	//	Return failure if we don't have a context or key.
	if(m_hCryptProv == NULL || m_hKey == NULL)
		return false;

	//	Return failure if the object is not serializable.
	if(serializable.IsSerializable() == FALSE)
		return false;

	//	Before we write to the file, trash it.
	m_file.SetLength(0);

	//	Create a storing archive from the memory file.
	CArchive ar(&m_file, CArchive::store);

	//	We know that serialzing an object will not change it's data, as we can
	//	safely use a const cast here.

	//	Write the data to the memory file.
	const_cast<CObject&>(serializable).Serialize(ar);
	
	//	Close the archive, flushing the write.
	ar.Close();

	//	Encrypt the contents of the memory file and store the result in the array.
	return InternalEncrypt(arData);
}

bool CCrypto::Decrypt(const CByteArray& arData, CObject& serializable)
{
	//	Return failure if we don't have a context or key.
	if(m_hCryptProv == NULL || m_hKey == NULL)
		return false;

	//	Return failure if the object is not serializable.
	if(serializable.IsSerializable() == FALSE)
		return false;

	//	Decrypt the contents of the array to the memory file.
	if(InternalDecrypt(arData) == false)
		return false;

	//	Create a reading archive from the memory file.
	CArchive ar(&m_file, CArchive::load);

	//	Read the data from the memory file.
	serializable.Serialize(ar);
	
	//	Close the archive.
	ar.Close();

	//	And we're done.
	return true;
}

bool CCrypto::Encrypt(const CString& str, CByteArray& arData)
{
	//	Return failure if we don't have a context or key.
	if(m_hCryptProv == NULL || m_hKey == NULL)
		return false;

	//	Before we write to the file, trash it.
	m_file.SetLength(0);

	//	Create a storing archive from the memory file.
	CArchive ar(&m_file, CArchive::store);

	//	Write the string to the memory file.
	ar << str;

	//	Close the archive, flushing the write.
	ar.Close();

	//	Encrypt the contents of the memory file and store the result in the array.
	return InternalEncrypt(arData);
}

bool CCrypto::Decrypt(const CByteArray& arData, CString& str)
{
	//	Return failure if we don't have a context or key.
	if(m_hCryptProv == NULL || m_hKey == NULL)
		return false;

	//	Decrypt the contents of the array to the memory file.
	if(InternalDecrypt(arData) == false)
		return false;

	//	Create a reading archive from the memory file.
	CArchive ar(&m_file, CArchive::load);

	//	Read the data from the memory file.
	ar >> str;
	
	//	Close the archive.
	ar.Close();

	//	And we're done.
	return true;
}

bool CCrypto::InternalEncrypt(CByteArray& arDestination)
{
	//	Get the length of the data in memory. Increase the capacity to handle the size of the encrypted data.
	ULONGLONG uLength = m_file.GetLength();
	ULONGLONG uCapacity = uLength * 2;
	m_file.SetLength(uCapacity);

	//	Acquire direct access to the memory.
	BYTE* pData = m_file.Detach();

	//	We need a DWORD to tell encrypt how much data we're encrypting.
	DWORD dwDataLength = static_cast<DWORD>(uLength);

	//	Now encrypt the memory file.
	if(!::CryptEncrypt(m_hKey, NULL, TRUE, 0, pData, &dwDataLength, static_cast<DWORD>(uCapacity)))
	{
		//	Free the memory we release from the memory file.
		delete [] pData;

		return false;
	}	

	//	Assign all of the data we have encrypted to the byte array- make sure anything 
	//	already in the array is trashed first.
	arDestination.RemoveAll();
	arDestination.SetSize(static_cast<INT_PTR>(dwDataLength));
	memcpy(arDestination.GetData(), pData, dwDataLength);

	//	Free the memory we release from the memory file.
	delete [] pData;

	return true;
}

bool CCrypto::InternalDecrypt(const CByteArray& arSource)
{
	//	Trash the file.
	m_file.SetLength(0);

	//	Write the contents of the byte array to the memory file.
	m_file.Write(arSource.GetData(), static_cast<UINT>(arSource.GetCount()));
	m_file.Flush();

	//	Acquire direct access to the memory file buffer.
	BYTE* pData = m_file.Detach();

	//	We need a DWORD to tell decrpyt how much data we're encrypting.
	DWORD dwDataLength = static_cast<DWORD>(arSource.GetCount());
	DWORD dwOldDataLength = dwDataLength;

	//	Now decrypt the data.
	if(!::CryptDecrypt(m_hKey, NULL, TRUE, 0, pData, &dwDataLength))
	{
		//	Free the memory we release from the memory file.
		delete [] pData;

		return false;
	}

	//	Set the length of the data file, write the decrypted data to it.
	m_file.SetLength(dwDataLength);
	m_file.Write(pData, dwDataLength);
	m_file.Flush();
	m_file.SeekToBegin();

	//	Free the memory we release from the memory file.
	delete [] pData;

	return true;
}