#include <NfcTag.h>
#include <string.h>
#include <PN532_debug.h>

NfcTag::NfcTag()
{
    _uid = 0;
    _uidLength = 0;
    _tagType = "Unknown";
    _ndefMessage = (NdefMessage*)NULL;
}

NfcTag::NfcTag(uint8_t *uid, unsigned int uidLength)
{
    _uid = uid;
    _uidLength = uidLength;
    _tagType = "Unknown";
    _ndefMessage = (NdefMessage*)NULL;
}

NfcTag::NfcTag(uint8_t *uid, unsigned int  uidLength, string tagType)
{
    _uid = uid;
    _uidLength = uidLength;
    _tagType = tagType;
    _ndefMessage = (NdefMessage*)NULL;
}

NfcTag::NfcTag(uint8_t *uid, unsigned int  uidLength, string tagType, NdefMessage& ndefMessage)
{
    _uid = uid;
    _uidLength = uidLength;
    _tagType = tagType;
    _ndefMessage = new NdefMessage(ndefMessage);
}

// I don't like this version, but it will use less memory
NfcTag::NfcTag(uint8_t *uid, unsigned int uidLength, string tagType, const uint8_t *ndefData, const int ndefDataLength)
{
    _uid = uid;
    _uidLength = uidLength;
    _tagType = tagType;
    _ndefMessage = new NdefMessage(ndefData, ndefDataLength);
}

NfcTag::~NfcTag()
{
    delete _ndefMessage;
}

NfcTag& NfcTag::operator=(const NfcTag& rhs)
{
    if (this != &rhs)
    {
        delete _ndefMessage;
        _uid = rhs._uid;
        _uidLength = rhs._uidLength;
        _tagType = rhs._tagType;
        // TODO do I need a copy here?
        _ndefMessage = rhs._ndefMessage;
    }
    return *this;
}

uint8_t NfcTag::getUidLength()
{
    return _uidLength;
}

void NfcTag::getUid(uint8_t *uid, unsigned int uidLength)
{
    memcpy(_uid, uid, uidLength);
}

string NfcTag::getUidString()
{
    string uidString = "";

    char buffer[4];
    buffer[3]='\0';

    for (int i = 0; i < _uidLength; i++)
    {
        sprintf(buffer, "%02X-", _uid[i]);
        uidString.append(buffer);
    }

    uidString.pop_back();
    
    return uidString;
}

string NfcTag::getTagType()
{
    return _tagType;
}

bool NfcTag::hasNdefMessage()
{
    return (_ndefMessage != NULL);
}

NdefMessage NfcTag::getNdefMessage()
{
    return *_ndefMessage;
}

void NfcTag::print()
{
    DMSG("NFC Tag - ");
    DMSG_INT(_tagType);
    DMSG("UID - ");
    DMSG(getUidString().c_str());
    if (_ndefMessage == NULL)
    {
        DMSG("\nNo NDEF Message");
    }
    else
    {
        _ndefMessage->print();
    }
}
