
/* File generated by gen_cfile.py. Should not be modified. */

#include "master.h"

/**************************************************************************/
/* Declaration of mapped variables                                        */
/**************************************************************************/

/**************************************************************************/
/* Declaration of value range types                                       */
/**************************************************************************/

#define valueRange_EMC 0x9F /* Type for index 0x1003 subindex 0x00 (only set of value 0 is possible) */
UNS32 master_valueRangeTest (UNS8 typeValue, void * value)
{
  switch (typeValue) {
    case valueRange_EMC:
      if (*(UNS8*)value != (UNS8)0) return OD_VALUE_RANGE_EXCEEDED;
      break;
  }
  return 0;
}

/**************************************************************************/
/* The node id                                                            */
/**************************************************************************/
/* node_id default value.*/
UNS8 master_bDeviceNodeId = 0x00;

/**************************************************************************/
/* Array of message processing information */

const UNS8 master_iam_a_slave = 0;

TIMER_HANDLE master_heartBeatTimers[1];

/*
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

                               OBJECT DICTIONARY

$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
*/

/* index 0x1000 :   Device Type. */
                    UNS32 master_obj1000 = 0x0;	/* 0 */
                    subindex master_Index1000[] = 
                     {
                       { RO, uint32, sizeof (UNS32), (void*)&master_obj1000 }
                     };

/* index 0x1001 :   Error Register. */
                    UNS8 master_obj1001 = 0x0;	/* 0 */
                    subindex master_Index1001[] = 
                     {
                       { RO, uint8, sizeof (UNS8), (void*)&master_obj1001 }
                     };

/* index 0x1003 :   Pre-defined Error Field */
                    UNS8 master_highestSubIndex_obj1003 = 0; /* number of subindex - 1*/
                    UNS32 master_obj1003[] = 
                    {
                      0x0	/* 0 */
                    };
                    ODCallback_t master_Index1003_callbacks[] = 
                     {
                       NULL,
                       NULL,
                     };
                    subindex master_Index1003[] = 
                     {
                       { RW, valueRange_EMC, sizeof (UNS8), (void*)&master_highestSubIndex_obj1003 },
                       { RO, uint32, sizeof (UNS32), (void*)&master_obj1003[0] }
                     };

/* index 0x1005 :   SYNC COB ID */
                    UNS32 master_obj1005 = 0x0;   /* 0 */

/* index 0x1006 :   Communication / Cycle Period */
                    UNS32 master_obj1006 = 0x0;   /* 0 */

/* index 0x100C :   Guard Time */ 
                    UNS16 master_obj100C = 0x0;   /* 0 */

/* index 0x100D :   Life Time Factor */ 
                    UNS8 master_obj100D = 0x0;   /* 0 */

/* index 0x1014 :   Emergency COB ID */
                    UNS32 master_obj1014 = 0x80 + 0x00;   /* 128 + NodeID */

/* index 0x1016 :   Consumer Heartbeat Time */
                    UNS8 master_highestSubIndex_obj1016 = 0;
                    UNS32 master_obj1016[]={0};

/* index 0x1017 :   Producer Heartbeat Time. */
                    UNS16 master_obj1017 = 0x0;	/* 0 */
                    ODCallback_t master_Index1017_callbacks[] = 
                     {
                       NULL,
                     };
                    subindex master_Index1017[] = 
                     {
                       { RW, uint16, sizeof (UNS16), (void*)&master_obj1017 }
                     };

/* index 0x1018 :   Identity. */
                    UNS8 master_highestSubIndex_obj1018 = 4; /* number of subindex - 1*/
                    UNS32 master_obj1018_Vendor_ID = 0x0;	/* 0 */
                    UNS32 master_obj1018_Product_Code = 0x0;	/* 0 */
                    UNS32 master_obj1018_Revision_Number = 0x0;	/* 0 */
                    UNS32 master_obj1018_Serial_Number = 0x0;	/* 0 */
                    subindex master_Index1018[] = 
                     {
                       { RO, uint8, sizeof (UNS8), (void*)&master_highestSubIndex_obj1018 },
                       { RO, uint32, sizeof (UNS32), (void*)&master_obj1018_Vendor_ID },
                       { RO, uint32, sizeof (UNS32), (void*)&master_obj1018_Product_Code },
                       { RO, uint32, sizeof (UNS32), (void*)&master_obj1018_Revision_Number },
                       { RO, uint32, sizeof (UNS32), (void*)&master_obj1018_Serial_Number }
                     };

/* index 0x1280 :   Client SDO 1 Parameter. */
                    UNS8 master_highestSubIndex_obj1280 = 3; /* number of subindex - 1*/
                    UNS32 master_obj1280_COB_ID_Client_to_Server_Transmit_SDO = 0x0;	/* 0 */
                    UNS32 master_obj1280_COB_ID_Server_to_Client_Receive_SDO = 0x0;	/* 0 */
                    UNS8 master_obj1280_Node_ID_of_the_SDO_Server = 0x0;	/* 0 */
                    subindex master_Index1280[] = 
                     {
                       { RO, uint8, sizeof (UNS8), (void*)&master_highestSubIndex_obj1280 },
                       { RW, uint32, sizeof (UNS32), (void*)&master_obj1280_COB_ID_Client_to_Server_Transmit_SDO },
                       { RW, uint32, sizeof (UNS32), (void*)&master_obj1280_COB_ID_Server_to_Client_Receive_SDO },
                       { RW, uint8, sizeof (UNS8), (void*)&master_obj1280_Node_ID_of_the_SDO_Server }
                     };

/**************************************************************************/
/* Declaration of pointed variables                                       */
/**************************************************************************/

const indextable master_objdict[] = 
{
  { (subindex*)master_Index1000,sizeof(master_Index1000)/sizeof(master_Index1000[0]), 0x1000},
  { (subindex*)master_Index1001,sizeof(master_Index1001)/sizeof(master_Index1001[0]), 0x1001},
  { (subindex*)master_Index1017,sizeof(master_Index1017)/sizeof(master_Index1017[0]), 0x1017},
  { (subindex*)master_Index1018,sizeof(master_Index1018)/sizeof(master_Index1018[0]), 0x1018},
  { (subindex*)master_Index1280,sizeof(master_Index1280)/sizeof(master_Index1280[0]), 0x1280},
};

const indextable * master_scanIndexOD (UNS16 wIndex, UNS32 * errorCode, ODCallback_t **callbacks)
{
	int i;
	*callbacks = NULL;
	switch(wIndex){
		case 0x1000: i = 0;break;
		case 0x1001: i = 1;break;
		case 0x1017: i = 2;*callbacks = master_Index1017_callbacks; break;
		case 0x1018: i = 3;break;
		case 0x1280: i = 4;break;
		default:
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
	}
	*errorCode = OD_SUCCESSFUL;
	return &master_objdict[i];
}

/* 
 * To count at which received SYNC a PDO must be sent.
 * Even if no pdoTransmit are defined, at least one entry is computed
 * for compilations issues.
 */
s_PDO_status master_PDO_status[1] = {s_PDO_status_Initializer};

const quick_index master_firstIndex = {
  0, /* SDO_SVR */
  4, /* SDO_CLT */
  0, /* PDO_RCV */
  0, /* PDO_RCV_MAP */
  0, /* PDO_TRS */
  0 /* PDO_TRS_MAP */
};

const quick_index master_lastIndex = {
  0, /* SDO_SVR */
  4, /* SDO_CLT */
  0, /* PDO_RCV */
  0, /* PDO_RCV_MAP */
  0, /* PDO_TRS */
  0 /* PDO_TRS_MAP */
};

const UNS16 master_ObjdictSize = sizeof(master_objdict)/sizeof(master_objdict[0]); 

CO_Data master_Data = CANOPEN_NODE_DATA_INITIALIZER(master);

