#include "cpp_rf.h" 

void PrintError( LPCSTR str)
{
	LPVOID lpMessageBuffer;
	int error = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
		(LPTSTR) &lpMessageBuffer,
		0,
		NULL
		);
//	printf("%s: (%d) %s\n\n",str,error,lpMessageBuffer);
	LocalFree( lpMessageBuffer );
}

LMS200::LMS200()
{
	m_h = NULL;
}
HANDLE LMS200::create_connection()
{
	HANDLE h = CreateFile("COM2",
		GENERIC_READ|GENERIC_WRITE,
		0,NULL,	
		OPEN_EXISTING,0,NULL);

	if(h == INVALID_HANDLE_VALUE) 
	{
		PrintError("E012_Failed to open port");
	} 
	else 
	{
		// set timeouts
		COMMTIMEOUTS cto = { 1, 100, 1000, 0, 0 };
		DCB dcb;
		if(!SetCommTimeouts(h,&cto))
		{
			PrintError("E013_SetCommTimeouts failed");
			return INVALID_HANDLE_VALUE;
		}

		// set DCB
		memset(&dcb,0,sizeof(dcb));
		dcb.DCBlength = sizeof(dcb);
		dcb.BaudRate = 9600;
		dcb.fBinary = 1;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
		// dcb.fOutxCtsFlow = 1;
		// dcb.fRtsControl = DTR_CONTROL_HANDSHAKE;

		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.ByteSize = 8;

		if(!SetCommState(h,&dcb))
		{
			PrintError("E014_SetCommState failed");
			return INVALID_HANDLE_VALUE;
		}
	}

	return m_h=h;
}

DWORD LMS200::read_port(unsigned char* buf)
{
	DWORD read;
	ReadFile(m_h,buf,4,&read,NULL); // read is updated with the number of bytes read
/*	printf("Reading: ");
	for (int i=0; i<read; i++)
		printf("%i ", (unsigned char)buf[i]);
	printf("\n");
*/
	if ((read != 4) || (buf[0] != 0x02) || (buf[1] != 0x80)) return 0;
	ReadFile(m_h,buf+4,(unsigned int)buf[2] + buf[3] * 256 + 2,&read,NULL);

/*	printf("Reading: ");
	for (int i=0; i<read + 4; i++)
		printf("%i ", (unsigned char)buf[i]);
	printf("\n");*/
	return read + 4;
}

int LMS200::read_byte(BYTE * buf)
{
	DWORD read;
	ReadFile(m_h,buf,1,&read,NULL); // read is updated with the number of bytes read
//	if (read == 1) printf("Reading byte: %i\n", *buf);
	return read;
}

int LMS200::calc_chksum(unsigned char *CommData, unsigned int uLen)
{
	unsigned short uCrc16;
	unsigned char abData[2];
	uCrc16 = 0;
	abData[0] = 0;
	while (uLen-- )
	{
		abData[1] = abData[0];
		abData[0] = *CommData++;
		if(uCrc16 & 0x8000)
		{
			uCrc16 = (uCrc16 & 0x7fff) << 1;
			uCrc16 ^= 0x8005;
		}
		else
		{
			uCrc16 <<= 1;
		}
		uCrc16 ^= ((unsigned short) (abData[0]) | ((unsigned short)(abData[1]) << 8));
	}
	return(uCrc16);
}

int LMS200::send_command(int cop)
{
	unsigned char command[200];			//!!!!!!!!!!!!!!!!!!
	DWORD index = 0;
	command[index++] = 0x02;
    command[index++] = 0x00;
	// set command length
	command[index++] = 1;
	command[index++] = 0;
	// set command number
	command[index++] = (char)cop;
	// set argument
	// set checksum
	int checksum = calc_chksum(command, index);
	command[index++] = (char)(checksum%256 & 0xff);
	command[index++] = (char)(checksum/256 & 0xff);

	WriteFile(m_h,command,index,&index,NULL); // write is updated with the number of bytes written

	/* ============= log ==========================*/
	//printf("Writing ");
	//for (unsigned int i=0; i<index; i++)
	//	printf("%i ", (unsigned char)command[i]);
	//printf("\n");
	/*================ log ==========================*/

	BYTE bf = 0;
	read_byte(&bf);
	if (bf != 0x06) printf("Error: LMS returned %i\n", bf);

	return bf;
}

int LMS200::send_command(int cop, BYTE argument)
{
	unsigned char command[200];			//!!!!!!!!!!!!!!!!!!
	DWORD index = 0;
	command[index++] = 0x02;
    command[index++] = 0x00;
	// set command length
	command[index++] = 2;
	command[index++] = 0;
	// set command number
	command[index++] = (char)cop;
	// set argument
	command[index++] = argument;
	// set checksum
	int checksum = calc_chksum(command, index);
	command[index++] = (char)(checksum%256 & 0xff);
	command[index++] = (char)(checksum/256 & 0xff);

	WriteFile(m_h,command,index,&index,NULL); // write is updated with the number of bytes written

	/* ============= log ==========================*/
	//printf("Writing ");
	//for (unsigned int i=0; i<index; i++)
	//	printf("%i ", (unsigned char)command[i]);
	//printf("\n");
	/*================ log ==========================*/

	BYTE bf = 0;
	read_byte(&bf);
	if (bf != 0x06) printf("Error: LMS returned %i\n", bf);

	return bf;
}

int LMS200::close_connection()
{
	if(m_h!=NULL)
	{
		CloseHandle(m_h);
	}
	return 0;
}

int LMS200::full_scan(WORD * buf)
{
	int rez = send_command(0x30, 1);
	if (rez != 0x06) return -1;
	BYTE buf2[1000];
	Sleep(100);
	int read = read_port(buf2);
	if (read >= 7)
	{
		int i=0;
		for (i = 0; i < (buf2[2] + buf2[3] * 256 - 2)/2; i++)
			buf[i] = buf2[5+i*2] + buf2[6+i*2]*256;
		return i;
	}
	return -2;
}
