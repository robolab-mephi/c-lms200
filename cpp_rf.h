#include <windows.h>
#include <winuser.h>
#include <math.h>
#include <stdio.h>

class LMS200{
public:
	LMS200(void);
	HANDLE create_connection();
	int close_connection();
	int full_scan(WORD * buf);
	DWORD read_port(unsigned char* buf);
	int read_byte(BYTE* buf);
	int calc_chksum(unsigned char *CommData, unsigned int uLen);
	int send_command(int cop);
	int send_command(int cop, BYTE argument);
	HANDLE m_h;
};
