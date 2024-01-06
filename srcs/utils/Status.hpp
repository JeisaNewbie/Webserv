#pragma once

/* 100 */

#define	CONTINUE					100

/* 200 */

#define	OK							200 //0
#define	CREATED						201
#define	ACCEPTED					202
#define	NO_CONTENT					204 //3
#define LAST_2XX					207

/* 300 */

#define OFF_3XX						7
#define	MOVED_PERMANENTLY			301 //4
#define	FOUND						302
#define	SEE_OTHER					303
#define	NOT_MODIFIED				304
#define	TEMPORARY_REDIRECT			307
#define	PERMANENT_REDIRECT			308 //9
#define LAST_3XX					309

/* 400 */

#define OFF_4XX						15
#define	BAD_REQUEST					400 //10
#define	UNAUTHORIZED				401
#define	FORBIDDEN					403
#define	NOT_FOUND					404
#define	METHOD_NOT_ALLOWED			405
#define	NOT_ACCEPTABLE				406
#define	REQUEST_TIMEOUT				408
#define	CONFLICT					409
#define	REQUEST_ENTITY_TOO_LARGE	413
#define	URI_TOO_LONG				414
#define	UNSUPPORTED_MEDIA_TYPE		415
#define	EXPECTION_FAILED			417 //21
#define LAST_4XX					430

/* 500 */

#define	OFF_5XX						45
#define	INTERNAL_SERVER_ERROR		500 //22
#define	NOT_IMPLEMENTED				501
#define	BAD_GATEWAY					502
#define	SERVICE_UNAVAILABLE			503
#define	GATEWAY_TIMEOUT				504
#define	HTTP_VERSION_NOT_SUPPORTED	505 //27


struct Status
{
	const char	*code;
	const char	*text;
};

extern Status	status_line[];
