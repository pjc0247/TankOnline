enum PROTOCOL {
	HELLO=100,

	FILE_LENGTH = 101,
	FILE_NAME = 102,
	FILE_ERR = 103,

	LOGIN=200,
	LOGIN_ACCEPT=201,
	LOGIN_DENY=202,
	LOGIN_SESSION_EXIST,

	LOGOUT = 210,
	LOGOUT_OK = 211,
	LOGOUT_ERR = 212,

	SIGNUP = 290,
	SIGNUP_OK = 291,
	SIGNUP_ERR_ID = 292,
	SIGNUP_ERR_UNKNOWN = 293,


	TANK_JOIN = 300,
	TANK_MOVE,
	TANK_LEAVE,

	PING=999
};

