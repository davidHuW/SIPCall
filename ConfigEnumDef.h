
#pragma once


#define SEND_BUFFER_SZ	(8)
#define USER_BUFFER_SZ	(16)
#define TRANS_BUFFER_SZ	(16)


enum enEventStatus{
	ENUM_EVENT_NONE,
	ENUM_EVENT_IDLE,	// 空闲
	ENUM_EVENT_BUSY,	// 忙
	ENUM_EVENT_RING,	// 响铃
	ENUM_EVENT_ALERT,   // 回铃
	ENUM_EVENT_ANSWER,	// 被叫同意和主叫建立通话
	ENUM_EVENT_ANSEERED,// 主叫检查到被叫同意建立通话
	ENUM_EVENT_BYE,		// 挂断
	ENUM_EVENT_NOANSWER,// 无人接听
	ENUM_EVENT_ONLINE,	// 在线
	ENUM_EVENT_OFFLINE,	// 离线

	ENUM_EVENT_DIVERT,   //呼叫转移
	ENUM_EVENT_ALL,
};

enum enCallStatus{
	ENUM_CALL_NONE = 0,
	ENUM_CALL_REQ ,
	ENUM_CALL_WAIT_E1,
	ENUM_CALL_E1,
	ENUM_CALL_WAIT_RING,
	ENUM_CALL_RING,
	ENUM_CALL_WAIT_ANSWER,
	ENUM_CALL_ANSWER,
	ENUM_CALL_WAIT_E2,
	ENUM_CALL_E2,
	ENUM_CALL_WAIT_OPEN_ACK,
	ENUM_CALL_OPEN_ACK,
};
enum enBCStatus2Master
{
	ENUM_BC_IS_USING = 1,
	ENUM_BC_IS_USABLE = 2,
	ENUM_BC_IS_REMOTE_USING = 3,
};
enum enCallStatus2Master
{
	ENUM_CALL_IS_USABLE = 1,
	ENUM_CALL_IS_OFFLINIE = 2,
	ENUM_CALL_IS_OPENED = 3,
	ENUM_CALL_IS_RING = 4,
	ENUM_CALL_IS_USING = 5,
	
	ENUM_CALL_IS_SIP_DOWN = 6,
	ENUM_CALL_IS_OM_DOWN = 7,
};
enum enCallReqFromMaster
{
	ENUM_CALL_OPEN_REQ = 1,
	ENUM_CALL_CLOSE_REQ = 2,
	ENUM_CALL_DIVERT_OPEN_REQ = 4,
};

 enum enBCReqFromMaster
 {
	 ENUM_BC_OPEN_REQ = 1,
	 ENUM_BC_CLOSE_REQ = 2,
 };
enum enBCStatus{
	ENUM_BC_NONE = 0,
	ENUM_BC_REQ ,
	ENUM_BC_WAIT_E3,
	ENUM_BC_E3,
	ENUM_BC_WAIT_E4,
	ENUM_BC_E4,
};

enum enTempGroupID{
	ENUM_TEMP_GROUP_ID_NONE = 0,
	ENUM_TEMP_GROUP_ID_AUX = 7,
	ENUM_TEMP_GROUP_ID_MIC = 9,
	ENUM_TEMP_GROUP_ID_ALL,
};

enum enPlaySrc{
	ENUM_MODEL_NONE = 0,
	ENUM_MODEL_FILE,
	ENUM_MODEL_FILE_LIST,
	ENUM_MODEL_TTS,
	ENUM_MODEL_MIC,	
	ENUM_MODEL_AUX,
};
enum enPlayModel{
	ENUM_PLAY_MODEL_NONE = 0,
	ENUM_PLAY_MODEL_ONE_ONCE,		// 单曲结束
	ENUM_PLAY_MODEL_ONE_CONTINUE,	// 单曲循环
	ENUM_PLAY_MODEL_LIST,			// 列表循环
	ENUM_PLAY_MODEL_LIST_ONCE,		// 顺序列表
};
enum enAdminType{
	ENUM_TYPE_NONE,
	ENUM_TYPE_SUPER,
	ENUM_TYPE_ADMIN,
	ENUM_TYPE_USER,	
	ENUM_TYPE_ALL,
};

enum enDeviceStatus{
	ENUM_DEVICE_STATUS_UNKNOWN,		// 未知状态

	ENUM_DEVICE_STATUS_OK,			// 待机状态	
	ENUM_DEVICE_STATUS_RUN,			// 使用状态

	ENUM_DEVICE_STATUS_RING,		// 拨号状态


	ENUM_DEVICE_STATUS_ERROR,		// 巡检故障

	ENUM_DEVICE_STATUS_REMOTE_USING,  //远端控制器PC正在使用该分机
	ENUM_DEVICE_STATUS_OUTER_USING,
	ENUM_DEVICE_OM_DOWN,
};


enum enErrorCode{
	ENUM_ERROR_STATUS_NONE = 0,


	ENUM_ERROR_STATUS_UNKNOWN,
	ENUM_ERROR_STATUS_MIC,			
	ENUM_ERROR_STATUS_DOOR,
	ENUM_ERROR_STATUS_BUTTON,

	ENUM_ERROR_STATUS_MIC_DOOR,			
	ENUM_ERROR_STATUS_DOOR_BTN,
	ENUM_ERROR_STATUS_MIC_BTN,
	ENUM_ERROR_STATUS_MIC_DOOR_BTN,

	ENUM_ERROR_STATUS_ALL,
};

enum enConfigErrorCode{
	ENUM_CONFIG_ERROR_NONE = 0,
	ENUM_CONFIG_ERROR_PLAY_SRC,
	ENUM_CONFIG_ERROR_PLAY_MODEL,
	ENUM_CONFIG_ERROR_DEVICE_CHECK,
	ENUM_CONFIG_ERROR_FILE_PATH,
	ENUM_CONFIG_ERROR_TTS_PATH,
	ENUM_CONFIG_ERROR_PLAYLIST,
	ENUM_CONFIG_ERROR_ALL,
};

enum enCmdCode{
	ENUM_CMD_CODE_NONE = 0,
	ENUM_CMD_CODE_RING,
	ENUM_CMD_CODE_CALL_START,
	ENUM_CMD_CODE_CALL_STOP,
	ENUM_CMD_CODE_RESERVE,
	ENUM_CMD_CODE_BC_OPEN,
	ENUM_CMD_CODE_BC_CLOSE,
	ENUM_CMD_CODE_ERROR,
	ENUM_CMD_CODE_ALL,
};

enum enStopMsg{
	ENUM_STOP_MSG_NONE = 0,
	ENUM_STOP_MSG_TIME,
	ENUM_STOP_MSG_CLICK,
	ENUM_STOP_MSG_CHANGE,
};

enum enSendBcMsg{
	ENUM_SEND_BC_NONE = 0,
	ENUM_SEND_BC_STATUS_OPEN,
	ENUM_SEND_BC_STATUS_CLOSE,
	ENUM_SEND_BC_OPT_OPEN,
	ENUM_SEND_BC_OPT_CLOSE,
	ENUM_SEND_CALL_OPEN,
	ENUM_SEND_CALL_CLOSE,
};
enum enBCOptStatus{
	ENUM_BC_OPT_NONE = 0,
	ENUM_BC_OPT_REMOTE,		//远程操作
	ENUM_BC_OPT_LOCAL,		//本地操作
};

