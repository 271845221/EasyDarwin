/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyCMSSession.h
    Contains:   CMS Session
*/

#include "Task.h"
#include "TimeoutTask.h"

#include "QTSSModuleUtils.h"
#include "OSArrayObjectDeleter.h"
#include "OSMemory.h"
#include "QTSSMemoryDeleter.h"
#include "OSRef.h"
#include "StringParser.h"
#include "MyAssert.h"

#include "QTSServerInterface.h"
#include "HTTPProtocol.h"
#include "OSHeaders.h"
#include "QTSS.h"
#include "SocketUtils.h"
#include "EasyProtocol.h"

#include "HTTPRequestStream.h"
#include "HTTPResponseStream.h"
#include "HTTPRequest.h"

using namespace EasyDarwin::Protocol;
using namespace std;

#ifndef __EASY_CMS_SESSION__
#define __EASY_CMS_SESSION__

class EasyCMSSession : public Task
{
public:
    EasyCMSSession();
    virtual ~EasyCMSSession();

	static void Initialize(QTSS_ModulePrefsObject inPrefs);
    
	enum
	{
		kSessionOffline		= 0,	
		kSessionOnline		= 1
	};
	typedef UInt32	SessionStatus;

	SessionStatus GetSessionStatus() { return fSessionStatus; }
	void SetSessionStatus(SessionStatus status) { fSessionStatus = status; }

	// 更新最新快照缓存
	QTSS_Error UpdateSnapCache(Easy_PostSnap_Params* params);

private:
    virtual SInt64 Run();

	// 初步判断Session Socket是否已连接
	Bool16 isConnected() { return fSocket->GetSocket()->IsConnected(); }

	// transfer error code for http status code
	size_t getStatusNo(QTSS_Error errNo);

	void cleanupRequest();

	// 设备注册到EasyCMS
	QTSS_Error doDSRegister();

	// 上传快照图片到EasyCMS
	QTSS_Error doDSPostSnap();

	// 处理HTTPRequest请求报文
	QTSS_Error processMessage();

	// 重置客户端参数
	void resetClientSocket();

private:
	enum
	{
		kIdle = 0,
		kReadingMessage = 1,
		kProcessingMessage = 2,
		kSendingMessage = 3,
		kCleaningUp = 4
	};
	UInt32 fState;

	SessionStatus	fSessionStatus;

	TimeoutTask fTimeoutTask;
	ClientSocket* fSocket;

	// 为CMSSession专门进行网络数据包读取的对象
	HTTPRequestStream   fInputStream;
	// 为CMSSession专门进行网络数据包发送的对象
	HTTPResponseStream  fOutputStream;

	// 初始化时为NULL
	// 在每一次请求发出或者接收命令时,都有可能生成HTTPRequest对象并进行处理
	// 每一次状态机流程在处理完成kIdle~kCleanUp的流程都需要清理HTTPRequest对象
	HTTPRequest*        fRequest;

	// 读取网络报文前先锁住Session防止重入读取
	OSMutex             fReadMutex;

	// Session锁
	OSMutex             fMutex;

	// 请求报文的Content部分
	char*				fContentBuffer;

	// 请求报文的Content读取偏移量,在多次读取到完整Content部分时用到
	UInt32				fContentBufferOffset;

	EasyMsgDSPostSnapREQ* fSnapReq;

	// send message count
	unsigned int fSendMessageCount;

	size_t fCSeqCount;

};

#endif

