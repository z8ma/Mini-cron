#ifndef REQUEST_H
#define REQUEST_H


int handle_request(int fdrequest, int fdreply);

#define LS_OPCODE 0x4c53 // 'LS'
#define CR_OPCODE 0x4352 // 'CR'
#define CB_OPCODE 0x4342 // 'CB'
#define RM_OPCODE 0x524d // 'RM'
#define TX_OPCODE 0x5458 // 'TX'
#define SO_OPCODE 0x534f // 'SO'
#define SE_OPCODE 0x5345 // 'SE'
#define TM_OPCODE 0x4b49 // 'KI' surement pour kill

#define OK_ANSTYPE 0x4f4b // 'OK'
#define ER_ANSTYPE 0x4552 // 'ER'

#endif