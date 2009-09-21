#ifndef __DUPLICATESOCKET_H__
#define __DUPLICATESOCKET_H__

class DuplicateSocket
{
public:
	DuplicateSocket();
	~DuplicateSocket();

	bool Duplicate(SOCKET original);

	void GetTargetProcess();

	bool TransportToTarget();

private:
	bool bTransferred;
	DWORD dwProcessId;
	WSAPROTOCOL_INFO  WSAProtocolInfo;

	// for log
	HANDLE hLogFile;
};

#endif