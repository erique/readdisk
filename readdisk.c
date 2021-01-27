
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <proto/exec.h>

void kprintf( const char* fmt, ... ); // debug.lib

uint8_t sector[512];

void DumpBuffer(const uint8_t* buffer, uint32_t size);

// readdisk.exe <device> <unit> <start> <length>

int main(const int argc, const char** argv)
{
    const char* deviceName = "scsi.device";
    uint32_t deviceUnit = 0;

    uint32_t sectorOffset = 0;
    uint32_t sectorCount = 1;

    if (argc > 1)
        deviceName = argv[1];

    if (argc > 2)
        deviceUnit = atoi(argv[2]);

    if (argc > 3)
        sectorOffset = atoi(argv[3]);

    if (argc > 4)
        sectorCount = atoi(argv[4]);

    kprintf("reading %ld sectors (offset %ld) from '%s/%ld'\n", sectorCount, sectorOffset, deviceName, deviceUnit);

    struct MsgPort* msgPort = CreateMsgPort();
    if (!msgPort)
    {
        kprintf("error msgPort\n");
        return -1;
    }

    struct IOStdReq* ioReq = CreateIORequest(msgPort, sizeof(struct IOStdReq));
    if (!ioReq)
    {
        kprintf("error ioReq\n");
        return -1;
    }

    int8_t error = OpenDevice(deviceName, deviceUnit, (struct IORequest *)ioReq, 0L);
    if (error)
    {
        kprintf("error device '%s/%ld' : %ld\n", deviceName, deviceUnit, (int)error);
        return -1;
    }

    for (int i = 0; i < sectorCount; ++i)
    {
        ioReq->io_Data    = sector;
        ioReq->io_Length  = sizeof(sector);
        ioReq->io_Offset  = sectorOffset + i*sizeof(sector);
        ioReq->io_Command = CMD_READ;

        DoIO((struct IORequest *) ioReq);

        if (!ioReq->io_Error)
        {
            DumpBuffer(sector, sizeof(sector));
        }
        else
        {
            kprintf("error reading : %ld\n", ioReq->io_Error);
            break;
        }
    }

    CloseDevice((struct IORequest *)ioReq);

    DeleteIORequest(ioReq);

    DeleteMsgPort(msgPort);

    return 0;
}

void DumpBuffer(const uint8_t* buffer, uint32_t size)
{
    uint32_t i, j, len;
    char format[150];
    char alphas[27];
    strcpy(format, "$%08lx: %02x%02x%02x%02x %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X ");

    for (i = 0; i < size; i += 16) {
        len = size - i;

        // last line is less than 16 bytes? rewrite the format string
        if (len < 16) {
            strcpy(format, "$%08X: ");

            for (j = 0; j < 16; ++j) {
                if (j < len) {
                    strcat(format, "%02X");

                } else {
                    strcat(format, "__");
                }

                if ((j & 0x3) == 0x3) {
                    strcat(format, " ");
                }
            }

        } else {
            len = 16;
        }

        // create the ascii representation
        for (j = 0; j < len; ++j) {
            alphas[j] = (isalnum(buffer[i + j]) ? buffer[i + j] : '.');
        }

        for (; j < 16; ++j) {
            alphas[j] = '_';
        }

        alphas[j] = 0;

        j = strlen(format);
        sprintf(format + j, "'%s'\n", alphas);

        kprintf(format, buffer+i,
              buffer[i + 0], buffer[i + 1], buffer[i + 2], buffer[i + 3], buffer[i + 4], buffer[i + 5], buffer[i + 6], buffer[i + 7],
              buffer[i + 8], buffer[i + 9], buffer[i + 10], buffer[i + 11], buffer[i + 12], buffer[i + 13], buffer[i + 14], buffer[i + 15]);

        format[j] = '\0';
    }
}
