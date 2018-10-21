#include "mbed.h"

#include "PN532_SPI.h"
#include "PN532.h"
#include "NfcAdapter.h"

#include "SWO.h"

#include "utilities.h"

static DigitalOut led1(LED1);
static InterruptIn btn(BUTTON1);

SWO_Channel swo("channel");

uint8_t ndefBuf[128];

SPI spi(PB_15, PB_14, PB_13);
PN532_SPI pn532spi(spi, PB_2);
NfcAdapter nfc(pn532spi);

static Thread s_thread_manage_nfc;
static EventQueue s_eq_manage_nfc;

#define NFC_EVENT_PROC_CYCLE_INTERVAL_MSECS 500

// Main
static EventQueue s_eq_main;

static std::string s_latest_tag_uid_detected;
static bool s_latest_tag_present;
static Mutex s_latest_tag_mutex;

static void on_tag_detected()
{
    s_latest_tag_mutex.lock();
    swo.printf("### NFC TAG DETECTED : %s ###\n", s_latest_tag_uid_detected.c_str());
    s_latest_tag_mutex.unlock();
}

static void on_tag_undetected()
{
    s_latest_tag_mutex.lock();
    swo.printf("### NFC TAG REMOVED ###\n");
    s_latest_tag_mutex.unlock();
}

static void nfc_event_proc()
{
    bool tagPresent = nfc.tagPresent();

    s_latest_tag_mutex.lock();
    bool latest_tag_detected = s_latest_tag_present;
    s_latest_tag_mutex.unlock();

    if (!latest_tag_detected && tagPresent)
    {
        NfcTag tag = nfc.read();

        s_latest_tag_mutex.lock();
        s_latest_tag_present=true;
        s_latest_tag_uid_detected=tag.getUidString();
        s_latest_tag_mutex.unlock();

        s_eq_main.call(on_tag_detected);
    }
    else if(latest_tag_detected && !tagPresent)
    {
        s_latest_tag_mutex.lock();
        s_latest_tag_present=false;
        s_latest_tag_uid_detected="";
        s_latest_tag_mutex.unlock();

        s_eq_main.call(on_tag_undetected);
    }
}

static void dumpTagData(NfcTag tag)
{
    if (tag.hasNdefMessage())
    {
        NdefMessage ndefmsg = tag.getNdefMessage();

        int recordscount = ndefmsg.getRecordCount();

        swo.printf("\tFound %d NDEF Records:\n", recordscount);

        for (int i = 0; i < recordscount; i++)
        {
            NdefRecord record = ndefmsg.getRecord(i);

            // Id
            string idstring = "<EMPTY>";

            auto idlen = record.getIdLength();

            if (idlen != 0)
            {
                auto idBuffer = new uint8_t[idlen];
                record.getId(idBuffer);
                idstring = getByteArrayHexString(idBuffer, idlen);
                delete[] idBuffer;
            }

            // Type
            string typestring = "<EMPTY>";
            string typedecodedstring = "?";

            auto typelen = record.getTypeLength();

            if (typelen != 0)
            {
                auto typeBuffer = new uint8_t[typelen];
                record.getType(typeBuffer);
                typestring = getByteArrayHexString(typeBuffer, typelen);
                typedecodedstring = getByteArrayString(typeBuffer, typelen);
                delete[] typeBuffer;
            }

            // Payload
            string payloadstring = "<EMPTY>";
            string payloaddecodedstring = "?";
            string payloaddecodedstring_prefix = "";
            string payloaddecodedstring_data = "";
            uint8_t payloadprefixlen = 0;

            auto payloadlen = record.getPayloadLength();

            if (payloadlen != 0)
            {
                auto payloadBuffer = new uint8_t[payloadlen];
                record.getPayload(payloadBuffer);
                payloadstring = getByteArrayHexString(payloadBuffer, payloadlen);

                if (typedecodedstring == "T" || typedecodedstring == "U")
                {
                    payloadprefixlen = payloadBuffer[0];
                    payloaddecodedstring = getByteArrayString(payloadBuffer + 1, payloadlen - 1);
                }
                else
                {
                    payloaddecodedstring = getByteArrayString(payloadBuffer, payloadlen);
                }

                if (payloadprefixlen > 0)
                {
                    payloaddecodedstring_prefix = payloaddecodedstring.substr(0, payloadprefixlen);
                    payloaddecodedstring_data = payloaddecodedstring.substr(payloadprefixlen);
                }
                else
                {
                    payloaddecodedstring_prefix = "<EMPTY>";
                    payloaddecodedstring_data = payloaddecodedstring;
                }

                delete[] payloadBuffer;
            }

            swo.printf("\t\tRecord: TNF=%d, ID=%s, TYPE=%s (%s), PAYLOAD=%s (Prefix:%s, Data:%s)\n",
                       record.getTnf(),
                       idstring.c_str(),
                       typestring.c_str(), typedecodedstring.c_str(),
                       payloadstring.c_str(), payloaddecodedstring_prefix.c_str(), payloaddecodedstring_data.c_str());
        }
    }
}

int main()
{
    swo.printf("[INFO] Initializing PN532...");

    nfc.begin();

    swo.printf("[INFO] ...done.\n");

    s_eq_manage_nfc.call_every(NFC_EVENT_PROC_CYCLE_INTERVAL_MSECS, nfc_event_proc);
    s_thread_manage_nfc.start(callback(&s_eq_manage_nfc, &EventQueue::dispatch_forever));

    swo.printf("[INFO] ...running.\n");

    s_eq_main.dispatch_forever();
}