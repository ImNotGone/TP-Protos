#ifndef RESPONSES_H
#define RESPONSES_H

#define CRLF "\r\n"
#define DOT_CRLF "."CRLF

#define OK_HEADER "+OK"
#define ERROR_HEADER "-ERR"

#define RESPONSE_OK_PLAIN       OK_HEADER""CRLF
#define RESPONSE_ERROR_PLAIN    ERROR_HEADER""CRLF

#define RESPONSE_UNKNOWN        ERROR_HEADER" UNKNOWN COMMAND"CRLF

#define RESPONSE_GREETING       OK_HEADER" POP3 SERVER READY"CRLF

// TODO: fill
// AUTH COMMANDS RESPONSES
// Mando OK igual en error y success
#define RESPONSE_USER           RESPONSE_OK_PLAIN

#define RESPONSE_PASS_ERROR              ERROR_HEADER" AUTHENTICATION FAILED"CRLF
#define RESPONSE_PASS_INVALID_PASSWORD   ERROR_HEADER" INVALID PASSWORD"CRLF
#define RESPONSE_PASS_BUSY               ERROR_HEADER" UNABLE TO LOCK MAILDROP"CRLF
#define RESPONSE_PASS_SUCCESS            OK_HEADER" AUTHORIZED"CRLF

// CAPABILITIES
// CAPA (suporteo el comando capa)
// USER (suporteo login con comandos USER PASS)
// PIPELINING (suporteo pipelining)
#define RESPONSE_AUTH_CAPA      OK_HEADER""CRLF     \
                                "CAPA"CRLF          \
                                "USER"CRLF          \
                                "PIPELINING"CRLF    \
                                DOT_CRLF


#define RESPONSE_AUTH_QUIT      OK_HEADER" QUITING"CRLF


// TRANSACTION COMMANDS RESPONSES
#define RESPONSE_TRANSACTION_NOOP   RESPONSE_OK_PLAIN

#define RESPONSE_TRANSACTION_RSET   RESPONSE_OK_PLAIN

// CAPABILITIES
// CAPA (suporteo el comando capa)
// PIPELINING (suporteo pipelining)
#define RESPONSE_TRANSACTION_CAPA   OK_HEADER""CRLF     \
                                    "CAPA"CRLF          \
                                    "PIPELINING"CRLF    \
                                    DOT_CRLF

// UPDATE COMMANDS RESPONSES
#define RESPONSE_UPDATE_QUIT_ERROR    ERROR_HEADER" COULD NOT DELETE MAILS"CRLF
#define RESPONSE_UPDATE_QUIT_SUCCESS  OK_HEADER" QUITING"CRLF

#endif
