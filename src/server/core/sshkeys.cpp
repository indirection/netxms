/*
** NetXMS - Network Management System
** Copyright (C) 2021 Raden Solutions
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** File: sshkeys.cpp
**
**/

#include "nxcore.h"

#define MAX_SSH_KEY_NAME 128
static unsigned char s_sshHeader[11] = { 0x00, 0x00, 0x00, 0x07, 0x73, 0x73, 0x68, 0x2D, 0x72, 0x73, 0x61};

/**
 * SSH key data structure
 */
class SshKeyData
{
private:
   uint32_t m_id;
   TCHAR m_name[MAX_SSH_KEY_NAME];
   TCHAR *m_publicKey;
   char *m_privateKey;

public:
   static shared_ptr<SshKeyData> generateNewKeys(const TCHAR *name);

   SshKeyData();
   SshKeyData(NXCPMessage *msg, shared_ptr<SshKeyData> &data);
   SshKeyData(DB_RESULT result, int row);
   ~SshKeyData();

   void saveToDatabase();
   void deleteFromDatabase();

   uint32_t getId() const { return m_id; }
   const TCHAR *getName() const { return m_name; }
   const TCHAR *getPublicKey() const { return m_publicKey; }
   const char *getPrivateKey() const { return m_privateKey; }

};

static SynchronizedSharedHashMap<uint32_t, SshKeyData> s_sshKeys;

/**
 * Default constructor
 */
SshKeyData::SshKeyData()
{
   m_id = CreateUniqueId(IDG_SSH_KEYS);
   m_name[0] = 0;
   m_publicKey = nullptr;
   m_privateKey = nullptr;
}

/**
 * Constructor from NXCP message
 */
SshKeyData::SshKeyData(NXCPMessage *msg, shared_ptr<SshKeyData> &data)
{
   if (msg->getFieldAsInt32(VID_SSH_KEY_ID) != 0)
      m_id = msg->getFieldAsInt32(VID_SSH_KEY_ID);
   else
      m_id = CreateUniqueId(IDG_SSH_KEYS);
   msg->getFieldAsString(VID_NAME, m_name, MAX_SSH_KEY_NAME);
   if (msg->isFieldExist(VID_PRIVATE_KEY) || data == nullptr)
   {
      m_publicKey = msg->getFieldAsString(VID_PUBLIC_KEY);
      m_privateKey = msg->getFieldAsMBString(VID_PRIVATE_KEY);
   }
   else
   {
      m_publicKey = MemCopyString(data->m_publicKey);
      m_privateKey = MemCopyStringA(data->m_privateKey);
   }
}

/**
 * Constructor form DB result
 */
SshKeyData::SshKeyData(DB_RESULT result, int row)
{
   m_id = DBGetFieldULong(result, row, 0);
   DBGetField(result, row, 1, m_name, MAX_SSH_KEY_NAME);
   m_publicKey = DBGetField(result, row, 2, nullptr, 0);
   m_privateKey = DBGetFieldA(result, row, 3, nullptr, 0);
}

/**
 * SSH key data destructor
 */
SshKeyData::~SshKeyData()
{
   MemFree(m_publicKey);
   MemFree(m_privateKey);
}

/**
 * Encode public key parts
 */
static int SshEncodeBuffer(unsigned char *encoding, int bufferLen, unsigned char* buffer)
{
   int adjustedLen = bufferLen, index;
   if (*buffer & 0x80)
   {
      adjustedLen++;
      encoding[4] = 0;
      index = 5;
   }
   else
   {
      index = 4;
   }
   encoding[0] = (unsigned char) (adjustedLen >> 24);
   encoding[1] = (unsigned char) (adjustedLen >> 16);
   encoding[2] = (unsigned char) (adjustedLen >>  8);
   encoding[3] = (unsigned char) (adjustedLen);
   memcpy(&encoding[index], buffer, bufferLen);
   return index + bufferLen;
}

/**
 * Generate SSH key
 */
shared_ptr<SshKeyData> SshKeyData::generateNewKeys(const TCHAR *name)
{
   shared_ptr<SshKeyData> data = make_shared<SshKeyData>();
   _tcsncpy(data->m_name, name, MAX_SSH_KEY_NAME);
   RSA *key = RSAGenerateKey(4056);
   if(key == nullptr)
   {
      nxlog_write(NXLOG_ERROR, _T("SshKeyData::generateNewKeys: failed to generate RSA key"));
      RSA_free(key);
      return shared_ptr<SshKeyData>();
   }
   //Format private key to PKCS#1 and encode to base64
   StringBuffer privateKey;
   privateKey.append(_T("-----BEGIN RSA PRIVATE KEY-----\n"));
   char buf[4096];
   BYTE *pBufPos = reinterpret_cast<BYTE *>(buf);
   uint32_t privateLen = i2d_RSAPrivateKey(key, &pBufPos);
   base64_encode_alloc(buf, privateLen, &data->m_privateKey);
   privateKey.appendMBString(data->m_privateKey, strlen(data->m_privateKey), CP_UTF8);
   privateKey.append(_T("\n-----END RSA PRIVATE KEY-----"));
   MemFree(data->m_privateKey);
   data->m_privateKey = privateKey.getUTF8String();


   //Format public key
   // reading the modulus
   const BIGNUM *n, *e;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
   n = pRsa->n;
   e = pRsa->e;
#else
   const BIGNUM *d;
   RSA_get0_key(key, &n, &e, &d);
#endif
   if(n == nullptr || e == nullptr)
   {
      nxlog_write(NXLOG_ERROR, _T("SshKeyData::generateNewKeys: failed to generate RSA key components"));
      RSA_free(key);
      return shared_ptr<SshKeyData>();
   }
   int nLen = BN_num_bytes(n);
   unsigned char* nBytes = (unsigned char*) MemAlloc(nLen);
   BN_bn2bin(n, nBytes);

   // reading the public exponent
   int eLen = BN_num_bytes(e);
   unsigned char *eBytes = (unsigned char*) MemAlloc(eLen);
   BN_bn2bin(e, eBytes);

   int encodingLength = 11 + 4 + eLen + 4 + nLen;
   // correct depending on the MSB of e and N
   if (eBytes[0] & 0x80)
      encodingLength++;
   if (nBytes[0] & 0x80)
      encodingLength++;

   unsigned char* encoding = (unsigned char*) MemAlloc(encodingLength);
   memcpy(encoding, s_sshHeader, 11);

   int index = SshEncodeBuffer(&encoding[11], eLen, eBytes);
   index = SshEncodeBuffer(&encoding[11 + index], nLen, nBytes);

   StringBuffer publicKey;
   publicKey.append(_T("ssh-rsa "));
   char *out;
   int len = base64_encode_alloc(reinterpret_cast<const char *>(encoding), encodingLength, &out);
   publicKey.appendMBString(out, len, CP_UTF8);

   MemFree(nBytes);
   MemFree(eBytes);
   MemFree(encoding);
   MemFree(out);

   publicKey.append(_T(" "));
   publicKey.append(name);
   data->m_publicKey = MemCopyString(publicKey.getBuffer());

   RSA_free(key);
   return data;
}

/**
 * Save SSH key data to database
 */
void SshKeyData::saveToDatabase()
{
   DB_HANDLE hdb = DBConnectionPoolAcquireConnection();
   static const TCHAR *columns[] = { _T("name"), _T("public_key"), _T("private_key"), nullptr };
   DB_STATEMENT stmt =  DBPrepareMerge(hdb, _T("ssh_keys"), _T("id"), m_id, columns);
   if (stmt != nullptr)
   {
      DBBind(stmt, 1, DB_SQLTYPE_VARCHAR, m_name, DB_BIND_STATIC);
      DBBind(stmt, 2, DB_SQLTYPE_VARCHAR, m_publicKey, DB_BIND_STATIC);
      DBBind(stmt, 3, DB_CTYPE_STRING, TStringFromUTF8String(m_privateKey), DB_BIND_DYNAMIC);
      DBBind(stmt, 4, DB_SQLTYPE_INTEGER, m_id);
      DBExecute(stmt);
      DBFreeStatement(stmt);
   }
}

/**
 * Delete SSH key from database
 */
void SshKeyData::deleteFromDatabase()
{
   DB_HANDLE hdb = DBConnectionPoolAcquireConnection();
   ExecuteQueryOnObject(hdb, m_id, _T("DELETE FROM ssh_keys WHERE id=?"));
}

/**
 * Data structure for fill message with SSH data
 */
struct FillMessageData
{
   uint32_t *baseId;
   NXCPMessage *msg;
   bool withPublicKey;
};

/**
 * Callback to fill message with SSH key data
 */
static EnumerationCallbackResult FillMessageSshKeys(const uint32_t &key, const shared_ptr<SshKeyData> &value, FillMessageData *data)
{
   uint32_t *baseId = data->baseId;
   NXCPMessage *msg = data->msg;
   msg->setField((*baseId)++, value->getId());
   msg->setField((*baseId)++, value->getName());
   if (data->withPublicKey)
      msg->setField((*baseId)++, value->getPublicKey());
   else
      (*baseId)++;
   (*baseId)+=2;

   return _CONTINUE;
}

/**
 * Get SSH list
 */
void FillMessageWithSshKeys(NXCPMessage *msg, bool withPublicKey)
{
   uint32_t baseId = VID_SSH_CERT_LIST_BASE;
   FillMessageData data;
   data.baseId = &baseId;
   data.msg = msg;
   data.withPublicKey = withPublicKey;
   s_sshKeys.forEach(FillMessageSshKeys, &data);
   msg->setField(VID_SSH_KEY_COUT, (baseId - VID_SSH_CERT_LIST_BASE) / 5);
}

/**
 * Load SSH keys
 */
void LoadSshKeys()
{
   DB_HANDLE hdb = DBConnectionPoolAcquireConnection();
   DB_RESULT result = DBSelect(hdb, _T("SELECT id,name,public_key,private_key FROM ssh_keys"));
   if (result != nullptr)
   {
      int count = DBGetNumRows(result);
      for (int i = 0; i < count; i++)
      {
         shared_ptr<SshKeyData> key = make_shared<SshKeyData>(result, i);
         s_sshKeys.set(key->getId(), key);
      }
      DBFreeResult(result);
   }
   else
   {
      nxlog_write(NXLOG_ERROR, _T("Error loading ssh keys configuration from database"));
   }
}

/**
 * Generate SSH key
 */
uint32_t GenerateSshKey(const TCHAR *name)
{
   shared_ptr<SshKeyData> key = SshKeyData::generateNewKeys(name);
   if (key != nullptr)
   {
      s_sshKeys.set(key->getId(), key);
      TCHAR threadKey[32];
      _sntprintf(threadKey, 32, _T("SshKey_%d"), key->getId());
      ThreadPoolExecuteSerialized(g_mainThreadPool, threadKey, key, &SshKeyData::saveToDatabase);
      NotifyClientSessions(NX_NOTIFY_SSH_KEY_DATA_CHAGED, 0);
      return RCC_SUCCESS;
   }
   else
   {
      return RCC_INTERNAL_ERROR;
   }
}

/**
 * Generate SSH key
 */
void CreateOrEditSshKey(NXCPMessage *msg)
{
   uint32_t id = msg->getFieldAsInt32(VID_SSH_KEY_ID);
   shared_ptr<SshKeyData> oldKey = shared_ptr<SshKeyData>();
   if (id != 0)
   {
      oldKey = s_sshKeys.getShared(id);
   }
   shared_ptr<SshKeyData> newKey = make_shared<SshKeyData>(msg, oldKey);
   TCHAR threadKey[32];
   _sntprintf(threadKey, 32, _T("SshKey_%d"), newKey->getId());
   ThreadPoolExecuteSerialized(g_mainThreadPool, threadKey, newKey, &SshKeyData::saveToDatabase);
   s_sshKeys.set(newKey->getId(), newKey);
   NotifyClientSessions(NX_NOTIFY_SSH_KEY_DATA_CHAGED, 0);
}

/**
 * Callback to find if deleted ssh key is used by node
 */
static bool CheckIfSshKeyIsUsed(NetObj *object, uint32_t *data)
{
   return static_cast<Node *>(object)->getSshKeyId() == (*data);
}

/**
 * Remove deleted SSH key from nodes configuration
 */
static void RemoveDeletedSshKeyFromNode(NetObj *object, uint32_t *data)
{
   if (static_cast<Node *>(object)->getSshKeyId() == (*data))
      static_cast<Node *>(object)->clearSshKey();
}

/**
 * Delete SSH key from database
 */
void DeleteSshKey(NXCPMessage *response, uint32_t id, bool forceDelete)
{
   shared_ptr<SshKeyData> key = s_sshKeys.getShared(id);
   if (key != nullptr)
   {
      SharedObjectArray<NetObj> *objects = g_idxNodeById.findAll(CheckIfSshKeyIsUsed, &id);

      if (objects->size() == 0 || forceDelete)
      {
         if(objects->size() > 0)
         {
            g_idxNodeById.forEach(RemoveDeletedSshKeyFromNode, &id);
         }
         TCHAR threadKey[32];
         _sntprintf(threadKey, 32, _T("SshKey_%d"), key->getId());
         ThreadPoolExecuteSerialized(g_mainThreadPool, threadKey, key, &SshKeyData::deleteFromDatabase);
         s_sshKeys.remove(key->getId());
         NotifyClientSessions(NX_NOTIFY_SSH_KEY_DATA_CHAGED, 0);
         response->setField(VID_RCC, RCC_SUCCESS);
      }
      else
      {
         response->setField(VID_NUM_NODES, objects->size());
         uint32_t base = VID_NODE_LIST_BASE;
         for (int i = 0; i < objects->size(); i ++)
            response->setField(base++, objects->get(i)->getId());
         response->setField(VID_RCC, RCC_SSH_KEY_IN_USE);
      }

      delete objects;
   }
   else
   {
      response->setField(VID_RCC, RCC_INVALID_SSH_KEY_ID);
   }
}

/**
 * Find SSH key by id and fill message with private key
 */
void FindSshKeyById(uint32_t id, NXCPMessage *msg)
{
   shared_ptr<SshKeyData> key = s_sshKeys.getShared(id);
   if (key != nullptr)
   {
      msg->setField(VID_PUBLIC_KEY, key->getPublicKey());
      nxlog_debug(2, _T("SSHSession::connect: priv %hs"), key->getPrivateKey());
      msg->setFieldFromUtf8String(VID_PRIVATE_KEY, key->getPrivateKey());
      msg->setField(VID_RCC, ERR_SUCCESS);
   }
   else
   {
      msg->setField(VID_RCC, ERR_INVALID_SSH_KEY_ID);
   }
}
